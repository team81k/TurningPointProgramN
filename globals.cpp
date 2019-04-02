#include "main.h"
#include "globals.hpp"
#include "pid.hpp"
#include "units.hpp"

pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Controller partner(pros::E_CONTROLLER_PARTNER);
pros::Motor FR(1), FL(2, true), BR(3), BL(4, true);
pros::Motor flywheel(5, pros::E_MOTOR_GEARSET_06, true);
pros::Motor intake(6);

double DIFFERENTIAL_UP = 4095;
double DIFFERENTIAL_DOWN = 10;

pros::ADIPotentiometer differentialPot('a');

double flywheelSpeed = 0;
double flywheelPower = 0;
long flywheelLaunchStart = 0;
long update = 0;

double wheelWidth = (13.6_in + 10.5_in) / 2.0; //inches
double wheelCircumference = PI * 4_in; //inches
double gearRatio = 18.0 / 15.0;
double ticksPerRevolution = 900;
/*
1800 ticks/rev with 36:1 gears (red)
900 ticks/rev with 18:1 gears (green)
300 ticks/rev with 6:1 gears (blue)
*/

double getDriveStraightTicks(double meters)
{ return (meters / wheelCircumference) / gearRatio * ticksPerRevolution; }
double getDriveTurnTicks(double radians)
{ return getDriveStraightTicks((PI * wheelWidth) * (radians / TAU)); }

pid rightSide(0, 0, 0, -127, 127, 0);
pid leftSide(0, 0, 0, -127, 127, 0);
pid differentialPID(0.15, 0, 0.012, -90, 90, 127 * 3);
pid flywheelPID1(1.8, 0, 0, -127, 127, 127);
pid flywheelPID2(0.15, 0, 0, -127, 127, 127);

pid forwardDrivePID(0.2, 0, 0.05, -100, 100, 127 * 3);
pid turnDrivePID(0.4, 0.001, 0.025, -100, 100, 127 * 3);

double robotX = 0; //in inches
double robotY = 0;
double robotDir = 0; //radians

pros::task_t autonomousAsyncTask;

//0 = none
//1 = sides
//2 = skills
int autonType = 0;

bool autonRed = true;
bool autonNear = true;
bool autonPlatform = true;

int activePage = 0;
int runAutonomous = -1;

lv_style_t screenStyle = lv_style_scr;

lv_obj_t * * homePage;
lv_obj_t * homeTextPage, * homeTextObject, * homeChart;
lv_chart_series_t * seriesZero, * series[10];
char buffer[512];

lv_obj_t * * autonomousPage;
lv_obj_t * autonTypePage, * autonNone, * autonSides, * autonSkills;
lv_obj_t * autonNonePage;
lv_obj_t * autonSidesPage, * autonSideColor, * autonSideDistance, * autonSidePlatform;
lv_obj_t * autonSideDescription;
lv_obj_t * autonSkillsPage;

lv_obj_t * * autonomousRunPage;
lv_obj_t * autonRunTypePage, * autonRunNormal, * autonRunSkills, * autonRunUnlimited;
lv_obj_t * autonRunDescription;

bool pageNavigation = true;

void setNavigation(bool nav)
{
    pageNavigation = nav;
    lv_obj_set_hidden(homePage[1], !nav);
    lv_obj_set_hidden(homePage[2], !nav);
    lv_obj_set_hidden(autonomousPage[1], !nav);
    lv_obj_set_hidden(autonomousPage[2], !nav);
    lv_obj_set_hidden(autonomousRunPage[1], !nav);
    lv_obj_set_hidden(autonomousRunPage[2], !nav);
}

void setPage(int page)
{
    activePage = page;
    lv_obj_set_hidden(homePage[0], page != 0);
    lv_obj_set_hidden(autonomousPage[0], page != 1);
    lv_obj_set_hidden(autonomousRunPage[0], page != 2);
}

std::string generateSidesDescription()
{
    std::string description = "The robot will ";
	if(autonRed) description += "Red, ";
    else description += "Blue, ";
    if(autonNear) description += "Near, ";
    else description += "Far, ";
    if(autonPlatform) description += "Platform: Yes";
    else description += "Platform: No";
    description += ". And stuff. Bla Bla Bla.";
    return description;
}

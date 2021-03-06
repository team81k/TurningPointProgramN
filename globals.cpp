#include "main.h"
#include "globals.hpp"
#include "pid.hpp"
#include "units.hpp"

pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Controller partner(pros::E_CONTROLLER_PARTNER);
pros::Motor FR(1), FL(2, true), BR(3), BL(4, true);
pros::Motor flywheel(5, pros::E_MOTOR_GEARSET_06, true);
pros::Motor intake(6);
pros::Motor hood(7);
pros::Motor lift(8);

double DIFFERENTIAL_UP = 4095;
double DIFFERENTIAL_DOWN = 10;

double HOOD_TOP_FLAG = 0;
double HOOD_LOW_FLAG = 4095;

pros::ADIPotentiometer differentialPot('a');
pros::ADIPotentiometer hoodPot('c');

bool differentialStay = true;

int liftStep = 0;
long liftTimerStart = 0;
int liftSetSpeed = 0;

double flywheelSpeed = 0;
double preFlywheelSpeed = 0;
double flywheelPower = 0;
long flywheelLaunchStart = 0;
int doubleShot = 0;
long doubleShotStart = 0;
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
pid hoodPID(0.03, 0, 0.0005, -127, 127, 0);
pid liftPID(0.15, 0, 0, -127, 127, 0);

pid forwardDrivePID(0.35, 0, 0.05, -127, 127, 127 * 3);
pid turnDrivePID(1.5, 0, 0.5, -127, 127, 127 * 3);

double robotX = 0; //in inches
double robotY = 0;
double robotDir = 0; //radians

//0 = none
//1 = sides
//2 = skills
int autonType = 0;

bool autonRed = true;
bool autonNear = true;
bool autonPlatform = true;

bool autonSkillsRed = true;

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
lv_obj_t * autonSkillsPage, * autonSkillsColor;
lv_obj_t * autonSkillsDescription;

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
	if(autonRed) description += "start on #ff0000 red#, ";
    else description += "start on #0000ff blue#, ";
    if(autonNear) description += "near tile. ";
    else description += "far tile. ";
    description += "It will get another rball and flip all three ";
    if(autonNear) description += "flags. ";
    else description += "center flags. ";
    if(autonPlatform) description += "Then it will drive onto the low platform.";
    else description += "";
    return description;
}

std::string generateSkillsDescription()
{
    std::string description = "The robot will ";
    if(autonSkillsRed) description += "start on #ff0000 red#, ";
    else description += "start on #0000ff blue#, ";
    description += "near tile. It will get another ball and flip all three flags. Then it will drive onto the high platform.";
    return description;
}

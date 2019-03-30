#include "main.h"
#include "globals.hpp"
#include "pid.hpp"
#include "units.hpp"

pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Controller partner(pros::E_CONTROLLER_PARTNER);
pros::Motor FR(1), FL(2, true), BR(3), BL(4, true);

double DIFFERENTIAL_UP = 4095;
double DIFFERENTIAL_DOWN = 10;

pros::ADIPotentiometer differentialPot('a');

double wheelWidth = (13.6_in + 10.5_in) / 2.0; //inches
double wheelCircumference = PI * 4_in; //inches
double gearRatio = 18.0 / 15.0;
double ticksPerRevolution = 900;
/*
1800 ticks/rev with 36:1 gears (red)
900 ticks/rev with 18:1 gears (green)
300 ticks/rev with 6:1 gears (blue)
*/

pid rightSide(0, 0, 0, -127, 127, 0);
pid leftSide(0, 0, 0, -127, 127, 0);
pid differentialPID(0.15, 0, 0.012, -90, 90, 127 * 3);

pid forwardDrivePID(0.12, 0, 0.05, -127, 127, 127 * 3);
pid turnDrivePID(0.3, 0.001, 0.02, -127, 127, 127 * 3);

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

lv_style_t screenStyle = lv_style_scr;

lv_obj_t * homePage;
lv_obj_t * homeTitlePage, * homeTitleText, * homeTitleAuton;
lv_obj_t * homeTextPage, * homeTextObject;

lv_obj_t * autonomousPage;
lv_obj_t * autonTitlePage, * autonTitleHome, * autonTitleText, * autonTitleRun;
lv_obj_t * autonTypePage, * autonNone, * autonSides, * autonSkills;
lv_obj_t * autonNonePage;
lv_obj_t * autonSidesPage, * autonSideColor, * autonSideDistance, * autonSidePlatform;
lv_obj_t * autonSideDescription;
lv_obj_t * autonSkillsPage;

lv_obj_t * autonomousRunPage;
lv_obj_t * autonRunTitlePage, * autonRunTitleHome, * autonRunTitleText, * autonRunTitleBack;

void setAutonomousNav(bool nav)
{
    lv_obj_set_hidden(autonTitleHome, !nav);
    lv_obj_set_hidden(autonTitleRun, !nav);
}

void setPage(int page)
{
    lv_obj_set_hidden(homePage, page != 0);
    lv_obj_set_hidden(autonomousPage, page != 1);
    lv_obj_set_hidden(autonomousRunPage, page != 2);
}

const char * generateSidesDescription()
{
    std::string description = "The robot will ";
	if(autonRed) description += "Red, ";
    else description += "Blue, ";
    if(autonNear) description += "Near, ";
    else description += "Far, ";
    if(autonPlatform) description += "Platform: Yes";
    else description += "Platform: No";
    description += ". And stuff.";
    return description.c_str();
}

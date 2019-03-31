#include "main.h"
#include "pid.hpp"
#include "units.hpp"

#pragma once

extern pros::Controller master;
extern pros::Controller partner;
extern pros::Motor FR, FL, BR, BL;
extern pros::Motor flywheel;
extern pros::Motor intake;

extern double DIFFERENTIAL_UP;
extern double DIFFERENTIAL_DOWN;

extern pros::ADIPotentiometer differentialPot;

extern double wheelWidth;
extern double wheelCircumference;
extern double gearRatio;
extern double ticksPerRevolution;

extern pid rightSide;
extern pid leftSide;
extern pid differentialPID;
extern pid flywheelPID1;
extern pid flywheelPID2;

extern pid forwardDrivePID;
extern pid turnDrivePID;

extern double robotX;
extern double robotY;
extern double robotDir;

extern int autonType;

extern bool autonRed;
extern bool autonNear;
extern bool autonPlatform;

extern int activePage;
extern int runAutonomous;

extern lv_style_t screenStyle;

extern lv_obj_t * * homePage;
extern lv_obj_t * homeTextPage, * homeTextObject;
extern char buffer[250];

extern lv_obj_t * * autonomousPage;
extern lv_obj_t * autonTypePage, * autonNone, * autonSides, * autonSkills;
extern lv_obj_t * autonNonePage;
extern lv_obj_t * autonSidesPage, * autonSideColor, * autonSideDistance, * autonSidePlatform;
extern lv_obj_t * autonSideDescription;
extern lv_obj_t * autonSkillsPage;

extern lv_obj_t * * autonomousRunPage;
extern lv_obj_t * autonRunTypePage, * autonRunNormal, * autonRunSkills, * autonRunUnlimited;

extern bool pageNavigation;

void setNavigation(bool nav);
void setPage(int page);
const char * generateSidesDescription();

#include "main.h"
#include "pid.hpp"
#include "units.hpp"

#pragma once

extern pros::Controller master;
extern pros::Controller partner;
extern pros::Motor FR, FL, BR, BL;

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

extern pid forwardDrivePID;
extern pid turnDrivePID;

extern double robotX;
extern double robotY;
extern double robotDir;

extern lv_style_t screenStyle;

extern lv_obj_t * homePage;
extern lv_obj_t * textObject;

extern lv_obj_t * autonomousPage;

extern lv_obj_t * autonomousRunPage;

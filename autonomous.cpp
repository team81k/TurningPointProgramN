#include "main.h"
#include "globals.hpp"
#include "pid.hpp"
#include <vector>

int driveType = 0;
double driveThreshold = 0;

void autonomousAsync(void * param)
{
    if(driveType == 0)
    {
        FR.move(0);
        BR.move(0);
        FL.move(0);
        BL.move(0);
    }
    else if(driveType == 1)
    {
        double avgDistance = (FR.get_position() + BR.get_position() + FL.get_position() + BL.get_position()) / 4;

        FR.move(forwardDrivePID.calculate(avgDistance));
        BR.move(forwardDrivePID.getPower());
        FL.move(forwardDrivePID.getPower());
        BL.move(forwardDrivePID.getPower());
    }
    else if(driveType == 2)
    {
        double avgDistance = (FR.get_position() + BR.get_position() - FL.get_position() - BL.get_position()) / 4;

        FR.move(turnDrivePID.calculate(avgDistance));
        BR.move(turnDrivePID.getPower());
        FL.move(-turnDrivePID.getPower());
        BL.move(-turnDrivePID.getPower());
    }
}

/*
void driveStrait(double meters, bool wait = true, long timeout = -1);
void driveTurn(double radians, bool wait = true, long timeout = -1);
void waitOnDrive();
void driveStop();
void flywheelSpeed(double speed, bool wait = true, long timeout = -1);
void waitOnFlywheel();
void intakeIn();
void launchBall(bool wait =  true, long timeout = -1);
*/

void driveStrait(double meters, bool wait = true, long timeout = -1)
{
    driveType = 1;
    FR.tare_position();BR.tare_position();FL.tare_position();BL.tare_position();
    forwardDrivePID.clear();
    forwardDrivePID.target = (meters / wheelCircumference) / gearRatio * ticksPerRevolution;
    driveThreshold = 50;
    if(wait) while(fabs(forwardDrivePID.getError()) > driveThreshold) pros::delay(3);
}

void driveTurn(double radians, bool wait = true, long timeout = -1)
{
    driveType = 2;
    FR.tare_position();BR.tare_position();FL.tare_position();BL.tare_position();
    turnDrivePID.clear();
    double inches = (PI * wheelWidth) * (radians / TAU);
    turnDrivePID.target = (inches / wheelCircumference) / gearRatio * ticksPerRevolution;
    driveThreshold = 50;
    if(wait) while(fabs(turnDrivePID.getError()) > driveThreshold) pros::delay(3);
}

void waitOnDrive()
{
    if(driveType == 1) while(fabs(forwardDrivePID.getError()) > driveThreshold) pros::delay(3);
    else if(driveType == 2) while(fabs(turnDrivePID.getError()) > driveThreshold) pros::delay(3);
}

void driveStop() { driveType = 0; }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void autonomous()
{
    pros::task_t autonomousAsyncTask = pros::c::task_create(autonomousAsync, (void*)"PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "autonomousAsync");

    for(int i = 0; i < 8; i++)
    {
        driveStrait(10_in);
        driveTurn(90_deg);
    }

    driveStop();

    pros::c::task_delete(autonomousAsyncTask);
}

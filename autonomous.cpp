#include "main.h"
#include "globals.hpp"
#include "pid.hpp"
#include <vector>

int driveType = 0;
double driveThreshold = 0;
bool autonPIDReady = false;

void autonomousAsync(void * param)
{
    while(true)
    {
        double rightPower = 0;
        double leftPower = 0;

        if(driveType == 1)
        {
            double avgDistance = (FR.get_position() + BR.get_position() + FL.get_position() + BL.get_position()) / 4;

            rightPower = forwardDrivePID.calculate(avgDistance);
            leftPower = forwardDrivePID.getPower();
        }
        else if(driveType == 2)
        {
            double avgDistance = (FR.get_position() + BR.get_position() - FL.get_position() - BL.get_position()) / 4;

            rightPower = turnDrivePID.calculate(avgDistance);
            leftPower = -turnDrivePID.getPower();
        }

        double differentialPower = differentialPID.calculate(differentialPot.get_value());

        double FRP = rightPower + differentialPower;
        double BRP = rightPower - differentialPower;
        double FLP = leftPower + differentialPower;
        double BLP = leftPower - differentialPower;

        if(FRP > 127) { BRP -= FRP - 127; FRP = 127; }
        if(BRP > 127) { FRP -= BRP - 127; BRP = 127; }
        if(FRP < -127) { BRP -= FRP + 127; FRP = -127; }
        if(BRP < -127) { FRP -= BRP + 127; BRP = -127; }

        if(FLP > 127) { BLP -= FLP - 127; FLP = 127; }
        if(BLP > 127) { FLP -= BLP - 127; BLP = 127; }
        if(FLP < -127) { BLP -= FLP + 127; FLP = -127; }
        if(BLP < -127) { FLP -= BLP + 127; BLP = -127; }

        FR.move(FRP);
        BR.move(BRP);
        FL.move(FLP);
        BL.move(BLP);

        autonPIDReady = true;

        pros::delay(3);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void driveTransform(double position, bool wait = true, long timeout = -1)
{
    differentialPID.setTarget(position);
    autonPIDReady = false;
    long waitStart = pros::millis();
    if(wait) while((fabs(differentialPID.getError()) > 25 || !autonPIDReady) && (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

void driveStraight(double meters, bool wait = true, long timeout = -1)
{
    driveType = 1;
    FR.tare_position();BR.tare_position();FL.tare_position();BL.tare_position();
    forwardDrivePID.clear();
    forwardDrivePID.setTarget(getDriveStraightTicks(meters));
    driveThreshold = getDriveStraightTicks(0.25_in);
    autonPIDReady = false;
    long waitStart = pros::millis();
    if(wait) while((fabs(forwardDrivePID.getError()) > driveThreshold || !autonPIDReady) && (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

void driveTurn(double radians, bool wait = true, long timeout = -1)
{
    driveType = 2;
    FR.tare_position();BR.tare_position();FL.tare_position();BL.tare_position();
    turnDrivePID.clear();
    turnDrivePID.setTarget(getDriveTurnTicks(radians));
    driveThreshold = getDriveTurnTicks(1_deg);
    autonPIDReady = false;
    long waitStart = pros::millis();
    if(wait) while((fabs(turnDrivePID.getError()) > driveThreshold || !autonPIDReady) && (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

void waitOnDrive(long timeout = -1)
{
    long waitStart = pros::millis();
    if(driveType == 1) while((fabs(forwardDrivePID.getError()) > driveThreshold || fabs(differentialPID.getError()) > 25 || !autonPIDReady) && (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
    else if(driveType == 2) while((fabs(turnDrivePID.getError()) > driveThreshold || fabs(differentialPID.getError()) > 25 || !autonPIDReady) && (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

void driveStop() { driveType = 0; }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void driveTransform(double position, bool wait = true, long timeout = -1);
void driveStraight(double meters, bool wait = true, long timeout = -1);
void driveTurn(double radians, bool wait = true, long timeout = -1);
void waitOnDrive(long timeout = -1);
void driveStop();
void flywheelSpeed(double speed, bool wait = true, long timeout = -1);
void waitOnFlywheel();
void intakeIn();
void launchBall(bool wait =  true, long timeout = -1);
*/

void autonomous()
{
    setNavigation(false);
	setPage(0);
    lv_label_set_text(homeTextObject, "");

    autonomousAsyncTask = pros::c::task_create(autonomousAsync, (void*)"PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "autonomousAsync");

    for(int i = 0; i < 8; i++)
    {
        driveStraight(10_in);
        driveTurn(90_deg);
    }

    driveStop();

    pros::c::task_delete(autonomousAsyncTask);
}

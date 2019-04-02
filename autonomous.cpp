#include "main.h"
#include "globals.hpp"
#include "pid.hpp"
#include <vector>

int driveType = 0;
double driveThreshold = 0;
bool autonPIDReady = false;

void autonomousAsync(void * param)
{
    driveType = 0;
    flywheelSpeed = 0;
    flywheelPower = 0;
    flywheelLaunchStart = 0;
    autonPIDReady = false;

    while(true)
    {
        //Drive
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

        //flywheel
        flywheelPID1.setTarget(flywheelSpeed * 6);
		flywheelPID2.setTarget(flywheelSpeed * 6);
		double flywheelActualSpeed = flywheel.get_actual_velocity();
		flywheelPower = flywheelPID1.calculate(flywheelActualSpeed);
		if(fabs(flywheelPID1.getError()) < 0.15 * 600.0)
		{
			flywheelPower = flywheelPID2.calculate(flywheelActualSpeed) + flywheelSpeed * 1.2;
			flywheelPID1.setPower(flywheelPower);
		}

		if(flywheelPower > 127) flywheelPower = 127;
		if(flywheelPower < -127) flywheelPower = -127;

		if(flywheelLaunchStart != -1 && pros::millis() - flywheelLaunchStart < 50) flywheel.move(0);
		else
		{
			flywheel.move(flywheelPower);
		}

        if(flywheelLaunchStart != -1 && pros::millis() - flywheelLaunchStart > 50) flywheelLaunchStart = -1;

        //Display values
		/*sprintf(buffer,
			"robot: (%f, %f)\n"
			"angle: %fdeg\n",
            robotX, robotY,
            robotDir * 180.0 / PI);
		lv_label_set_text(homeTextObject, buffer);*/

		if(pros::millis() - update > 50)
		{
			update = pros::millis();
			lv_chart_set_next(homeChart, series[0], flywheelPower * (100.0 / 127.0));
			lv_chart_set_next(homeChart, series[1], flywheelActualSpeed / 6.0);
			lv_chart_set_next(homeChart, series[2], flywheelSpeed);
		}

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
    if(wait) while((fabs(differentialPID.getError()) > 25 || !autonPIDReady) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
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
    if(wait) while((fabs(forwardDrivePID.getError()) > driveThreshold || !autonPIDReady) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
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
    if(wait) while((fabs(turnDrivePID.getError()) > driveThreshold || !autonPIDReady) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

void waitOnDrive(long timeout = -1)
{
    long waitStart = pros::millis();
    if(driveType == 1) while((fabs(forwardDrivePID.getError()) > driveThreshold ||
        fabs(differentialPID.getError()) > 25 || !autonPIDReady) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
    else if(driveType == 2) while((fabs(turnDrivePID.getError()) > driveThreshold ||
        fabs(differentialPID.getError()) > 25 || !autonPIDReady) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

void driveStop() { driveType = 0; }

void flywheelSpin(double speed, bool wait = true, long timeout = -1)
{
    flywheelSpeed = speed;
    autonPIDReady = false;
    long waitStart = pros::millis();
    if(wait) while((fabs(flywheelPID1.getError()) > 0.015 * 600 || !autonPIDReady) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

void waitOnFlywheel(long timeout = -1)
{
    long waitStart = pros::millis();
    while((fabs(flywheelPID1.getError()) > 0.015 * 600 || !autonPIDReady) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

void intakeSpin(double speed)
{
    intake.move(speed);
}

void launchBall(bool wait = true, long timeout = -1)
{
    flywheelLaunchStart = pros::millis();
    long waitStart = pros::millis();
    if(wait) while(flywheelLaunchStart != -1 &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) pros::delay(3);
}

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
void flywheelSpin(double speed, bool wait = true, long timeout = -1);
void waitOnFlywheel(long timeout = -1);
void intakeSpin(double speed);
void launchBall(bool wait = true, long timeout = -1);
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

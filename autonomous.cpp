#include "main.h"
#include "globals.hpp"
#include "pid.hpp"
#include <vector>

int driveType = 0;
double driveThreshold = 0;
bool turnNegative = false;

void autonomousRun(int delayTime = 0)
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

    //hood
    hood.move(-hoodPID.calculate(hoodPot.get_value()));
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

    if(delayTime != 0) pros::delay(delayTime);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void driveTransform(double position, bool wait = true, long timeout = -1)
{
    differentialPID.setTarget(position);
    autonomousRun();
    long waitStart = pros::millis();
    if(wait) while(fabs(differentialPID.getError()) > 25 &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
}

void driveStraight(double meters, bool wait = true, long timeout = -1)
{
    driveType = 1;
    FR.tare_position();BR.tare_position();FL.tare_position();BL.tare_position();
    forwardDrivePID.clear();
    forwardDrivePID.setTarget(getDriveStraightTicks(meters));
    driveThreshold = getDriveStraightTicks(0.5_in);
    autonomousRun();
    long waitStart = pros::millis();
    if(wait) while((fabs(forwardDrivePID.getError()) > driveThreshold || forwardDrivePID.getChange() > 5) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
}

void driveTurn(double radians, bool wait = true, long timeout = -1)
{
    if(turnNegative) radians = -radians;
    driveType = 2;
    FR.tare_position();BR.tare_position();FL.tare_position();BL.tare_position();
    turnDrivePID.clear();
    turnDrivePID.Kp = (1 / fabs(radians / PI)) * 2.5;
    turnDrivePID.Kd = (1 / fabs(radians / PI)) * 0.8;
    turnDrivePID.setTarget(getDriveTurnTicks(radians));
    driveThreshold = getDriveTurnTicks(1_deg);
    autonomousRun();
    long waitStart = pros::millis();
    if(wait) while((fabs(turnDrivePID.getError()) > driveThreshold || turnDrivePID.getChange() > 5) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
}

void waitOnDrive(long timeout = -1)
{
    long waitStart = pros::millis();
    if(driveType == 1) while((fabs(forwardDrivePID.getError()) > driveThreshold ||
        forwardDrivePID.getChange() > 5 || fabs(differentialPID.getError()) > 25) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
    else if(driveType == 2) while((fabs(turnDrivePID.getError()) > driveThreshold ||
        turnDrivePID.getChange() > 5 || fabs(differentialPID.getError()) > 25) &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
}

void driveStop() { driveType = 0; }

void flywheelSpin(double speed, bool wait = true, long timeout = -1)
{
    flywheelSpeed = speed;
    autonomousRun();
    long waitStart = pros::millis();
    if(wait) while(fabs(flywheelPID1.getError()) > 0.015 * 600 &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
}

void waitOnFlywheel(long timeout = -1)
{
    long waitStart = pros::millis();
    while(fabs(flywheelPID1.getError()) > 0.015 * 600 &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
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
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
}

void autonDelay(int delayTime)
{
    long delayStart = pros::millis();
    while(pros::millis() - delayStart < delayTime) autonomousRun(3);
}

void hoodSet(double position, bool wait = true, long timeout = -1)
{
    hoodPID.setTarget(position);
    autonomousRun();
    long waitStart = pros::millis();
    if(wait) while(fabs(hoodPID.getError()) < 300 &&
        (timeout == -1 || pros::millis() - waitStart < timeout)) autonomousRun(3);
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
void autonDelay(int delayTime);
void hoodSet(double position, bool wait = true, long timeout = -1);
*/

void autonomous()
{
    setNavigation(false);
	setPage(0);
    lv_label_set_text(homeTextObject, "");

    if(autonType == 1 && autonNear)
    {
        if(!autonRed) turnNegative = true;

        intakeSpin(60);
        flywheelSpin(100, false);
        driveStraight(50_in);
        autonDelay(500);
        driveStraight(-40_in);
        driveTurn(90_deg);
        waitOnFlywheel();
        launchBall();
        driveStraight(25_in);
        launchBall();
        intakeSpin(0);
        flywheelSpin(0, false);
        driveTurn(20_deg);
        driveStraight(20_in);
        driveStraight(-20_in);
        driveTurn(-20_deg);
        driveStraight(-20_in);
        driveStop();
        intakeSpin(0);
    }
}

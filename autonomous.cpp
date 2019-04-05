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

    FR.move_velocity(FRP);
    BR.move_velocity(BRP);
    FL.move_velocity(FLP);
    BL.move_velocity(BLP);

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

    //lift
    if(liftStep == 0)
    {
        lift.move(-50);
        liftTimerStart = pros::millis();
        liftStep++;
    }
    else if(liftStep == 1)
    {
        if(pros::millis() - liftTimerStart > 2000 || (pros::millis() - liftTimerStart > 500 && fabs(lift.get_actual_velocity()) < 5))
        {
            lift.move(0);
            liftTimerStart = pros::millis();
            liftStep++;
        }
    }
    else if(liftStep == 2)
    {
        if(pros::millis() - liftTimerStart > 500)
        {
            lift.tare_position();
            liftStep++;
        }
    }
    else if(liftStep == 3)
    {
        if(liftSetSpeed >= 0 && lift.get_position() > 500) liftSetSpeed = (800 - lift.get_position()) * 0.15;
        if(liftSetSpeed <= 0 && lift.get_position() < 300) liftSetSpeed = lift.get_position() * -0.15;

        lift.move(liftSetSpeed);
    }

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
    turnDrivePID.Kd = (1 / fabs(radians / PI)) * 0.85;
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

void flipperSpeed(double speed)
{
    liftSetSpeed = speed;
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
void flipperSpeed(double speed, bool wait = true, long timeout = -1);
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
        hoodSet(HOOD_TOP_FLAG, false);
        flywheelSpin(90, false);
        driveStraight(45_in);
        autonDelay(250);
        driveStraight(-36_in);
        driveTurn(90_deg);
        driveStraight(4_in);
        waitOnFlywheel();
        launchBall();
        hoodSet(HOOD_LOW_FLAG);
        autonDelay(100);
        launchBall();
        driveStraight(34_in);
        flipperSpeed(100);
        autonDelay(500);
        if(autonPlatform)
        {
            driveStraight(-56_in, false);
            autonDelay(1000);
            flipperSpeed(-100);
            waitOnDrive();
            driveTurn(-90_deg);
            driveStraight(40_in, true, 2500);
        }
        else
        {
            driveStraight(-30_in);
            flipperSpeed(-100);
        }
        driveStop();
        intakeSpin(0);
    }

    if(autonType == 2)
    {
        if(!autonSkillsRed) turnNegative = true;

        intakeSpin(60);
        hoodSet(HOOD_TOP_FLAG, false);
        flywheelSpin(90, false);
        driveStraight(45_in);
        autonDelay(250);
        driveStraight(-36_in);
        driveTurn(90_deg);
        driveStraight(8_in);
        waitOnFlywheel();
        launchBall();
        hoodSet(HOOD_LOW_FLAG);
        autonDelay(100);
        launchBall();
        driveStraight(30_in);
        flipperSpeed(100);
        autonDelay(500);
        driveStraight(-62_in, false);
        autonDelay(1000);
        flipperSpeed(-100);
        waitOnDrive();
        driveTurn(-90_deg);
        driveStraight(100_in, true, 3000);
        driveStop();
        intakeSpin(0);
    }

    autonDelay(99999999);
}

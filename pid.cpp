#include "main.h"
#include "pid.hpp"

pid::pid(double Kp, double Ki, double Kd, double min, double max, double slew) :
    Kp(Kp), Ki(Ki), Kd(Kd), min(min), max(max), slew(slew)
{
    if(min > max) std::swap(min, max);
}

double pid::calculate(double value)
{
    long currentTime = pros::millis();
    bool timeData = lastUpdate != -1;
    bool timeUpdate = (currentTime - lastUpdate) >= updateMin;

    double uM = (currentTime - lastUpdate) / 1000.0; //for update per second
    if(timeUpdate || !timeData) lastUpdate = currentTime;

    switch(step)
    {
        case 0:
            error = target - value;
            if(timeData  && timeUpdate) integral += error * uM; //add error once per second
            if(timeData  && timeUpdate) derivative = (error - preError) / uM; //change per second

            if(timeUpdate) preError = error;

            //pid
            powerPreSlew = Kp * error + Ki * integral + Kd * derivative;

            //min/max
            if(powerPreSlew > max) powerPreSlew = max;
            if(powerPreSlew < min) powerPreSlew = min;
        case 1:

            //slew
            if(timeData && timeUpdate)
            {
                double change = powerPreSlew - power;
                if(change > slew * uM && slew != 0) change = slew * uM;
                if(change < -negativeSlew * uM && negativeSlew != 0) change = -negativeSlew * uM;
                else if(change < -slew * uM && slew != 0) change = -slew * uM;
                power += change;
            }
    }

    return power;
}

void pid::setTarget(double targetI)
{
    target = targetI;
    step = 0;
}

void pid::setPowerPreSlew(double powerPreSlewI)
{
    powerPreSlew = powerPreSlewI;
    step = 1;
}

void pid::setPower(double powerI)
{
    power = powerI;
    step = 2;
}

double pid::getPower()
{
    return power;
}

double pid::getError()
{
    return error;
}

void pid::clear()
{
    target = 0;
    step = 0;
    error = 0;
    derivative = 0;
    powerPreSlew = 0;
    power = 0;
    preError = 0;
    lastUpdate = -1;
}

// void update()
// {
//     //drive PD
//     rightPowerPreSlew = driveP * (rightTarget - FR.get_position()) - driveD * rightSpeed;
//     leftPowerPreSlew = driveP * (leftTarget - FL.get_position()) - driveD * leftSpeed;
//
//     if(rightPowerPreSlew > 127) rightPowerPreSlew = 127;
//     if(rightPowerPreSlew < -127) rightPowerPreSlew = -127;
//     if(leftPowerPreSlew > 127) leftPowerPreSlew = 127;
//     if(leftPowerPreSlew < -127) leftPowerPreSlew = -127;
//
//     //drive adjust
//     double driveDifference = FR.get_position() - FL.get_position();
//
//     driveAdjust = driveDifference;// * 0.001;
//
//     //if(driveAdjust > 50) driveAdjust = 50;
//     //if(driveAdjust < -50) driveAdjust = -50;
//
//     double rightPowerA = rightPower;
//     double leftPowerA = rightPower;
//
//     //correct for difference
//     rightPowerA += driveAdjust * rightPowerA * driveC;
//     leftPowerA -= driveAdjust * leftPowerA * driveC;
//
//     if(rightPowerA > 127) { leftPowerA -= rightPowerA - 127; rightPowerA = 127; }
//     if(rightPowerA < -127) { leftPowerA -= rightPowerA + 127; rightPowerA = -127; }
//     if(leftPowerA > 127) { rightPowerA -= leftPowerA - 127; leftPowerA = 127; }
//     if(leftPowerA < -127) { rightPowerA -= leftPowerA + 127; leftPowerA = -127; }
//
//     FR.move(rightPowerA);
//     BR.move(rightPowerA);
//     FL.move(leftPowerA);
//     BL.move(leftPowerA);
//
//     //time based update
//     if(pros::millis() - lastUpdate >= updateSpeed)
//     {
//         double uM = (pros::millis() - lastUpdate) / 1000.0; //for update per second
//
//         //speed
//         rightSpeed = (FR.get_position() - preRightPos) / uM;
//         leftSpeed = (FL.get_position() - preLeftPos) / uM;
//
//         //slew
//         double rightChange = rightPowerPreSlew - rightPower;
//         if(rightChange > driveSlew * uM) rightChange = driveSlew * uM;
//         if(rightChange < -driveSlew * uM) rightChange = -driveSlew * uM;
//         rightPower += rightChange;
//
//         double leftChange = leftPowerPreSlew - leftPower;
//         if(leftChange > driveSlew * uM) leftChange = driveSlew * uM;
//         if(leftChange < -driveSlew * uM) leftChange = -driveSlew * uM;
//         leftPower += leftChange;
//
//         /*//calculate position
//         if(fabs(rightSpeed - leftSpeed) < 1) //strait
//         {
//             double avgSpeed = (rightSpeed + leftSpeed) / 2;
//             double distance = (avgSpeed / ticksPerRevolution) * gearRatio * wheelCircumference * uM;
//
//             robotX += sin(robotDir) * distance;
//             robotY += cos(robotDir) * distance;
//         }
//         else //curve
//         {
//             double rightDistance = (rightSpeed / ticksPerRevolution) * gearRatio * wheelCircumference * uM;
//             double leftDistance = (leftSpeed / ticksPerRevolution) * gearRatio * wheelCircumference * uM;
//
//             double centerOffset = (rightDistance * wheelWidth) / (leftDistance - rightDistance);
//
//             double circleCenterX = robotX + sin(robotDir - PI * 0.5) * centerOffset + wheelWidth * 0.5;
//             double circleCenterY = robotY + cos(robotDir - PI * 0.5) * centerOffset + wheelWidth * 0.5;
//
//             double angle = (rightDistance / (TAU * centerOffset)) * TAU;
//             if(fabs(rightDistance * 3) < fabs(leftDistance)) angle = (leftDistance / (TAU * (centerOffset + wheelWidth))) * TAU;
//
//             robotX = circleCenterX + sin(robotDir + angle) * centerOffset + wheelWidth * 0.5;
//             robotY = circleCenterY + cos(robotDir + angle) * centerOffset + wheelWidth * 0.5;
//
//             robotDir += angle;
//         }*/
//
//         //update values
//         preRightPos = FR.get_position();
//         preLeftPos = FL.get_position();
//
//         lastUpdate = pros::millis();
//     }
// }

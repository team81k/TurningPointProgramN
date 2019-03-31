#include "main.h"
#include "globals.hpp"
#include "pid.hpp"

bool arcadeDrive = false;
double flywheelSpeed = 0;
double flywheelPower = 0;
bool launch = false;
long launchStart = 0;

void opcontrol()
{
	setNavigation(true);
	setPage(0);

	flywheel.set_brake_mode(MOTOR_BRAKE_BRAKE);

	while(true)
	{
		//Drive
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) arcadeDrive = false;
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) arcadeDrive = true;

		if(arcadeDrive)
		{
			rightSide.setPowerPreSlew(master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y)
				- master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X));
			leftSide.setPowerPreSlew(master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y)
				+ master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X));
		}
		else
		{
			rightSide.setPowerPreSlew(master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y));
			leftSide.setPowerPreSlew(master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y));
		}

		double differentialPower = differentialPID.calculate(differentialPot.get_value());

		double FRP = rightSide.calculate() + differentialPower;
		double BRP = rightSide.getPower() - differentialPower;
		double FLP = leftSide.calculate() + differentialPower;
		double BLP = leftSide.getPower() - differentialPower;

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

		//Transform
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1)) differentialPID.target = DIFFERENTIAL_UP;
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2)) differentialPID.target = DIFFERENTIAL_DOWN;

		//Flywheel / Intake
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) flywheelSpeed = 60;
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_A)) flywheelSpeed = 80;
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_B)) flywheelSpeed = 100;
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y)) flywheelSpeed = 0;

		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_UP)) intake.move(100);
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT)) intake.move(0);
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT)) intake.move(-100);
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN))
		{
			launch = true;
			launchStart = pros::millis();
		}

		flywheelPID1.target = flywheelSpeed * 6;
		flywheelPID2.target = flywheelSpeed * 6;
		double flywheelActualSpeed = flywheel.get_actual_velocity();
		flywheelPower = flywheelPID1.calculate(flywheelActualSpeed);
		if(fabs(flywheelPID1.getError()) < 0.2 * 600)
		{
			flywheelPower = flywheelPID2.calculate(flywheelActualSpeed) + flywheelSpeed * 1.2;
			flywheelPID1.setPower(flywheelPower);
		}

		if(flywheelPower > 127) flywheelPower = 127;
		if(flywheelPower < -127) flywheelPower = -127;

		if(launch && pros::millis() - launchStart < 50) flywheel.move(0);
		else
		{
			flywheel.move(flywheelPower);
			//flywheel.move_velocity(flywheelSpeed * 6);
		}

		//Display values
		sprintf(buffer,
			"pot: %i\n"
			"robot: (%f, %f)\n"
			"angle: %fdeg\n"
			"flywheelPower: %f\n"
			"speed: %f\n"
			"flywheelSpeed: %f"
			, differentialPot.get_value()
			, robotX, robotY
			, robotDir * 180.0 / PI
			, flywheelPower
			, flywheel.get_actual_velocity() / 6
			, flywheelSpeed);
		lv_label_set_text(homeTextObject, buffer);

		//pages
		while(activePage == 1)
		{
			pros::delay(3);
		}

		while(activePage == 2 && runAutonomous == -1)
		{
			if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) runAutonomous = 0;
			if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)) runAutonomous = 1;
			if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)) runAutonomous = 2;

			pros::delay(3);
		}

		if(runAutonomous != -1)
		{
			setNavigation(false);
			setPage(0);

			char * autonTypeString = (char*)"";
			if(runAutonomous == 0) autonTypeString = (char*)"15 seconds";
			if(runAutonomous == 1) autonTypeString = (char*)"60 seconds";
			if(runAutonomous == 2) autonTypeString = (char*)"unlimited";

			pros::task_t autonTask = pros::c::task_create([](void * param){autonomous();},
				(void*)"PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "AutonomousTask");

			long autonStartTime = pros::millis();

			while(true)
			{
				long elapsedTime = pros::millis() - autonStartTime;

				if(runAutonomous == 0 && elapsedTime > 15000) break;
				if(runAutonomous == 1 && elapsedTime > 60000) break;

				if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X)) break;

				sprintf(buffer, "Press (X) to stop\n\n"
				"Auton Type: %s\n"
				"Time Elapsed: %i",
				autonTypeString,
				(int)(elapsedTime / 1000));
				lv_label_set_text(homeTextObject, buffer);

				pros::delay(3);
			}

			if(pros::c::task_get_state(autonTask) != TASK_STATE_DELETED) pros::c::task_delete(autonTask);

			runAutonomous = -1;

			setNavigation(true);
			setPage(0);
		}

		//Delay
		pros::delay(3);
	}
}

#include "main.h"
#include "globals.hpp"
#include "pid.hpp"

bool arcadeDrive = false;

void opcontrol()
{
	setNavigation(true);
	setPage(0);

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

		/*if((fabs(differentialPID.getError()) < 25))
		{
			FR.move(rightSide.calculate());
			BR.move(rightSide.getPower());
			FL.move(leftSide.calculate());
			BL.move(leftSide.getPower());
		}
		else
		{
			FR.move(differentialPower);
			BR.move(-differentialPower);
			FL.move(differentialPower);
			BL.move(-differentialPower);
		}*/

		//Transform
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1)) differentialPID.target = DIFFERENTIAL_UP;
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2)) differentialPID.target = DIFFERENTIAL_DOWN;

		//Display values
		char buffer[250];
		sprintf(buffer,
			"pot: %i\n"
			"robot: (%f, %f)\n"
			"angle: %fdeg\n"
			, differentialPot.get_value()
			, robotX, robotY
			, robotDir * 180.0 / PI);
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

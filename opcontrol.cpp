#include "main.h"
#include "globals.hpp"
#include "pid.hpp"

bool arcadeDrive = false;
bool shift = false;

void opcontrol()
{
	setNavigation(true);
	setPage(0);

	master.print(0, 0, "fw speed: %f4", flywheelSpeed);

	while(true)
	{
		shift = master.get_digital(pros::E_CONTROLLER_DIGITAL_Y);

		//Drive
		if(shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) arcadeDrive = false;
		if(shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) arcadeDrive = true;

		if(arcadeDrive)
		{
			double rYJoy = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);

			if(shift) rYJoy *= 0.5;

			rightSide.setPowerPreSlew(rYJoy - master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X));
			leftSide.setPowerPreSlew(rYJoy + master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X));
		}
		else
		{
			rightSide.setPowerPreSlew(master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y));
			leftSide.setPowerPreSlew(master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y));
		}

		double differentialPower = differentialPID.calculate(differentialPot.get_value());
		if(differentialStay) differentialPower = 0;

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

		FR.move/*_velocity*/(FRP * (200.0 / 127.0));
		BR.move/*_velocity*/(BRP * (200.0 / 127.0));
		FL.move/*_velocity*/(FLP * (200.0 / 127.0));
		BL.move/*_velocity*/(BLP * (200.0 / 127.0));

		//Transform
		if(shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_L1))
		{
			differentialStay = false;
			differentialPID.setTarget(DIFFERENTIAL_UP);
		}
		if(shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_L2))
		{
			differentialStay = false;
			differentialPID.setTarget(DIFFERENTIAL_DOWN);
		}

		//Lift
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
			int pRYJoy = partner.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);

			if(!shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) liftSetSpeed = -100;
			else if(!shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) liftSetSpeed = 100;
			else if(abs(pRYJoy) > 5) liftSetSpeed = pRYJoy;
			else liftSetSpeed = 0;

			if(liftSetSpeed > 0 && lift.get_position() > 500) liftSetSpeed = (800 - lift.get_position()) * 0.15;
			if(liftSetSpeed < 0 && lift.get_position() < 300) liftSetSpeed = lift.get_position() * -0.15;

			lift.move(liftSetSpeed);

			//if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1) && master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) liftStep = 0;
		}

		//Double Shot
		if(!shift && master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X)) doubleShot = 1;
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y) && master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT)) doubleShot = 0;

		if(doubleShot == 1)
		{
			hoodPID.setTarget(HOOD_TOP_FLAG);
			doubleShot++;
		}
		else if(doubleShot == 2)
		{
			if(fabs(hoodPID.getError()) < 300) doubleShot++;
		}
		else if(doubleShot == 3)
		{
			flywheelLaunchStart = pros::millis();
			doubleShot++;
		}
		else if(doubleShot == 4)
		{
			if(flywheelLaunchStart == -1)
			{
				doubleShotStart = pros::millis();
				doubleShot++;
			}
		}
		else if(doubleShot == 5)
		{
			if(pros::millis() - doubleShotStart > 0) doubleShot++;
		}
		else if(doubleShot == 6)
		{
			hoodPID.setTarget(HOOD_LOW_FLAG);
			doubleShot++;
		}
		else if(doubleShot == 7)
		{
			if(fabs(hoodPID.getError()) < 300) doubleShot++;
		}
		else if(doubleShot == 8)
		{
			flywheelLaunchStart = pros::millis();
			doubleShot = 0;
		}

		//Flywheel / Intake
		if(!shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_A) && !doubleShot) flywheelSpeed = 85;
		if(!shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_B) && !doubleShot) flywheelSpeed = 100;
		if(shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_X) && !doubleShot) flywheelSpeed = 0;

		if(shift && master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A) && !doubleShot) flywheelSpeed += 5;
		if(shift && master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B) && !doubleShot) flywheelSpeed -= 5;

		if(flywheelSpeed > 100) flywheelSpeed = 100;
		if(flywheelSpeed < 0) flywheelSpeed = 0;

		if(preFlywheelSpeed != flywheelSpeed)
		{
			master.print(0, 0, "fw speed: %f4", flywheelSpeed);
			preFlywheelSpeed = flywheelSpeed;
		}

		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_UP) && !doubleShot) intake.move(50);
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT) && !doubleShot) intake.move(0);
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT) && !doubleShot) intake.move(-70);
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN) && !doubleShot)
		{
			flywheelLaunchStart = pros::millis();
		}

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

		//Hood
		if(!shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) hoodPID.setTarget(HOOD_TOP_FLAG);
		if(!shift && master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) hoodPID.setTarget(HOOD_LOW_FLAG);

		hood.move(-hoodPID.calculate(hoodPot.get_value()));

		//Display values
		sprintf(buffer,
			"hood: %i\n",
			hoodPot.get_value());
		lv_label_set_text(homeTextObject, buffer);

		if(pros::millis() - update > 50)
		{
			update = pros::millis();
			lv_chart_set_next(homeChart, series[0], flywheelPower * (100.0 / 127.0));
			lv_chart_set_next(homeChart, series[1], flywheelActualSpeed / 6.0);
			lv_chart_set_next(homeChart, series[2], flywheelSpeed);
		}

		//pages
		while(activePage == 1)
		{
			FR.move(0);
			FL.move(0);
			BR.move(0);
			BL.move(0);
			flywheel.move(0);
			intake.move(0);

			pros::delay(3);
		}

		while(activePage == 2 && runAutonomous == -1)
		{
			FR.move(0);
			FL.move(0);
			BR.move(0);
			BL.move(0);
			flywheel.move(0);
			intake.move(0);

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

				if(master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) break;

				sprintf(buffer, "Press (X) to stop\n\n"
				"Auton Type: %s\n"
				"Time Elapsed: %i",
				autonTypeString,
				(int)(elapsedTime / 1000));
				lv_label_set_text(homeTextObject, buffer);

				pros::delay(3);
			}

			if(pros::c::task_get_state(autonTask) != TASK_STATE_DELETED) pros::c::task_delete(autonTask);
			//if(pros::c::task_get_state(autonomousAsyncTask) != TASK_STATE_DELETED) pros::c::task_delete(autonomousAsyncTask);

			runAutonomous = -1;

			while(master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) pros::delay(3);

			setNavigation(true);
			setPage(0);

			flywheelSpeed = 0;
		    flywheelPower = 0;
		    flywheelLaunchStart = 0;
		}

		//Delay
		pros::delay(3);
	}
}

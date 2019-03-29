#include "main.h"
#include "globals.hpp"
#include "pid.hpp"

lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);

bool arcadeDrive = false;

void opcontrol()
{
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
		double BRP = rightSide.getPower() + differentialPower;
		double FLP = leftSide.calculate() - differentialPower;
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
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1)) differentialPID.target = 4095;
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2)) differentialPID.target = 10;

		//Display values
		char buffer[250];
		sprintf(buffer,
			"pot: %i\n"
			"robot: (%f, %f)\n"
			"angle: %fdeg\n"
			, differentialPot.get_value()
			, robotX, robotY
			, robotDir * 180.0 / PI);
		lv_label_set_text(label, buffer);

		pros::delay(3);
	}
}

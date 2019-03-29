#include "main.h"
#include "globals.hpp"

void setFullscreenPage(lv_obj_t * a){lv_obj_set_hidden(a, false);lv_obj_set_size(a, LV_HOR_RES, LV_VER_RES);
	lv_obj_set_style(a, &lv_style_transp_tight);lv_obj_set_pos(a, 0, 0);}

void initialize()
{
	differentialPID.target = 4095;

	screenStyle.body.main_color = LV_COLOR_MAKE(0, 0, 0);
    screenStyle.body.grad_color = LV_COLOR_MAKE(0, 0, 0);
    screenStyle.text.color = LV_COLOR_MAKE(255, 255, 255);
    lv_obj_set_style(lv_scr_act(), &screenStyle);

	setFullscreenPage(homePage);
	setFullscreenPage(autonomousPage);
	setFullscreenPage(autonomousRunPage);

	lv_obj_t * autonBack = lv_btn_create(autonomousPage, NULL);
	lv_obj_set_size(autonBack, 15, 15);
	lv_obj_set_pos(autonBack, 0, 0);
	lv_obj_t * label = lv_label_create(autonBack, NULL);
	lv_label_set_text(label, SYMBOL_HOME);

	lv_obj_t * label2 = lv_label_create(autonomousPage, NULL);
	lv_label_set_text(label2, "Autonomous");

	lv_obj_t * autonRun = lv_btn_create(autonomousPage, NULL);
	lv_cont_set_fit(autonRun, true, false);
	lv_obj_set_size(autonRun, 15, 15);
	lv_obj_set_pos(autonRun, 0, 0);
	lv_obj_align(autonRun, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_t * label3 = lv_label_create(autonRun, NULL);
	lv_label_set_text(label3, "Run");
}

void disabled() {}

void competition_initialize() {}

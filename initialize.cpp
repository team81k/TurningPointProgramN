#include "main.h"
#include "globals.hpp"

struct color_t
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

void btnSetToggled(lv_obj_t * btn, bool toggled)
	{ if(toggled != (lv_btn_get_state(btn) >= 2)) lv_btn_toggle(btn); }

lv_res_t btn_click_action(lv_obj_t * btn)
{
	if(btn == autonomousPage[1] || btn == autonomousRunPage[1]) setPage(0);
	if(btn == homePage[2] || btn == autonomousRunPage[2]) setPage(1);
	if(btn == autonomousPage[2]) setPage(2);

	if(btn == autonNone || btn == autonSides || btn == autonSkills)
	{
		btnSetToggled(autonNone, btn == autonNone);
		btnSetToggled(autonSides, btn == autonSides);
		btnSetToggled(autonSkills, btn == autonSkills);

		lv_obj_set_hidden(autonNonePage, btn != autonNone);
		lv_obj_set_hidden(autonSidesPage, btn != autonSides);
		lv_obj_set_hidden(autonSkillsPage, btn != autonSkills);

		if(btn == autonNone) autonType = 0;
		if(btn == autonSides) autonType = 1;
		if(btn == autonSkills) autonType = 2;
	}

	if(btn == autonSideColor)
	{
		lv_obj_t * label = lv_obj_get_child(autonSideColor, NULL);
		autonRed = true;
		char * newTitle = (char*)"Red";
		lv_color_t newColor = LV_COLOR_MAKE(200, 0, 0);
		if(strcmp(lv_label_get_text(label), "Red") == 0)
			{ newTitle = (char*)"Blue";newColor = LV_COLOR_MAKE(0, 0, 200); autonRed = false; }
		lv_label_set_text(label, newTitle);
		for(int i = 0; i < 4; i++)
		{
			lv_style_t * style = lv_btn_get_style(autonSideColor, (lv_btn_style_t)i);
			if(i % 2 == 0) style->body.main_color = style->body.grad_color = newColor;
			else style->body.main_color = style->body.grad_color = {
				(uint8_t)std::fmin(newColor.blue + 50, 255),
				(uint8_t)std::fmin(newColor.green + 50, 255),
				(uint8_t)std::fmin(newColor.red + 50, 255)};
		}
	}

	if(btn == autonSideDistance)
	{
		lv_obj_t * label = lv_obj_get_child(autonSideDistance, NULL);
		autonNear = true;
		char * newTitle = (char*)"Near";
		if(strcmp(lv_label_get_text(label), "Near") == 0)
			{ newTitle = (char*)"Far"; autonNear = false; }
		lv_label_set_text(label, newTitle);
	}

	if(btn == autonSidePlatform)
	{
		lv_obj_t * label = lv_obj_get_child(autonSidePlatform, NULL);
		autonPlatform = true;
		char * newTitle = (char*)"Platform: Yes";
		if(strcmp(lv_label_get_text(label), "Platform: Yes") == 0)
			{ newTitle = (char*)"Platform: No"; autonPlatform = false; }
		lv_label_set_text(label, newTitle);
	}

	if(btn == autonSideColor || btn == autonSideDistance || btn == autonSidePlatform)
	{
		lv_label_set_text(autonSideDescription, generateSidesDescription().c_str());
	}

	if(btn == autonRunNormal) runAutonomous = 0;
	if(btn == autonRunSkills) runAutonomous = 1;
	if(btn == autonRunUnlimited) runAutonomous = 2;

	if(autonType == 0) lv_label_set_text(autonRunDescription, "The robot will do nothing.");
	if(autonType == 1) lv_label_set_text(autonRunDescription, generateSidesDescription().c_str());
	if(autonType == 2) lv_label_set_text(autonRunDescription, "The robot will do skills.");

	return LV_RES_OK;
}

lv_obj_t * createLabel(lv_obj_t * parent, const char * text,
	lv_coord_t x = INT16_MAX, lv_coord_t y = INT16_MAX, lv_align_t align = LV_ALIGN_IN_TOP_LEFT,
	lv_coord_t width = INT16_MAX, lv_coord_t height = INT16_MAX, lv_label_align_t textalign = LV_LABEL_ALIGN_CENTER)
{
	lv_obj_t * label = lv_label_create(parent, NULL);
	lv_label_set_text(label, text);
	lv_label_set_recolor(label, true);
	lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label, textalign);
	if(x != INT16_MAX && y != INT16_MAX) lv_obj_align(label, NULL, align, x ,y);
	if(width != INT16_MAX) lv_obj_set_width(label, width);
	if(height != INT16_MAX) lv_obj_set_height(label, height);
	return label;
}

lv_obj_t * createButton(lv_obj_t * parent, const char * title, color_t color, color_t grad_color,
	lv_coord_t w, lv_coord_t h, lv_coord_t x, lv_coord_t y, lv_align_t align = LV_ALIGN_IN_TOP_LEFT)
{
	lv_style_t * style[4];
	for(int i = 0; i < 4; i++)
	{
		style[i] = (lv_style_t*)malloc(sizeof(lv_style_t));
		lv_style_copy(style[i], &lv_style_plain);
		style[i]->text.color = LV_COLOR_WHITE;
		if(i % 2 == 0)
		{
			style[i]->body.main_color = LV_COLOR_MAKE(color.r, color.g, color.b);
			style[i]->body.grad_color = LV_COLOR_MAKE(grad_color.r, grad_color.g, grad_color.b);
		}
		else
		{
			style[i]->body.main_color = {
				(uint8_t)std::fmin(color.b + 50, 255),
				(uint8_t)std::fmin(color.g + 50, 255),
				(uint8_t)std::fmin(color.r + 50, 255)};
			style[i]->body.grad_color = {
				(uint8_t)std::fmin(grad_color.b + 50, 255),
				(uint8_t)std::fmin(grad_color.g + 50, 255),
				(uint8_t)std::fmin(grad_color.r + 50, 255)};
		}
	}
	style[2]->body.border.color = style[3]->body.border.color = LV_COLOR_WHITE;
	style[2]->body.border.width = style[3]->body.border.width = 5;
	//style[2]->body.border.part = style[3]->body.border.part = LV_BORDER_BOTTOM;
	lv_obj_t * button = lv_btn_create(parent, NULL);
	for(int i = 0; i < 4; i++) lv_btn_set_style(button, (lv_btn_style_t)i, style[i]);
	lv_obj_set_size(button, w, h);
	lv_obj_align(button, NULL, align, x, y);
	createLabel(button, title);
	lv_btn_set_action(button, LV_BTN_ACTION_CLICK, btn_click_action);
	lv_page_glue_obj(button, true);
	return button;
}

lv_obj_t * createButton(lv_obj_t * parent, const char * title, color_t color,
	lv_coord_t w, lv_coord_t h, lv_coord_t x, lv_coord_t y, lv_align_t align = LV_ALIGN_IN_TOP_LEFT)
{
	return createButton(parent, title, color, color, w, h, x, y, align);
}

lv_obj_t * createPage(lv_obj_t * parent, lv_coord_t width, lv_coord_t height, lv_coord_t x, lv_coord_t y,
	bool visible = true, lv_style_t * style = &lv_style_transp_tight)
{
	lv_obj_t * page = lv_page_create(parent, NULL);
	lv_obj_set_size(page, width, height);
	lv_obj_set_pos(page, x, y);
	lv_obj_set_style(page, style);
	lv_obj_set_hidden(page, !visible);
	return page;
}

lv_obj_t * * createPage(lv_obj_t * parent, const char * lButton, const char * title, const char * rButton, lv_coord_t lButtonWidth, lv_coord_t rButtonWidth)
{
	lv_obj_t * * page = (lv_obj_t* *)malloc(sizeof(lv_obj_t) * 3);
	page[0] = createPage(parent, LV_HOR_RES, LV_VER_RES, 0, 0, false);
	lv_obj_t * titleBar = createPage(page[0], LV_HOR_RES, 40, 0, 0);
	page[1] = createButton(titleBar, lButton, {100, 100, 100}, lButtonWidth, 40, 0, 0);
	createLabel(titleBar, title, 0, 0, LV_ALIGN_CENTER);
	page[2] = createButton(titleBar, rButton, {100, 100, 100}, rButtonWidth, 40, 0, 0, LV_ALIGN_IN_RIGHT_MID);
	return page;
}

void initialize()
{
	FR.set_brake_mode(MOTOR_BRAKE_BRAKE);
	FL.set_brake_mode(MOTOR_BRAKE_BRAKE);
	BR.set_brake_mode(MOTOR_BRAKE_BRAKE);
	BL.set_brake_mode(MOTOR_BRAKE_BRAKE);
	flywheel.set_brake_mode(MOTOR_BRAKE_BRAKE);
	differentialPID.target = 4095;
	flywheelPID1.negativeSlew = 127.0 / 5.0;

	screenStyle.body.empty = false;
	screenStyle.body.main_color = LV_COLOR_MAKE(50, 50, 50);
    screenStyle.body.grad_color = LV_COLOR_MAKE(50, 50, 50);
    screenStyle.text.color = LV_COLOR_MAKE(255, 255, 255);
    lv_obj_set_style(lv_scr_act(), &screenStyle);

	homePage = createPage(lv_scr_act(), "PIDTuner", "Home", "Autonomous", 150, 150);
	autonomousPage = createPage(lv_scr_act(), SYMBOL_HOME, "Autonomous", "Run", 100, 100);
	autonomousRunPage = createPage(lv_scr_act(), SYMBOL_HOME, "Autonomous Run", SYMBOL_LEFT" Back", 100, 100);

	setPage(0);

	/*********************************************/
	/*               Home Text View              */
	/*********************************************/
	homeChart = lv_chart_create(homePage[0], NULL);
	lv_obj_set_size(homeChart, LV_HOR_RES, LV_VER_RES - 50);
	lv_obj_set_pos(homeChart, 0, 50);
	lv_chart_set_range(homeChart, -20, 120);
	lv_chart_set_point_count(homeChart, LV_HOR_RES);
	lv_obj_set_style(homeChart, &lv_style_transp);
	lv_chart_set_div_line_count(homeChart, 0, 0);

	seriesZero = lv_chart_add_series(homeChart, LV_COLOR_WHITE);
	lv_chart_init_points(homeChart, seriesZero, 0);

	series[0] = lv_chart_add_series(homeChart, LV_COLOR_RED);
	series[1] = lv_chart_add_series(homeChart, LV_COLOR_GREEN);
	series[2] = lv_chart_add_series(homeChart, LV_COLOR_ORANGE);

	for(int i = 0; i < 3; i++) lv_chart_init_points(homeChart, series[i], 0);

	homeTextPage = createPage(homePage[0], LV_HOR_RES - 10, LV_VER_RES - 50, 5, 45);
	homeTextObject = createLabel(homeTextPage, "", 0, 0, LV_ALIGN_IN_TOP_LEFT, LV_HOR_RES, INT16_MAX, LV_LABEL_ALIGN_LEFT);

	/*********************************************/
	/*        Autonomous Type Page Select        */
	/*********************************************/
	autonTypePage = createPage(autonomousPage[0], LV_HOR_RES, 40, 0, 50, true);

	lv_page_set_scrl_fit(autonTypePage, true, false);
	lv_page_set_scrl_height(autonTypePage, 40);
	lv_page_set_sb_mode(autonTypePage, LV_SB_MODE_OFF);

	autonNone = createButton(autonTypePage, "None", {100, 100, 100}, 100, 40, 0, 0);
	autonSides = createButton(autonTypePage, "Sides", {200, 0, 0}, {0, 0, 200}, 100, 40, 110, 0);
	autonSkills = createButton(autonTypePage, "Skills", {50, 150, 50}, 100, 40, 220, 0);

	btnSetToggled(autonNone, true);

	/*********************************************/
	/*           Autonomous Type Pages           */
	/*********************************************/

	//none page
	autonNonePage = createPage(autonomousPage[0], LV_HOR_RES, LV_VER_RES - 100, 0, 100, true);
	createLabel(autonNonePage, "The robot will do nothing.", 10, 0, LV_ALIGN_IN_TOP_LEFT, LV_HOR_RES - 20);

	//sides page
	autonSidesPage = createPage(autonomousPage[0], LV_HOR_RES, LV_VER_RES - 100, 0, 100, false);

	autonSideColor = createButton(autonSidesPage, "Red", {200, 0, 0}, 100, 40, 0, 0);
	autonSideDistance = createButton(autonSidesPage, "Near", {100, 100, 100}, 100, 40, 110, 0);
	autonSidePlatform = createButton(autonSidesPage, "Platform: Yes", {100, 100, 100}, 200, 40, 220, 0);

	autonSideDescription = createLabel(autonSidesPage, generateSidesDescription().c_str(), 10, 50, LV_ALIGN_IN_TOP_LEFT, LV_HOR_RES - 20);

	//skills page
	autonSkillsPage = createPage(autonomousPage[0], LV_HOR_RES, LV_VER_RES - 100, 0, 100, false);
	createLabel(autonSkillsPage, "The robot will do skills.", 10, 0, LV_ALIGN_IN_TOP_LEFT, LV_HOR_RES - 20);

	/*********************************************/
	/*               Autonomous Run              */
	/*********************************************/
	autonRunTypePage = createPage(autonomousRunPage[0], LV_HOR_RES, LV_VER_RES - 50, 0, 50, true);

	autonRunNormal = createButton(autonRunTypePage, "Normal (15s) (A)", {50, 50, 200}, 170, 40, 0, 0);
	autonRunSkills = createButton(autonRunTypePage, "Skills (60s) (B)", {50, 150, 50}, 150, 40, 180, 0);
	autonRunUnlimited = createButton(autonRunTypePage, "Unlimited (Y)", {200, 175, 0}, 150, 40, 340, 0);

	autonRunDescription = createLabel(autonRunTypePage, "The robot will do nothing.", 10, 50, LV_ALIGN_IN_TOP_LEFT, LV_HOR_RES - 20);
}

void disabled()
{
	setNavigation(false);
	setPage(1);
}

void competition_initialize()
{
	setNavigation(false);
	setPage(1);
}

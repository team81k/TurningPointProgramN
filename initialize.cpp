#include "main.h"
#include "globals.hpp"

void btnSetToggled(lv_obj_t * btn, bool toggled)
	{ if(toggled != (lv_btn_get_state(btn) >= 2)) lv_btn_toggle(btn); }

lv_res_t btn_click_action(lv_obj_t * btn)
{
	if(btn == autonTitleHome || btn == autonRunTitleHome) setPage(0);
	if(btn == homeTitleAuton || btn == autonRunTitleBack) setPage(1);
	if(btn == autonTitleRun) setPage(2);

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
		lv_color_t newColor = {0, 0, 200};
		if(strcmp(lv_label_get_text(label), "Red") == 0)
			{ newTitle = (char*)"Blue";newColor = {200, 0, 0}; autonRed = false; }
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
		lv_label_set_text(autonSideDescription, generateSidesDescription());
	}

	return LV_RES_OK;
}

void setFullscreenPage(lv_obj_t * a)
{
	lv_obj_set_hidden(a, true);
	lv_obj_set_size(a, LV_HOR_RES, LV_VER_RES);
	lv_obj_set_style(a, &lv_style_transp_tight);
	lv_obj_set_pos(a, 0, 0);
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

lv_obj_t * createButton(lv_obj_t * parent, const char * title, lv_color_t color, lv_color_t grad_color,
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
			style[i]->body.main_color = color;
			style[i]->body.grad_color = grad_color;
		}
		else
		{
			style[i]->body.main_color = {
				(uint8_t)std::fmin(color.blue + 50, 255),
				(uint8_t)std::fmin(color.green + 50, 255),
				(uint8_t)std::fmin(color.red + 50, 255)};
			style[i]->body.grad_color = {
				(uint8_t)std::fmin(grad_color.blue + 50, 255),
				(uint8_t)std::fmin(grad_color.green + 50, 255),
				(uint8_t)std::fmin(grad_color.red + 50, 255)};
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
	return button;
}

lv_obj_t * createButton(lv_obj_t * parent, const char * title, lv_color_t color,
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

void initialize()
{
	differentialPID.target = 4095;

	screenStyle.body.empty = false;
	screenStyle.body.main_color = LV_COLOR_MAKE(50, 50, 50);
    screenStyle.body.grad_color = LV_COLOR_MAKE(50, 50, 50);
    screenStyle.text.color = LV_COLOR_MAKE(255, 255, 255);
    lv_obj_set_style(lv_scr_act(), &screenStyle);

	homePage = lv_page_create(lv_scr_act(), NULL);
	autonomousPage = lv_page_create(lv_scr_act(), NULL);
	autonomousRunPage = lv_page_create(lv_scr_act(), NULL);

	setFullscreenPage(homePage);
	setFullscreenPage(autonomousPage);
	setFullscreenPage(autonomousRunPage);

	setPage(0);

	/*********************************************/
	/*              Home Title Bar               */
	/*********************************************/
	homeTitlePage = lv_page_create(homePage, NULL);
	lv_obj_set_size(homeTitlePage, LV_HOR_RES, 40);
	lv_obj_set_style(homeTitlePage, &lv_style_transp_tight);

	createButton(homeTitlePage, "PID Tunner", {100, 100, 100}, 150, 40, 0, 0);
	homeTitleText = createLabel(homeTitlePage, "Home", 0, 0, LV_ALIGN_CENTER);
	homeTitleAuton = createButton(homeTitlePage, "Autonomous", {100, 100, 100}, 150, 40, 0, 0, LV_ALIGN_IN_RIGHT_MID);;

	lv_btn_set_action(homeTitleAuton, LV_BTN_ACTION_CLICK, btn_click_action);

	/*********************************************/
	/*               Home Text View              */
	/*********************************************/
	homeTextPage = lv_page_create(homePage, NULL);
	lv_obj_set_size(homeTextPage, LV_HOR_RES - 10, LV_VER_RES - 50);
	lv_obj_set_pos(homeTextPage, 5, 45);
	lv_obj_set_style(homeTextPage, &lv_style_transp_tight);

	homeTextObject = createLabel(homeTextPage, "", 0, 0, LV_ALIGN_IN_TOP_LEFT, LV_HOR_RES, INT16_MAX, LV_LABEL_ALIGN_LEFT);

	/*********************************************/
	/*           Autonomous Title Bar            */
	/*********************************************/
	autonTitlePage = lv_page_create(autonomousPage, NULL);
	lv_obj_set_size(autonTitlePage, LV_HOR_RES, 40);
	lv_obj_set_style(autonTitlePage, &lv_style_transp_tight);

	autonTitleHome = createButton(autonTitlePage, SYMBOL_HOME, {100, 100, 100}, 100, 40, 0, 0, LV_ALIGN_IN_LEFT_MID);
	autonTitleText = createLabel(autonTitlePage, "Autonomous", 0, 0, LV_ALIGN_CENTER);
	autonTitleRun = createButton(autonTitlePage, "Run", {100, 100, 100}, 100, 40, 0, 0, LV_ALIGN_IN_RIGHT_MID);

	lv_btn_set_action(autonTitleHome, LV_BTN_ACTION_CLICK, btn_click_action);
	lv_btn_set_action(autonTitleRun, LV_BTN_ACTION_CLICK, btn_click_action);

	/*********************************************/
	/*        Autonomous Type Page Select        */
	/*********************************************/
	autonTypePage = createPage(autonomousPage, LV_HOR_RES, 40, 0, 50, true);

	lv_page_set_scrl_fit(autonTypePage, true, false);
	lv_page_set_scrl_height(autonTypePage, 40);
	lv_page_set_sb_mode(autonTypePage, LV_SB_MODE_OFF);

	autonNone = createButton(autonTypePage, "None", {100, 100, 100}, 100, 40, 0, 0);
	autonSides = createButton(autonTypePage, "Sides", {0, 0, 200}, {200, 0, 0}, 100, 40, 110, 0);
	autonSkills = createButton(autonTypePage, "Skills", {0, 150, 0}, 100, 40, 220, 0);

	lv_page_glue_obj(autonNone, true);
	lv_page_glue_obj(autonSides, true);
	lv_page_glue_obj(autonSkills, true);

	btnSetToggled(autonNone, true);

	lv_btn_set_action(autonNone, LV_BTN_ACTION_CLICK, btn_click_action);
	lv_btn_set_action(autonSides, LV_BTN_ACTION_CLICK, btn_click_action);
	lv_btn_set_action(autonSkills, LV_BTN_ACTION_CLICK, btn_click_action);

	/*********************************************/
	/*           Autonomous Type Pages           */
	/*********************************************/

	//none page
	autonNonePage = createPage(autonomousPage, LV_HOR_RES, LV_VER_RES - 100, 0, 100, true);
	createLabel(autonNonePage, "The robot will do nothing.", 0, 0, LV_ALIGN_IN_BOTTOM_MID);

	//sides page
	autonSidesPage = createPage(autonomousPage, LV_HOR_RES, LV_VER_RES - 100, 0, 100, false);

	autonSideColor = createButton(autonSidesPage, "Red", {0, 0, 200}, 100, 40, 0, 0);
	autonSideDistance = createButton(autonSidesPage, "Near", {100, 100, 100}, 100, 40, 110, 0);
	autonSidePlatform = createButton(autonSidesPage, "Platform: Yes", {100, 100, 100}, 200, 40, 220, 0);

	lv_btn_set_action(autonSideColor, LV_BTN_ACTION_CLICK, btn_click_action);
	lv_btn_set_action(autonSideDistance, LV_BTN_ACTION_CLICK, btn_click_action);
	lv_btn_set_action(autonSidePlatform, LV_BTN_ACTION_CLICK, btn_click_action);

	autonSideDescription = createLabel(autonSidesPage, generateSidesDescription(), 0, 50, LV_ALIGN_IN_TOP_MID);

	//skills page
	autonSkillsPage = createPage(autonomousPage, LV_HOR_RES, LV_VER_RES - 100, 0, 100, false);
	createLabel(autonSkillsPage, "The robot will do skills.", 0, 0, LV_ALIGN_IN_BOTTOM_MID);

	/*********************************************/
	/*         Autonomous Run Title Bar          */
	/*********************************************/
	autonRunTitlePage = lv_page_create(autonomousRunPage, NULL);
	lv_obj_set_size(autonRunTitlePage, LV_HOR_RES, 40);
	lv_obj_set_style(autonRunTitlePage, &lv_style_transp_tight);

	autonRunTitleHome = createButton(autonRunTitlePage, SYMBOL_HOME, {100, 100, 100}, 100, 40, 0, 0, LV_ALIGN_IN_LEFT_MID);
	autonRunTitleText = createLabel(autonRunTitlePage, "Autonomous Run", 0, 0, LV_ALIGN_CENTER);
	autonRunTitleBack = createButton(autonRunTitlePage, SYMBOL_LEFT" Back", {100, 100, 100}, 100, 40, 0, 0, LV_ALIGN_IN_RIGHT_MID);

	lv_btn_set_action(autonRunTitleHome, LV_BTN_ACTION_CLICK, btn_click_action);
	lv_btn_set_action(autonRunTitleBack, LV_BTN_ACTION_CLICK, btn_click_action);
}

void disabled()
{
	setAutonomousNav(false);
	setPage(1);
}

void competition_initialize()
{
	setAutonomousNav(false);
	setPage(1);
}

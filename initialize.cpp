#include "main.h"
#include "globals.hpp"

lv_style_t autonTitlePageStyle;
lv_obj_t * autonTitlePage, * autonTitleHome, * autonTitleText, * autonTitleRun;

lv_obj_t * container;

void setFullscreenPage(lv_obj_t * a){lv_obj_set_hidden(a, true);lv_obj_set_size(a, LV_HOR_RES, LV_VER_RES);
	lv_obj_set_style(a, &lv_style_transp_tight);lv_obj_set_pos(a, 0, 0);}

lv_obj_t * createButton(lv_obj_t * parent, const char * title, lv_color_t unpressed, lv_color_t pressed,
	lv_coord_t w, lv_coord_t h/*, lv_align_t align, lv_coord_t x, lv_coord_t y*/)
{
	lv_style_t * unpressedStyle = (lv_style_t*)malloc(sizeof(lv_style_t));
	lv_style_copy(unpressedStyle, &lv_style_plain);
	unpressedStyle->text.color = LV_COLOR_WHITE;
	unpressedStyle->body.main_color = unpressed;
	unpressedStyle->body.grad_color = unpressed;
	lv_style_t * pressedStyle = (lv_style_t*)malloc(sizeof(lv_style_t));
	lv_style_copy(pressedStyle, &lv_style_plain);
	pressedStyle->text.color = LV_COLOR_WHITE;
	pressedStyle->body.main_color = pressed;
	pressedStyle->body.grad_color = pressed;
	lv_obj_t * button = lv_btn_create(parent, NULL);
	lv_btn_set_style(button, LV_BTN_STYLE_REL, unpressedStyle);
	lv_btn_set_style(button, LV_BTN_STYLE_PR, pressedStyle);
	lv_obj_set_size(button, w, h);
	//lv_obj_align(button, NULL, align, x ,y);
	lv_label_set_text(lv_label_create(button, NULL), title);
	return button;
}

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

	lv_obj_set_hidden(autonomousPage, false);

	//title bar
	lv_style_copy(&autonTitlePageStyle, &lv_style_transp_tight);
	autonTitlePageStyle.body.empty = false;
	autonTitlePageStyle.body.main_color = LV_COLOR_MAKE(50, 50, 50);
	autonTitlePageStyle.body.grad_color = LV_COLOR_MAKE(50, 50, 50);

	autonTitlePage = lv_page_create(autonomousPage, NULL);
	lv_obj_set_size(autonTitlePage, LV_HOR_RES, 40);
	lv_obj_set_style(autonTitlePage, &autonTitlePageStyle);

	lv_obj_align(autonTitleHome = createButton(autonTitlePage, SYMBOL_HOME,
		LV_COLOR_MAKE(100, 100, 100), LV_COLOR_MAKE(150, 150, 150),
		100, 40), NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);

	autonTitleText = lv_label_create(autonTitlePage, NULL);
	lv_label_set_text(autonTitleText, "Autonomous");
	lv_obj_align(autonTitleText, NULL, LV_ALIGN_CENTER, 0, 0);

	lv_obj_align(autonTitleRun = createButton(autonTitlePage, "Run",
		LV_COLOR_MAKE(100, 100, 100), LV_COLOR_MAKE(150, 150, 150),
		100, 40), NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);

	//container
	container = lv_page_create(autonomousPage, NULL);
	lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES - 40);
	lv_obj_set_pos(container, 0, 40);
	lv_obj_set_style(container, &autonTitlePageStyle);

	createButton(container, "", LV_COLOR_WHITE, LV_COLOR_WHITE, 0, 0);

	lv_obj_set_pos(createButton(container, "None",
		LV_COLOR_MAKE(100, 100, 100), LV_COLOR_MAKE(150, 150, 150),
		100, 40), 0, 10);

	lv_obj_set_pos(createButton(container, "Red",
		LV_COLOR_MAKE(200, 0, 0), LV_COLOR_MAKE(250, 0, 0),
		100, 40), 110, 10);

	lv_obj_set_pos(createButton(container, "Blue",
		LV_COLOR_MAKE(0, 0, 200), LV_COLOR_MAKE(0, 0, 250),
		100, 40), 220, 10);
}

void disabled() {}

void competition_initialize() {}

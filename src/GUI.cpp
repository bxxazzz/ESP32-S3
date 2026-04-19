#include "GUI.h"
#include "LCD.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef LVGL_USE
    #include "lvgl.h"
#endif

uint8_t DSP_LATCH;
uint8_t BG_COLOR;

// ■■■■■■■■■■■■■■■■ LVGL ■■■■■■■■■■■■■■■■
// lv_screen_active : 현재 보여지는 화면.

#ifdef LVGL_USE
    // Label Object.
    lv_obj_t *label = NULL;
    // Button Object.
    static lv_style_t button_style;
    lv_obj_t *button_red = NULL;
    lv_obj_t *button_blue = NULL;
    lv_obj_t *button_green = NULL;
    lv_obj_t *label_red = NULL;
    lv_obj_t *label_blue = NULL;
    lv_obj_t *label_green = NULL;      

    void UI_INIT()
    {
        label = lv_label_create(lv_screen_active());
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
    }

    void DRAW_LABEL(uint32_t color, const char *text)
    {        
        lv_label_set_text(label, text);
        lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    }

    void DISPLAY_MAIN() 
    {
        if(DSP_LATCH == 0) 
        {
            button_red   = lv_button_create(lv_screen_active());
            button_blue  = lv_button_create(lv_screen_active());
            button_green = lv_button_create(lv_screen_active());
            label_red    = lv_label_create(button_red);
            label_blue   = lv_label_create(button_blue);
            label_green  = lv_label_create(button_green);

            // if     (BG_COLOR == 1) lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFC0303), 0);
            //else if(BG_COLOR == 2)  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x0352FC), 0);
            //else if(BG_COLOR == 3)  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x03FC30), 0);
           // else                    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), 0);

           lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), 0);

            // 스타일 초기화
            lv_style_init(&button_style);
            lv_style_set_radius(&button_style, 10);
            lv_style_set_border_width(&button_style, 2);

            // RED 버튼 설정
            lv_style_set_bg_color(&button_style, lv_color_hex(0xFC0303));
            lv_obj_add_style(button_red, &button_style, 0);
            lv_label_set_text(label_red, "RED");
            lv_obj_center(label_red);
            lv_obj_set_size(button_red, 200, 200);
            lv_obj_set_pos(button_red, 50, 120);

            lv_obj_set_style_bg_color(button_blue, lv_color_hex(0x0352FC), 0); 
            lv_label_set_text(label_blue, "BLUE");
            lv_obj_center(label_blue);
            lv_obj_set_size(button_blue, 200, 200);
            lv_obj_set_pos(button_blue, 300, 120);

            lv_obj_set_style_bg_color(button_green, lv_color_hex(0x03FC30), 0);
            lv_label_set_text(label_green, "GREEN");
            lv_obj_center(label_green);
            lv_obj_set_size(button_green, 200, 200);
            lv_obj_set_pos(button_green, 550, 120);

            DSP_LATCH = 1;
        }

        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
#endif
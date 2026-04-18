#include "LCD.h"
#include "CH422G.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

// Log NAME.
static const char *TAG = "LCD";

// 패널 & 프레임버퍼 초기화.
static esp_lcd_panel_handle_t LCD_PANEL = NULL;
static uint16_t *FRAME_1 = NULL;
static uint16_t *FRAME_2 = NULL;

// 현재 프레임.
static uint8_t FRAME_INDEX = 0;

// 프레임 변경.
#define CHANGE_FRAME    FRAME_INDEX ^= 1


#define LCD_WIDTH   1024
#define LCD_HEIGHT  600

#define RGB565(r, g, b)  (uint16_t)((((r) & 0x1F) << 11) | (((g) & 0x3F) << 5) | ((b) & 0x1F))

void LCD_INIT(void)
{
    // ESP32에서 LCD 제어를 위한 구조체 선언.
    /*
        1. Pixel Clock
        2. H/V Resolution
        3. HSYNC/VSYNC/DE/PCLK 핀 설정
        4. RGB Data핀 설정              
        5. Framebuffer
        6. bounce buffer                ==> RGB 전송 안정을 위한 중간 버퍼.
        7. PSRAM                        ==> 80MHz
    */
    esp_lcd_rgb_panel_config_t config = {};

    // Reset Low.
    CH422G_SET(3, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    // Reset High.
    CH422G_SET(3, 1);
    // 최소 딜레이 120ms.
    vTaskDelay(pdMS_TO_TICKS(120));

    config.data_width = 16;
    config.bits_per_pixel = 16;
    config.clk_src = LCD_CLK_SRC_PLL160M;

    config.pclk_gpio_num = 7;
    config.vsync_gpio_num = 3;
    config.hsync_gpio_num = 46;
    config.de_gpio_num = 5;
    config.disp_gpio_num = -1;

    int BGR[16] = 
    {
        14, 38, 18, 17, 10, 39, 0, 45, 
        48, 47, 21, 1, 2, 42, 41, 40
    };

    for (int i = 0; i < 16; i++) config.data_gpio_nums[i] = BGR[i];

    // 데모보드 기준 작성.
    config.timings.pclk_hz = 21 * 1000 * 1000;
    config.timings.h_res = 1024;
    config.timings.v_res = 600;

    config.timings.hsync_pulse_width = 30;
    config.timings.hsync_back_porch = 145;
    config.timings.hsync_front_porch = 170;
    config.timings.vsync_pulse_width = 2;
    config.timings.vsync_back_porch = 23;
    config.timings.vsync_front_porch = 12;

    config.timings.flags.pclk_active_neg = 1;
    config.timings.flags.hsync_idle_low = 0;
    config.timings.flags.vsync_idle_low = 0;
    config.timings.flags.de_idle_high = 0;    

    config.flags.fb_in_psram = 1;

    // double frame buffer.
    config.num_fbs = 2;             
    config.sram_trans_align = 4;
    config.psram_trans_align = 64;
    config.bounce_buffer_size_px = 1024 * 10;

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&config, &LCD_PANEL));
    ESP_ERROR_CHECK(esp_lcd_panel_init(LCD_PANEL));
    
    // PSRAM에 프레임버퍼 할당.ssssssssssssssss
    // Double frame buffer.
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(LCD_PANEL, 2, (void **)&FRAME_1, (void **)&FRAME_2));
    // Single frame buffer.
    // framebuffer = (uint16_t *)heap_caps_malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    // 백라이트 ON.
    CH422G_SET(2, 1);

    printf("LCD Init... \r\n");
}

/*
    1. ESP_LCD_NEW_RGB_PANEL()      ==>     패널 생성.
    2. ESP_LCD_PANEL_INIT()         ==>     해당 패널 초기화.
    3. PSRAM에 1024x600 버퍼 할당.
    4. 버퍼에 데이터 채움.
    5. ESP_LCD_PANEL_DRWA_BITMAP()  ==>     LCD패널에 데이터 올림.
*/

// ■■■■■■■■■■■■■■■■■■■■■■■■■■■ DOUBLE FRAME BUFFER ■■■■■■■■■■■■■■■■■■■■■■■■■■■
static uint16_t *GET_FRAMEBUFFER()
{
    if(FRAME_INDEX == 0) return FRAME_1;
    else                 return FRAME_2;
}

static void LCD_FILL_BUFFER(uint16_t *buffer, uint16_t color)
{
    if(buffer == NULL) return;

    uint32_t pixel = LCD_WIDTH * LCD_HEIGHT;
    uint32_t color_data = ((uint32_t)color << 16) | color;
    uint32_t *buffer_32 = (uint32_t *)buffer;
    uint32_t i;

    for(i=0; i<pixel / 2; i++) buffer_32[i] = color_data;
}

void LCD_FILL(uint8_t R, uint8_t G, uint8_t B)
{
    uint16_t *buffer = GET_FRAMEBUFFER();

    uint16_t color;
    uint32_t send_color;   
    uint32_t i, j;

    // 생성된 거 없으면 그냥 나감.
    if(LCD_PANEL == NULL || FRAME_1 == NULL || FRAME_2 == NULL) return;

    color = RGB565(R, G, B);
    LCD_FILL_BUFFER(buffer, color);
    esp_lcd_panel_draw_bitmap(LCD_PANEL, 0, 0, LCD_WIDTH, LCD_HEIGHT, buffer);

    CHANGE_FRAME;
}

// ■■■■■■■■■■■■■■■■■■■■■■■■■■■ SINGLE FRAME BUFFER ■■■■■■■■■■■■■■■■■■■■■■■■■■■
/*
void LCD_FILL(uint8_t R, uint8_t G, uint8_t B)
{
    uint16_t color;
    uint32_t i;

    if(framebuffer == NULL) 
    {
        printf("framebuffer NULL... \r\n");
        return;
    }
    else
    {
        color = RGB565(R, G, B);

        for(i=0; i<LCD_WIDTH * LCD_HEIGHT; i++) framebuffer[i] = color;
        esp_lcd_panel_draw_bitmap(LCD_PANEL, 0, 0, LCD_WIDTH, LCD_HEIGHT, framebuffer);
    }
}
*/
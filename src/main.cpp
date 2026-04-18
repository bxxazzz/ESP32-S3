/* Definitions for error constants. */
// #define ESP_OK          0       /*!< esp_err_t value indicating success (no error) */
// #define ESP_FAIL        -1      /*!< Generic esp_err_t code indicating failure */

// #define ESP_ERR_NO_MEM              0x101   /*!< Out of memory */
// #define ESP_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
// #define ESP_ERR_INVALID_STATE       0x103   /*!< Invalid state */
// #define ESP_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
// #define ESP_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
// #define ESP_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
// #define ESP_ERR_TIMEOUT             0x107   /*!< Operation timed out */
// #define ESP_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
// #define ESP_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
// #define ESP_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
// #define ESP_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */
// #define ESP_ERR_NOT_FINISHED        0x10C   /*!< Operation has not fully completed */
// #define ESP_ERR_NOT_ALLOWED         0x10D   /*!< Operation is not allowed */

// #define ESP_ERR_WIFI_BASE           0x3000  /*!< Starting number of WiFi error codes */
// #define ESP_ERR_MESH_BASE           0x4000  /*!< Starting number of MESH error codes */
// #define ESP_ERR_FLASH_BASE          0x6000  /*!< Starting number of flash error codes */
// #define ESP_ERR_HW_CRYPTO_BASE      0xc000  /*!< Starting number of HW cryptography module error codes */
// #define ESP_ERR_MEMPROT_BASE        0xd000  /*!< Starting number of Memory Protection API error codes */

// FreeRTOS 사용을 위한 필수 Header. 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Queue => FIFO.
#include "freertos/queue.h"

#include "CH422G.h"
#include "LCD.h"
#include "GT911.h"

// Queue Handle.
QueueHandle_t MAIN_QUEUE;
int16_t SI_MAIN[10];

// FreeRTOS.
// Core 0 : 통신.
// Core 1 : 화면 및 터치.
// Task는 10개 내외로 구성하는게 유리함.

// ■■■■■■■■■■■■■■■■ Core 0 : Test... ■■■■■■■■■■■■■■■■
static void TASK_COMM(void *pvParameters)
{

}

static void TASK_WIFI(void *pvParameters)
{
  
}

// ■■■■■■■■■■■■■■■■ Core 1 : LCD / Touch ■■■■■■■■■■■■■■■■
// void *pvParameters  => 타입 없는 포인터, FreeRTOS는 테스크에 어떤 데이터 넘겨줄 지 미리 알수가 없으므로, 일단 주소만 보내줌.
//                     ㄴ 대신, 사용할 땐 반드시 형변환을 거쳐줘야 함.
static void TASK_TOUCH(void *pvParameters)
{
  while(1)
  {
    TOUCH_READ();

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

static void TASK_GUI(void *pvParameters)
{
  uint8_t R, G, B;

  while(1)
  {
    R = (uint8_t)((LCD_TOUCH.x * 31) / 1023);
    G = (uint8_t)((LCD_TOUCH.y * 63) / 599);
    B = 15;

    LCD_FILL(R, G, B);
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}


extern "C" void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 큐 3개 생성, 개별 크기 : SI_MAIN 크기
    MAIN_QUEUE = xQueueCreate(3, sizeof(SI_MAIN));

    CH422G_INIT();
    I2C_SCAN();
    LCD_INIT();
    GT911_INIT();

    vTaskDelay(pdMS_TO_TICKS(50));

    // -------------------------------- LCD 테스트.
    LCD_FILL(255, 0, 0);              // R
    vTaskDelay(pdMS_TO_TICKS(1000));

    LCD_FILL(0, 255, 0);              // G
    vTaskDelay(pdMS_TO_TICKS(1000));

    LCD_FILL(0, 0, 255);              // B
    vTaskDelay(pdMS_TO_TICKS(1000));
    // -------------------------------- LCD 테스트.

    LCD_FILL(0, 0, 0);  

    //xTaskCreatePinnedToCore(TASK_COMM, "CommTask", 2048 * 2, NULL, 20, NULL, 0);
    //xTaskCreatePinnedToCore(TASK_WIFI, "WifiTask", 2048 * 4, NULL, 15, NULL, 0);
    xTaskCreatePinnedToCore(TASK_TOUCH, "TouchTask", 2048 * 1, NULL, 10, NULL, 1);
    xTaskCreatePinnedToCore(TASK_GUI, "GuiTask", 2048 * 6, NULL, 5, NULL, 1);
}
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
#include "WIFI.h"
#include "GUI.h"
#include "lvgl.h"

#include <string.h>
#include "lwip/sockets.h"

#define DEST_PORT 1000

// Queue Handle.
QueueHandle_t MAIN_QUEUE;
int16_t SI_MAIN[10];
uint8_t LCD_REFRESH;

void SEND_UDP_BROADCAST(uint8_t button) 
{
    // 1. UDP 소켓 생성
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        return;
    }

    // 2. 브로드캐스트 권한 활성화.
    int broadcast_permission = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_permission, sizeof(broadcast_permission));
    if (ret < 0) 
    {
        close(sock);
        return;
    }

    // 3. 255.255.255.255 (BROADCAST)로 전송.
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DEST_PORT);

    // 4. 메시지 전송
    const char *payload;

    switch(button)
    {
      case 0: payload = "TOUCH_RED";   break;
      case 1: payload = "TOUCH_BLUE";  break;
      case 2: payload = "TOUCH_GREEN"; break;

      default: break;
    }

    int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    
    // 5. 소켓 닫기.
    close(sock);
}

// FreeRTOS.
// Core 0 : 통신.
// Core 1 : 화면 및 터치.
// Task는 10개 내외로 구성하는게 속도나 메모리에서 유리함.

// ■■■■■■■■■■■■■■■■ Core 0 : Test... ■■■■■■■■■■■■■■■■
static void TASK_COMM(void *pvParameters)
{

}

static void TASK_WIFI(void *pvParameters)
{
  while(1)
  {
    /*
    if(LCD_REFRESH == 0)
    {
      if(WIFI_STATUS == WIFI_CONNECTED)
      {
        LCD_REFRESH = 1;
        LCD_FILL(0, 255, 0);
      }
    }
    */

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ■■■■■■■■■■■■■■■■ Core 1 : LCD / Touch ■■■■■■■■■■■■■■■■
// void *pvParameters  => 타입 없는 포인터, FreeRTOS는 테스크에 어떤 데이터 넘겨줄 지 미리 알수가 없으므로, 일단 주소만 보내줌.
//                     ㄴ 대신, 사용할 땐 반드시 형변환을 거쳐줘야 함.

static void TASK_TOUCH(void *pvParameters)
{
  static uint8_t SEND_LATCH;
  
  while(1)
  {
    TOUCH_READ();

    if((LCD_TOUCH.x > 50) && (LCD_TOUCH.x < 250) && (LCD_TOUCH.y > 140) && (LCD_TOUCH.y < 340) && SEND_LATCH == 0)
    {
      SEND_LATCH = 1;
      SEND_UDP_BROADCAST(0);
      BG_COLOR = 1;
    }
    else if((LCD_TOUCH.x > 300) && (LCD_TOUCH.x < 500) && (LCD_TOUCH.y > 140) && (LCD_TOUCH.y < 340) && SEND_LATCH == 0)
    {
      SEND_LATCH = 1;
      SEND_UDP_BROADCAST(1);
      BG_COLOR = 2;
    }
    else if((LCD_TOUCH.x > 550) && (LCD_TOUCH.x < 750) && (LCD_TOUCH.y > 140) && (LCD_TOUCH.y < 340) && SEND_LATCH == 0)
    {
      SEND_LATCH = 1;
      SEND_UDP_BROADCAST(2);
      BG_COLOR = 3;
    }
    else
    {
      SEND_LATCH = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

static void TASK_LVGL(void *pvParameters)
{
  while(1)
  {
    lv_tick_inc(5);
    lv_timer_handler(); 
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void TASK_GUI(void *pvParameters)
{
  uint8_t R, G, B;

  while(1)
  {
    #ifdef LVGL_USE      
      DISPLAY_MAIN();
    #else
      // -------------------------------- LCD 테스트.
      LCD_FILL(255, 0, 0);              // R
      vTaskDelay(pdMS_TO_TICKS(1000));

      LCD_FILL(0, 255, 0);              // G
      vTaskDelay(pdMS_TO_TICKS(1000));

      LCD_FILL(0, 0, 255);              // B
      vTaskDelay(pdMS_TO_TICKS(1000));
      // -------------------------------- LCD 테스트.

      //R = (uint8_t)((LCD_TOUCH.x * 31) / 1023);
      //G = (uint8_t)((LCD_TOUCH.y * 63) / 599);
      //B = 15;

      //LCD_FILL(R, G, B);
    #endif

    vTaskDelay(pdMS_TO_TICKS(10));
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
    WIFI_INIT();

    #ifdef LVGL_USE
      LVGL_INIT();
      UI_INIT();
    #endif

    vTaskDelay(pdMS_TO_TICKS(50));

    //xTaskCreatePinnedToCore(TASK_COMM, "CommTask", 2048 * 2, NULL, 20, NULL, 0);
    xTaskCreatePinnedToCore(TASK_WIFI, "WifiTask", 2048 * 4, NULL, 20, NULL, 0);
    xTaskCreatePinnedToCore(TASK_TOUCH, "TouchTask", 2048 * 1, NULL, 15, NULL, 1);
    xTaskCreatePinnedToCore(TASK_LVGL, "LvglTask", 2048 * 6, NULL, 10, NULL, 1);
    xTaskCreatePinnedToCore(TASK_GUI, "GuiTask", 2048 * 6, NULL, 5, NULL, 1);
}
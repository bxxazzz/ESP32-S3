// FreeRTOS 사용을 위한 필수 Header. 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "CH422G.h"
#include "LCD.h"

extern "C" void app_main(void)
{
    // 시작 전 딜레이 필수. 아니면 업로드 중 밑에 날아가고 바로 while()로 넘어감.
    vTaskDelay(pdMS_TO_TICKS(2000));

    CH422G_INIT();
    I2C_SCAN();
    LCD_INIT();

    while (1)
    {
      LCD_FILL(255, 0, 0);              // R
      vTaskDelay(pdMS_TO_TICKS(1000));

      LCD_FILL(0, 255, 0);              // G
      vTaskDelay(pdMS_TO_TICKS(1000));

      LCD_FILL(0, 0, 255);              // B
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
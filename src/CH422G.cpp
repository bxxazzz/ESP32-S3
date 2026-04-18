#include "CH422G.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// I2C.
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_SDA_IO    8
#define I2C_MASTER_SCL_IO    9
#define I2C_MASTER_FREQ_HZ   400000 

// CH422G 제어 흐름도.
// 1. 0x24 (제어 가능 레지스터 영역)    |  SET
// 2. 0x38 (출력 레지스터 영역)         |  SET
// 3. 사이에 딜레이는 불필요함.
#define CH422G_CMD_SET       0x24
#define CH422G_CMD_IO        0x38

#define DEFAULT_VALUE        0x1A
#define PIN_OUT_SET          0x01

static uint8_t LCD_BLON      = 0x1E;        //  0 | 0 | 0 | 1 | 1 | 1 | 1 | 0
static uint8_t LCD_BLOFF     = 0x1A;


// 주소 뒤 레지스터 없이 1Byte만 전송.
static esp_err_t CH422_WRITE(uint8_t cmd_addr, uint8_t data) 
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (cmd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t result = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(cmd);

    return result;
}

// I2C Setup.
void CH422G_INIT(void)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) printf("I2C Para Config Fail... \r\n");

    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK) printf("I2C Driver Setup Fail... \r\n");

    vTaskDelay(pdMS_TO_TICKS(100)); 
}

// CH422G EXIO핀 제어를 위한 함수.
void CH422G_SET(uint8_t pin, uint8_t level)
{
    uint8_t DATA;
    DATA = DEFAULT_VALUE;

    if(level == 1) DATA |= (1 << pin);
    else           DATA &= ~(1 << pin);

    if(!(CH422_WRITE(CH422G_CMD_SET, PIN_OUT_SET))) printf("OUTPUT SET... \r\n");
    else                                            printf("OUTPUT FAIL... \r\n");
    
    if(!(CH422_WRITE(CH422G_CMD_IO, DATA)))         printf("DATA SET... \r\n");
    else                                            printf("DATA FAIL... \r\n");
}

// I2C Scan용 함수.
void I2C_SCAN(void)
{
    uint8_t addr;

    printf("I2C SCAN START... \r\n");
    for (addr=1; addr<127; addr++) 
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        i2c_master_start(cmd);
        if (i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true) == ESP_OK) 
        {
            if (i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(10)) == ESP_OK) printf("0x%02X\r\n", addr);
        }

        i2c_cmd_link_delete(cmd);
    }
}
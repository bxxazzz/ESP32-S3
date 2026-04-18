#include "driver/i2c.h"
#include "GT911.h"
#include "CH422G.h"

// I2C.
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_SDA_IO    8
#define I2C_MASTER_SCL_IO    9
#define I2C_MASTER_FREQ_HZ   400000 

// GT911.
#define GT911_ADDR      0x5D

// REGISTER.
#define GT911_ID             0x8140         // 제품 상태 확인.
#define GT911_STATUS         0x814E         // 새로운 터치 발생하면 최상위 비트 SET, 터치 지점 개수.
#define GT911_POINT          0x8150         // 첫번째 터치 좌표 데이터.

TOUCH_POINT LCD_TOUCH;
uint16_t    DELAY_CNT;

static esp_err_t GT911_WRITE(uint8_t cmd_addr, uint16_t reg, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) return ESP_ERR_NO_MEM;

    esp_err_t result = ESP_OK;

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (cmd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, (reg >> 8) & 0xFF, true);
    i2c_master_write_byte(cmd, reg & 0xFF, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);

    result = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    return result;
}

static esp_err_t GT911_READ(uint8_t cmd_addr, uint16_t reg, uint8_t *data, size_t len)
{
    if(!data || len == 0) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if(!cmd) return ESP_ERR_NO_MEM;

    esp_err_t ret = ESP_OK;

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (cmd_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, (reg >> 8) & 0xFF, true);
    i2c_master_write_byte(cmd, reg & 0xFF, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (cmd_addr << 1) | I2C_MASTER_READ, true);

    if(len > 1) i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);

    i2c_master_read_byte(cmd, &data[len - 1], I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t GT911_READID(uint8_t cmd_addr, uint8_t id[5])
{
    uint8_t buf[4];

    esp_err_t result = GT911_READ(cmd_addr, GT911_ID, buf, 4);
    if(result != ESP_OK) return result;

    id[0] = buf[0];
    id[1] = buf[1];
    id[2] = buf[2];
    id[3] = buf[3];
    id[4] = '\0';

    return ESP_OK;
}

esp_err_t GT911_INIT()
{
    uint8_t id[5];
    esp_err_t result;

    result = GT911_READID(GT911_ADDR, id);

    if(result == ESP_OK) 
    {
        printf("GT911 : 0x%02X, ID : %s \r\n", GT911_ADDR, id);
        return ESP_OK;
    }

    printf("GT911 not found... \r\n");
    return ESP_FAIL;
}

// GT911이 800x480 기준으로 좌표를 받아와서, 스케일링 해줌.
static uint16_t TOUCH_SCALE(uint16_t v, uint16_t in_max, uint16_t out_max)
{
    if(in_max == 0) return 0;
    if(v >= in_max) return out_max - 1;

    return (uint32_t)v * (out_max - 1) / (in_max - 1);
}


static esp_err_t GT911_READXY(TOUCH_POINT *t_data)
{
    uint8_t status;
    uint8_t touch_count;
    uint8_t touch_buffer[8];

    if(!t_data) return ESP_ERR_INVALID_ARG;
    
    esp_err_t result = GT911_READ(GT911_ADDR, GT911_STATUS, &status, 1);

    if(result != ESP_OK) return ESP_OK;
        
    // 구조체 초기화.
    t_data -> pressed = false;
    t_data -> count   = 0;
    t_data -> x       = 0;
    t_data -> y       = 0;

    // 새 터치 없으면 나감.
    if((status & 0x80) == 0) return 1;

    touch_count     = status & 0x0F;
    t_data -> count = touch_count;

    if(touch_count > 0) 
    {
        result = GT911_READ(GT911_ADDR, GT911_POINT, touch_buffer, 8);

        if (result != ESP_OK)  return result;

        t_data -> x = (uint16_t)(touch_buffer[1] << 8 | touch_buffer[0]);
        t_data -> y = (uint16_t)(touch_buffer[3] << 8 | touch_buffer[2]);

        // 좌표값 스케일링.
        t_data -> x = TOUCH_SCALE(t_data -> x, 800, 1024);
        t_data -> y = TOUCH_SCALE(t_data -> y, 480, 600);

        if(t_data -> x > 1024) t_data -> x = 1024;
        if(t_data -> y > 600)  t_data -> y = 600;

        // 감지.
        t_data -> pressed = true;
    }

    // 비트 클리어. 안해주면 다음 터치 데이터 못받음.
    result = GT911_WRITE(GT911_ADDR, GT911_STATUS, 0x00);

    return result;
}


void TOUCH_READ()
{
    GT911_READXY(&LCD_TOUCH);

    if(LCD_TOUCH.pressed)
    {
        DELAY_CNT = 0;
        printf("X = %u, Y = %u, C = %u \r\n", LCD_TOUCH.x, LCD_TOUCH.y, LCD_TOUCH.count);
    }
    else
    {
        if(++DELAY_CNT > 200)
        {
            DELAY_CNT = 0;
            printf("No Touch!!! \r\n");
        }
    }
}
#include "debug.h"
#include "driver_ssd1306_interface.h"

uint8_t ssd1306_interface_iic_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    I2C_InitTypeDef I2C_InitStruct;
    I2C_InitStruct.I2C_ClockSpeed = 400000;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStruct);
    I2C_Cmd(I2C1, ENABLE);
    return 0;
}

uint8_t ssd1306_interface_iic_deinit(void)
{
    I2C_Cmd(I2C1, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
    return 0;
}

uint8_t ssd1306_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
    uint32_t timeout = 100000;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if (--timeout == 0) {
            return 1;
        }
    }
    I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
    timeout = 100000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if (--timeout == 0) {
            return 1;
        }
    }
    // §°§é§Ú§ã§ä§Ü§Ñ §æ§Ý§Ñ§Ô§à§Ó
    (void)I2C1->STAR1;
    (void)I2C1->STAR2;
    // §°§ä§á§â§Ñ§Ó§Ü§Ñ §Ñ§Õ§â§Ö§ã§Ñ §â§Ö§Ô§Ú§ã§ä§â§Ñ
    I2C_SendData(I2C1, reg);
    timeout = 100000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if (--timeout == 0) {
            return 1;
        }
    }
    // §°§ä§á§â§Ñ§Ó§Ü§Ñ §Õ§Ñ§ß§ß§í§ç
    for (uint16_t i = 0; i < len; i++) {
        I2C_SendData(I2C1, buf[i]);
        timeout = 100000;
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if (--timeout == 0) {
                return 1;
            }
        }
    }
    // §¤§Ö§ß§Ö§â§Ñ§è§Ú§ñ STOP
    I2C_GenerateSTOP(I2C1, ENABLE);

    return 0;
}

uint8_t ssd1306_interface_spi_init(void) {
    return 0;
}

uint8_t ssd1306_interface_spi_deinit(void) {
    return 0;
}

uint8_t ssd1306_interface_spi_write_cmd(uint8_t *buf, uint16_t len) {
    return 1;
}

void ssd1306_interface_delay_ms(uint32_t ms) {
    Delay_Ms(ms);
}

void ssd1306_interface_debug_print(const char *const fmt, ...) {
    (void)fmt;
}

uint8_t ssd1306_interface_spi_cmd_data_gpio_init(void) {
    return 1;
}

uint8_t ssd1306_interface_spi_cmd_data_gpio_deinit(void) {
    return 1;
}

uint8_t ssd1306_interface_spi_cmd_data_gpio_write(uint8_t value) {
    return 1;
}

uint8_t ssd1306_interface_reset_gpio_init(void) {
    return 0;
}

uint8_t ssd1306_interface_reset_gpio_deinit(void) {
    return 0;
}

uint8_t ssd1306_interface_reset_gpio_write(uint8_t value) {
    return 0;
}

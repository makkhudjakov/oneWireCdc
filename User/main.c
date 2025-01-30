/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"
#include "tusb_config.h"
#include "tusb.h"
#include "dshot.h"

volatile uint32_t system_ticks = 0;

__attribute__((interrupt))
void SysTick_Handler(void) {
  SysTick->SR = 0;
  system_ticks++;
}

uint32_t SysTick_Config(uint32_t ticks) {
  NVIC_EnableIRQ(SysTicK_IRQn);
  SysTick->CTLR = 0;
  SysTick->SR = 0;
  SysTick->CNT = 0;
  SysTick->CMP = ticks - 1;
  SysTick->CTLR = 0xF;
  return 0;
}

uint32_t tusb_time_millis_api(void) {
  return system_ticks;
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+

uint8_t inputBuffer[64];
uint8_t inputBufferPtr = 0;

uint8_t outputBuffer[64];
uint8_t outputBufferPtr = 0;

void cdc_task(void) {
    // connected and there are data available
    if (tud_cdc_available()) {
        // read data
        uint8_t buf[64];
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        if(count > 0)
        {
            memcpy(&inputBuffer[inputBufferPtr], buf, count);
            inputBufferPtr += count;
        }
    }
}


// USBD (fsdev)
__attribute__((used)) __attribute__((interrupt("WCH-Interrupt-fast")))
void USB_LP_CAN1_RX0_IRQHandler(void) {
  #if CFG_TUD_WCH_USBIP_FSDEV
  tud_int_handler(0);
  #endif
}

__attribute__((used)) __attribute__((interrupt("WCH-Interrupt-fast")))
void USB_HP_CAN1_TX_IRQHandler(void) {
  #if CFG_TUD_WCH_USBIP_FSDEV
  tud_int_handler(0);
  #endif

}

__attribute__((used)) __attribute__((interrupt("WCH-Interrupt-fast")))
void USBWakeUp_IRQHandler(void) {
  #if CFG_TUD_WCH_USBIP_FSDEV
  tud_int_handler(0);
  #endif
}


void inputCallback(uint16_t frame) {
    memcpy(&outputBuffer[outputBufferPtr], &frame, sizeof(frame));
    outputBufferPtr += sizeof(frame);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);
//    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    printf("This is printf example\r\n");

    if(SystemCoreClock == 48000000) {
        RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);
    }
    else if(SystemCoreClock == 96000000) {
        RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div2);
    }
    else if(SystemCoreClock == 144000000) {
        RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div3);
    }
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);  // FSDEV
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE); // USB FS

    // init device stack on configured roothub port
    tusb_rhport_init_t dev_init = {
      .role = TUSB_ROLE_DEVICE,
      .speed = TUSB_SPEED_AUTO
    };

    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    setIn();
    setCallback(&inputCallback);

    while (1) {
        tud_task(); // tinyusb device task
        cdc_task();
        if(inputBufferPtr >= 2) {
            for(uint8_t i = 0; i < inputBufferPtr; i += 2) {
                sendFrame(*(uint16_t*)&inputBuffer[i]);
                inputBufferPtr -= sizeof(uint16_t);
                tusb_time_delay_ms_api(1);
            }
        }
        if(outputBufferPtr > 0) {
            tud_cdc_write(outputBuffer, outputBufferPtr);
            tud_cdc_write_flush();
            outputBufferPtr = 0;
        }
    }
}

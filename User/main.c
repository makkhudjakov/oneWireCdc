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
#include "multiplex.h"
#include "motorControl.h"

uint32_t tusb_time_millis_api(void) {
  return millisecondsGet();
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
typedef enum Commands_e {
    Data = 0xAA,
    SetChannel = 0x01
} Commands_t;

uint8_t inputBuffer[64];
uint8_t inputBufferPtr = 0;
static bool writeFlag = false;

void inputCallback(uint16_t frame) {
    tud_cdc_write(&frame, sizeof(frame));
    writeFlag = true;
}

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

    if((inputBufferPtr % 3) == 0 && inputBufferPtr > 0) {
        motorControlDisable();
        uint8_t cmdNum = inputBufferPtr / 3;
        for(uint8_t cmd = 0; cmd < cmdNum; cmd += 3) {
            uint8_t cmdCode = inputBuffer[cmd];
            uint16_t arg;
            memcpy(&arg, &inputBuffer[cmd + 1], sizeof(arg));
            switch (cmdCode) {
                case Data:
                    dshotSendFrame(arg);
                    break;
                case SetChannel:
                    multiplexSetChannel(arg);
                    break;
                default:
                    break;
            }
        }
        inputBufferPtr = 0;
    }

    if(writeFlag) {
        if(tud_cdc_write_flush() > 0) {
            writeFlag = false;
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
    Delay_Init();
//    USART_Printf_Init(115200);
//    printf("SystemClk:%d\r\n", SystemCoreClock);
//    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
//    printf("This is printf example\r\n");

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

    dshotSetIn();
    dshotSetCallback(&inputCallback);
    multiplexInit();
    motorControlInit();

    while (1) {
        tud_task(); // tinyusb device task
        cdc_task();
        motorControlTask();
    }
}

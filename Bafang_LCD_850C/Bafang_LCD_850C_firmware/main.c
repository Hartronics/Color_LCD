/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"

#include "pins.h"
#include "lcd.h"
#include "buttons.h"
#include "timers.h"

#include "ugui/ugui.h"
#include "ugui_driver/ugui_bafang_500c.h"

void SetSysClockTo128Mhz(void);

void system_power (uint32_t ui32_state)
{
  if (ui32_state)
  {
    GPIO_SetBits(SYSTEM_POWER_ON_OFF__PORT, SYSTEM_POWER_ON_OFF__PIN);
  }
  else
  {
    GPIO_ResetBits(SYSTEM_POWER_ON_OFF__PORT, SYSTEM_POWER_ON_OFF__PIN);
  }
}

int main(void)
{
  SetSysClockTo128Mhz();

  RCC_APB1PeriphResetCmd(RCC_APB1Periph_WWDG, DISABLE);

  pins_init();
  system_power(ENABLE);
  systick_init();
  lcd_init();

  uint32_t ui32_timer_base_counter_1ms;
  uint32_t ui32_10ms_loop_counter;
  static buttons_events_type_t events = 0;
  static buttons_events_type_t last_events = 0;

  lcd_draw_main_menu_mask();

  while (1)
  {
    // because of continue; at the end of each if code block that will stop the while (1) loop there,
    // the first if block code will have the higher priority over any others
    ui32_timer_base_counter_1ms = get_timer_base_counter_1ms ();
    if ((ui32_timer_base_counter_1ms - ui32_10ms_loop_counter) > 5) // every 10ms
    {
      ui32_10ms_loop_counter = ui32_timer_base_counter_1ms;

      buttons_clock ();
//      lcd_clock ();
//      uart_data_clock ();

      lcd_draw_main_menu();

      continue;
    }
  }
}

void SetSysClockTo128Mhz(void)
{
  ErrorStatus HSEStartUpStatus;

  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if (HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);

    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1);

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* PLLCLK = 8MHz * 16 = 128 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_16);

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock configuration.
       User can add here some code to deal with this error */

    /* Go to infinite loop */
    while (1)
    {
    }
  }
}

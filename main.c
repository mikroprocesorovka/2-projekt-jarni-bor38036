//1.Rozsvítí se jedna LEDka pokud je napìtí na potenciometru vìtší, než napìtí dìlièe a zhasne LED pokud je tomu naopak.
//2.Poloha potenciometru øídí jas druhé LEDky pomocí PWM (jedna krajní poloha 0% jasu, druhá krajní poloha 100% jasu).
//3.Každou 1 sekundu odesílá na UART informaci o poloze potenciometru v procentech a hodnotì napìtí.

#include "stm8s.h"
#include "milis.h"
#include "stdio.h"
#include "spse_stm8.h"
#include "stm8_hd44780.h" 
#include "swspi.h"
#include "stm8s_adc2.h"

uint32_t adc_value1=0,adc_value2=0;

void uart_putchar(char data); 
void uart_puts(char* retezec); 
char text[32]; 

void init_pwm(void);
void process_pwm_change(void);
void init_adc(void);
void blik(void);
void uart(void);

void main(void){
CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // 16MHz z interního RC oscilátoru

// nastavení UART
UART1_Init(115200,UART1_WORDLENGTH_8D,UART1_STOPBITS_1,UART1_PARITY_NO,UART1_SYNCMODE_CLOCK_DISABLE,UART1_MODE_TX_ENABLE);
UART1_Cmd(ENABLE);

init_milis(); // spustit èasovaè millis
init_adc(); // ADC
init_pwm(); // PWM

// inicializace pinu LEDek
GPIO_Init(GPIOC,GPIO_PIN_1,GPIO_MODE_OUT_PP_LOW_SLOW); 
GPIO_Init(GPIOD,GPIO_PIN_3,GPIO_MODE_OUT_PP_LOW_SLOW);


  while (1){
		adc_value1 = ADC_get(ADC2_CHANNEL_2); 
		adc_value2 = ADC_get(ADC2_CHANNEL_3); 
		
		blik();
		process_pwm_change();
		uart();
	}
}

// rozsvícení a zhasnutí LEDky podle polohy potenciometru
void blik(void){
	if(adc_value2 >= adc_value1){GPIO_WriteHigh(GPIOC,GPIO_PIN_1);}
	else {GPIO_WriteLow(GPIOC,GPIO_PIN_1);}
}

// nastavování jasu LEDky
void process_pwm_change(void){
uint32_t x=0,y=0;	
static uint16_t last_time=0;

  if(milis() - last_time >= 10){ 
		last_time = milis();
		
		x=(adc_value2*100)/1024;
		y=(x*1023)/100;
		TIM2_SetCompare2(y); 
  }
}
// ovládání PWM
void init_pwm(void){
TIM2_TimeBaseInit(TIM2_PRESCALER_1,1023);
	
TIM2_OC2Init(
	TIM2_OCMODE_PWM1, 
	TIM2_OUTPUTSTATE_ENABLE,
	0,
	TIM2_OCPOLARITY_HIGH
	);
	
TIM2_OC2PreloadConfig(ENABLE);
TIM2_Cmd(ENABLE);
}
// ovládání UART
void uart(void){
	uint16_t volty=0;
	uint16_t desetiny=0;
	uint16_t voltage=0;
	uint16_t procenta=0;
	
	static uint16_t posledni_cas=0;	
	uint16_t aktualni_cas;
 
	aktualni_cas = milis();
	
	// každou 1 vteøinu se informace vypíše pøes UART
	if(aktualni_cas - posledni_cas > 1000){
		posledni_cas = aktualni_cas;
		
		voltage = ((uint32_t)adc_value2*500 + 512)/1024; 
		procenta = (voltage*100)/500;
		volty = voltage/100;
		desetiny = voltage%100;
		
		// napsání informace na UART
		sprintf(text,"Hodnota potenciometru v PROCENTECH: %3u \n\r",procenta); 
		uart_puts(text); 
		sprintf(text,"Napeti na potenciometru: %3u,%2u V \n\r",volty,desetiny); 
		uart_puts(text); 
	}
}

void uart_putchar(char data){
 while(UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
	UART1_SendData8(data); 
}

void uart_puts(char* retezec){ 
 while(*retezec){ 
  uart_putchar(*retezec); 
  retezec++; 
 }
}

void init_adc(void){
// na pinech/vstupech ADC_IN2 (PB2) a ADC_IN3 (PB3) vypneme vstupní buffer
ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL2,DISABLE);
ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL3,DISABLE);
// nastavíme clock pro ADC (16MHz / 4 = 4MHz)
ADC2_PrescalerConfig(ADC2_PRESSEL_FCPU_D4);
// volíme zarovnání výsledku (typicky vpravo, jen vyjmeènì je výhodné vlevo)
ADC2_AlignConfig(ADC2_ALIGN_RIGHT);
// nasatvíme multiplexer na nìkterý ze vstupních kanálù
ADC2_Select_Channel(ADC2_CHANNEL_2);
// rozbìhneme AD pøevodník
ADC2_Cmd(ENABLE);
// poèkáme než se AD pøevodník rozbìhne (~7us)
ADC2_Startup_Wait();
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

//1.Rozsv�t� se jedna LEDka pokud je nap�t� na potenciometru v�t��, ne� nap�t� d�li�e a zhasne LED pokud je tomu naopak.
//2.Poloha potenciometru ��d� jas druh� LEDky pomoc� PWM (jedna krajn� poloha 0% jasu, druh� krajn� poloha 100% jasu).
//3.Ka�dou 1 sekundu odes�l� na UART informaci o poloze potenciometru v procentech a hodnot� nap�t�.

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
CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // 16MHz z intern�ho RC oscil�toru

// nastaven� UART
UART1_Init(115200,UART1_WORDLENGTH_8D,UART1_STOPBITS_1,UART1_PARITY_NO,UART1_SYNCMODE_CLOCK_DISABLE,UART1_MODE_TX_ENABLE);
UART1_Cmd(ENABLE);

init_milis(); // spustit �asova� millis
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

// rozsv�cen� a zhasnut� LEDky podle polohy potenciometru
void blik(void){
	if(adc_value2 >= adc_value1){GPIO_WriteHigh(GPIOC,GPIO_PIN_1);}
	else {GPIO_WriteLow(GPIOC,GPIO_PIN_1);}
}

// nastavov�n� jasu LEDky
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
// ovl�d�n� PWM
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
// ovl�d�n� UART
void uart(void){
	uint16_t volty=0;
	uint16_t desetiny=0;
	uint16_t voltage=0;
	uint16_t procenta=0;
	
	static uint16_t posledni_cas=0;	
	uint16_t aktualni_cas;
 
	aktualni_cas = milis();
	
	// ka�dou 1 vte�inu se informace vyp�e p�es UART
	if(aktualni_cas - posledni_cas > 1000){
		posledni_cas = aktualni_cas;
		
		voltage = ((uint32_t)adc_value2*500 + 512)/1024; 
		procenta = (voltage*100)/500;
		volty = voltage/100;
		desetiny = voltage%100;
		
		// naps�n� informace na UART
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
// na pinech/vstupech ADC_IN2 (PB2) a ADC_IN3 (PB3) vypneme vstupn� buffer
ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL2,DISABLE);
ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL3,DISABLE);
// nastav�me clock pro ADC (16MHz / 4 = 4MHz)
ADC2_PrescalerConfig(ADC2_PRESSEL_FCPU_D4);
// vol�me zarovn�n� v�sledku (typicky vpravo, jen vyjme�n� je v�hodn� vlevo)
ADC2_AlignConfig(ADC2_ALIGN_RIGHT);
// nasatv�me multiplexer na n�kter� ze vstupn�ch kan�l�
ADC2_Select_Channel(ADC2_CHANNEL_2);
// rozb�hneme AD p�evodn�k
ADC2_Cmd(ENABLE);
// po�k�me ne� se AD p�evodn�k rozb�hne (~7us)
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

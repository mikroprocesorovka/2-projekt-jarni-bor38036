// Tyto makra upravujte podle potøeby 
//DIN PB5, CS PB4, CLK PB3
#define DIN_GPIO 	GPIOB
#define DIN_PIN 	GPIO_PIN_5
#define CS_GPIO 	GPIOB
#define CS_PIN 		GPIO_PIN_4
#define CLK_GPIO 	GPIOB
#define CLK_PIN 	GPIO_PIN_3

void swspi_init(void);
void swspi_tx16(uint16_t data);

#include "stm32f10x.h"
#include "GLCD.h"
#include "systick.h"

void putChar(char a) {
	static int xpos=0, ypos=20; //initialization is done only once.
	static int i=0;
	char str[2];
	str[0]=a;
	str[1]='\0';
	GUI_Text(xpos, ypos, str, Black, White-i);
	xpos+=15;
	if(xpos>240) {
		ypos+=15;
		xpos=0;
	}
	if(ypos>300)
	{
		xpos=0;
		ypos=20;
		i+=1;
	}
}

void GPIO_init() {

#define USER_KEYA GPIO_Pin_13 //port C
#define USER_KEYB GPIO_Pin_2 //port B
#define LED1 GPIO_Pin_0 //port B
#define LED2 GPIO_Pin_1 //port B

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = USER_KEYB;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //floating becuse they re connected to power through resitatence
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

int main(void) {
	  SystemInit();
	  delay_init();
	  LCD_Initializtion();
	  LCD_BackLight_Init();
	  LCD_Clear(White);
	  GUI_Text(0,0,"PTM morse decoder",Black,White);
	  GUI_Text(45,305,"Artur Zochniak, PWR 2012",Black,White);
	  GPIO_init();
	  /* Infinite loop */
	  while (1) {
	    char i;
		if (0)
			for (i = 100; i > 0; i--) {
				LCD_BackLight(i * 10);
				delay_ms(500);
			}
		else{
			if(GPIOB->IDR & USER_KEYB)
			{
				putChar('a');
				delay_ms(10);
				//while(GPIOB->IDR & USER_KEYB) delay_ms(1);
			}
		}
	  }
}

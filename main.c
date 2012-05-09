#include "stm32f10x.h"
#include "GLCD.h"
#include "systick.h"

void putChar(char a, int replace) {
	static int startxpos=0, startypos=20;
	static int id=0, xpos=0, ypos=20; //initialization is done only once.
	static int i=0;
	char str[2];
    int no_in_row = 240 / 11;
	str[0]=a;
	str[1]='\0';
    ypos = (id / no_in_row)*11+startypos;
    xpos = (id % no_in_row)*11+startxpos;
    GUI_Text(xpos, ypos, str, Black, White-i);
    if(!replace)
        return;
    id += 1;
}

int bar;
void proceedBar(){
	static int barx=140;
	static int bary=10;
	static int barWidth=90;
	if(bar==0){
		bar=barWidth;
		LCD_DrawLine(barx, bary, barx+barWidth, bary, Black);
	} else {
		LCD_SetPoint(barx+bar, bary, White);
		--bar;
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
	  //LCD_DrawLine(20, 46, 120, 46, Black);
	  bar=0;
	  while (1) {
		  char a='a'-1;
		  if(GPIOB->IDR & USER_KEYB) {
			while(GPIOB->IDR & USER_KEYB) {
				a++;
				if(a>'z') a='a';
				putChar(a,0);
				delay_ms(250);
				//while(GPIOB->IDR & USER_KEYB) delay_ms(1);
			}
			putChar(a,1);
		  }
			//else {
			//	drawCircle(0, 0, 100, 100, 50);
				proceedBar();
				delay_ms(10);
			//}

		}
}

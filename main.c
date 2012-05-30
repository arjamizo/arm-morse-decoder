#include "stm32f10x.h"
#include "GLCD.h"
#include "systick.h"
#include <string.h>
//#include <stdio.h>
#include <stdlib.h> //itoa

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "misc.h"



#define USER_KEYA_PORT	GPIOC
#define USER_KEYA		GPIO_Pin_13
#define USER_KEYA_LINE	EXTI_Line13
#define USER_KEYA_PORT_SOURCE GPIO_PortSourceGPIOC
#define USER_KEYA_PIN_SOURCE GPIO_PinSource13

#define USER_KEYB_PORT	GPIOB
#define USER_KEYB 		GPIO_Pin_2
#define USER_KEYB_LINE	EXTI_Line2
#define USER_KEYB_PORT_SOURCE GPIO_PortSourceGPIOB
#define USER_KEYB_PIN_SOURCE GPIO_PinSource2

#define LED1_PIN		GPIO_Pin_0
#define LED1_PORT 		GPIOB
#define LED2_PIN		GPIO_Pin_1
#define LED2_PORT 		GPIOB

void putChar(char a, int replace) {
	static int startxpos = 0, startypos = 20;
	static int id = 0, xpos = 0, ypos = 20; //initialization is done only once.
	static int i = 0;
	static int rowHeight = 14;
	static int letterWidth = 11;
	char str[2];
	int no_in_row = 240 / rowHeight;
	str[0] = a;
	str[1] = '\0';
	ypos = (id / no_in_row) * rowHeight + startypos;
	xpos = (id % no_in_row) * letterWidth + startxpos;
	GUI_Text(xpos, ypos, str, Black, White - i);
	if (!replace)
		return;
	id += 1;
}

int series_total=0;
int series_pressed=0;
void HandlePressed(int *total, int *pressed);

void proceedBar(int isPressed) {
	static int barx = 144;
	static int bary = 4;
	static int barWidth = 90;
	static int bar=0;
	static int which=0;
	int h=5;
	if (bar>barWidth) {
		bar = 0;
		for(;h>0; --h)
			LCD_DrawLine(barx, bary+h, barx + barWidth, bary+h, Black);
		HandlePressed(&series_total, &series_pressed);
	} else {
		for(;h>0; --h)
			LCD_SetPoint(barx + bar%barWidth, bary+h, !isPressed?Green:Red);
		++bar;
	}
}

#include "stm32f10x_flash.h"
void RCC_Configuration(void) { //str 52 w stm micro controlers
	RCC_DeInit();

	RCC_HSEConfig(RCC_HSE_ON);

	ErrorStatus HSEStartUpStatus;
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS) {
		// HCLK = SYSCLK
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		// PCLK2 = HCLK
		RCC_PCLK2Config(RCC_HCLK_Div1);
		// PCLK1 = HCLK/2
		RCC_PCLK1Config(RCC_HCLK_Div2);
		// Flash 2 wait state
		FLASH_SetLatency(FLASH_Latency_2);
		// Enable Prefetch Buffer
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		// PLLCLK = 8MHz * 9 = 72 MHz
		RCC_PLLConfig(0x00010000, RCC_PLLMul_9);
		//Enable PLL
		RCC_PLLCmd(ENABLE);
		//Wait till PLL is ready
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
		}
		// Select PLL as system clock source
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		// Wait till PLL is used as system clock source
		while (RCC_GetSYSCLKSource() != 0x08) {
		}
	}
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM1, ENABLE);
	// Enable GPIOB, GPIOC and AFIO clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
			| RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO | RCC_APB2Periph_TIM1, ENABLE);
}

void GPIO_init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = USER_KEYB;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //floating becuse they re connected to power through resitatence
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USER_KEYB_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED1_PORT, &GPIO_InitStructure); //thats possible cos LED2_PORT=LED1_PORT
}

void NVIC_conf() {
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	GPIO_EXTILineConfig(USER_KEYB_PORT_SOURCE, USER_KEYB_PIN_SOURCE);

	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = USER_KEYB_LINE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);


	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn; //strona 113
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

void LCD_DrawSquare(const int X, const int Y, const int w, const int h,
		uint16_t color/*=White*/) {
	int x, y;
	for (x = X; x < X + w; ++x)
		for (y = Y; y < Y + h; ++y) {
			LCD_SetPoint(x, y, color);
		}
}

void delay() {
	int i=0;
	for(i=0; i<0xfffff; ++i);
}

void GPIO_Blink(void) {

	//int swtch = (BitAction) ((1 - GPIO_ReadOutputDataBit(LED1_PORT,	LED1_PIN)));
	//		GPIO_WriteBit(LED1_PORT, LED1_PIN, swtch);
	LED2_PORT->ODR ^= LED2_PIN;
	delay();
}

int on = 0;
void EXTI2_IRQHandler(void) { //zgodnie ze strona 91. 'mikrokontrolery w praktyce'
	if (EXTI_GetFlagStatus(USER_KEYB_LINE) != RESET) {
		//on = !on;
		//on = (LED2_PORT->ODR & LED2_PIN);
		int swtch = (BitAction) ((1 - GPIO_ReadOutputDataBit(LED1_PORT,	LED1_PIN)));
		GPIO_WriteBit(LED1_PORT, LED1_PIN, !swtch);
		//swtch = on;
		//delay_ms(100);
		EXTI_ClearITPendingBit(USER_KEYB_LINE);
	}
}
/*Timer Constant*/
const uint16_t CC1 = 32768;

void TIM1_CC_IRQHandler() {
	static int counter=0;
	if(TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET) {
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
		if(counter==80) GPIO_Blink();
		else if(counter==81) {
			counter=0;
			GPIO_Blink();
		}
		counter++;
		//proceedBar();
		//TIM_SetCompare1(TIM1, TIM_GetCapture1(TIM1)+CC1);
	}
}

void TIM_Conf(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 1440/100; //72Mhz/1440*...=50kHz*...
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CC1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);

	TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);

	TIM_Cmd(TIM1, ENABLE);
}

char *itoaa(int value, char *buf, int base) {
	int i=0;
	buf[i]='\0';
	int t=1;
	do t*=base; while(t<=value);
	t/=base;
	do {
		buf[i]='0'+value/t;
		value-=t;
		buf[++i]='\0';
	} while (value!=0);
	return buf;
}

char *itoa(int value, char *buf, int base) {
	int i=0;
	int len=3;
	int b=base*base; //base^len
	//if(value!=0)
		for(i=0; i<len; ++i) {
			buf[i]='0'+((value/b)%10);
			//value-=(value/b)*b;
			b/=base;
			if(0)if(buf[i]=='0')
				buf[i]=' ';
		}
	if(value==0) buf[len-1]='0';
	buf[len]=0;
	return buf;
}

int inSerie=0;
int isThisSeriePressed=0;
void putSeriesNumber(int newSeries, int whetherCalledFromMailLoop) {
	static int needsUpdate=1;
	static char buf[35];
	static int series=0;
	if(whetherCalledFromMailLoop!=0) {
		//sprintf(buf, "%3d repeats", series);
		itoa(series, buf, 10);
		strcat(buf, " repeats of ");
		strcat(buf, isThisSeriePressed?"pressed ":"released");
		GUI_Text(10, 15, buf, Red, Black);
		needsUpdate=0;
	} else {
		needsUpdate=(series!=newSeries);
		series=newSeries;
	}
}


void Log(const char *text, int clr, int whetherCalledFromMainLoop) {
	static const char *t=0;
	static int needsUpdate=0;
	static int offset=0;
	static int color=Green;
	if(whetherCalledFromMainLoop) {
		if(needsUpdate) {
			int maxheight=6*15;
			GUI_Text(0, 200+offset, t, color, Black);
			offset+=15;
			GUI_Text(0, 200+offset%maxheight, "#                   ", Blue, Black);
			if(offset>=maxheight)
				offset=0;
			needsUpdate=0;
		}
	} else {
		needsUpdate=1;//(strcmp(text, t)!=0);
		color=clr;
		t=text;
	}
}

#define morseBufferMax 25
char morseBuffer[25];

#define maxConsoleLineInput 25
void HandlePressed(int *total, int *pressed) {
	static char consoleLine[25];
	float percent=1.0* *pressed/(*total);
	int countAsPressed=(percent>0.50f);
	int hasStateChanged=(countAsPressed!=isThisSeriePressed);
	if(hasStateChanged) {
		isThisSeriePressed=!isThisSeriePressed;
		const char *message=(isThisSeriePressed?" counted presses ":" counted released");
		itoa(inSerie, consoleLine, 10);
		strncat(consoleLine, message, maxConsoleLineInput-strlen(consoleLine));
		Log(consoleLine, isThisSeriePressed?Red:Green, 0);
		inSerie=1;
	} else {
		inSerie++;
	}
	putSeriesNumber(inSerie, 0); //only updating static field in function
	*total=0;
	*pressed=0;
}

#define outputMaxSize 256
char output[outputMaxSize];

void handleDecodedString() {
	static int lastLen=0; //tricky empting string
	int needsUpdate=lastLen!=strlen(output);
	if(!needsUpdate)return;
	GUI_Text(0, 305-20, output, White, Black);
	lastLen=strlen(output);
}
void addCharToOutput(char c) {
	char buf[2];
	buf[0]=c;
	buf[1]=0;
	if(strlen(output)<outputMaxSize)
		strcat(output, buf);
}

int main() {
	output[0]=morseBuffer[0]=0;
	SystemInit();
	delay_init();
	LCD_Initializtion();
	LCD_BackLight_Init();
	LCD_Clear(Black);

	RCC_Configuration();
	GPIO_init();
	NVIC_conf();
	TIM_Conf();

	GUI_Text(0, 0, "PTM morse decoder", White, Black);
	GUI_Text(45, 305, "Artur Zochniak, PWR 2012", White, Black);
	const int bufsize=7;
	char buf[bufsize];
	int i;
	for(i=0; i<bufsize-1; ++i)
		buf[i]='a'+i%26;
	buf[strlen(buf)-1]=*"\0";

	char temp[10];
#define TIMES(n) int _i; for(_i=0; _i<(n); ++_i)
	TIMES(4) Log(" ", Blue, 0); //just setting the text, not displaying it.
	while(1) {
		int isOn = GPIOB->IDR & USER_KEYB;
		proceedBar(isOn);
		series_total++;
		if(isOn) series_pressed++;

		char buf[10];
		itoa(series_total, buf, 10);
		GUI_Text(200,30,buf, White, Black);
		itoa(series_pressed, buf, 10);
		GUI_Text(200,30+13,buf, Red, Black);

		handleDecodedString();
		Log(0, 0, 1);

		putSeriesNumber(0xffffffff, 1); //means only putting value on the screen
		delay_ms(25);
	}
	return 0;
}
/*
int maina(void) {
	LCD_Initializtion();
	LCD_BackLight_Init();
	LCD_Clear(Yellow);
	RCC_Configuration();
	GPIO_init();
	NVIC_conf();
	TIM_Conf();
	SystemInit();
	delay_init();

	delay_ms(500);
	while(1);
	return 0;
	//LCD_DrawLine(20, 46, 120, 46, Black);

	int inserie = 0;
	int waslast = 0;
	int counter=0;
	while (1) {
		char a = 'a';
		int on = GPIOB->IDR & USER_KEYB;
		LCD_DrawSquare(100, 100, 10, 10, Black);
		if (!on)
			LCD_DrawSquare(100 + 1, 100 + 1, 10 - 2, 10 - 2, White);
		LED1_PORT->ODR = !on ? (LED1_PORT->ODR & (~LED1_PIN)) : (LED1_PORT->ODR
				| (LED1_PIN));
		//GUI_Text(45, 305, "Artur Zochniak, PWR 2012", (on) ? White : Black, (!on) ? White : Black);
		delay_ms(75);
		counter++;

		if(counter++%5!=0) continue;
		if (on) {
			inserie += waslast ? 1 : 0;
			putChar(a + (inserie / 2) % ('z' - 'a' + 1), 0);
			waslast = 1;
		} else if (waslast) {
			putChar(a + (inserie / 2) % ('z' - 'a' + 1), 1);
			inserie = 0;
			waslast = 0;
		}


	}
}
*/

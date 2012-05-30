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

#define TIMES(n) int _i; for(_i=0; _i<(n); ++_i)

//returns if there was new turn
int proceedBar(int isPressed) {
	static int barx = 144;
	static int bary = 4;
	static const int barWidth = 90;
	static int bar=0;
	static int which=0;
	static int first=1;
	int h=5;
	int i;
	if (bar>barWidth/2 || first) {
		bar = 0;
		for(i=h;i>=0; --i)
			LCD_DrawLine(barx, bary+i, barx + barWidth, bary+i, Green);
		if(first) {
			first=0;
			return 0;
		} else {
			return 1;
		}
	} else {
		for(i=h;i>=0; --i) LCD_SetPoint(barx + bar, bary+i, Black);
		for(i=h;i>=0; --i) LCD_SetPoint(barx + barWidth-bar, bary+i, Black);
		++bar;
		return 0;
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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //for keyb
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //for keya

	GPIO_InitStructure.GPIO_Pin = USER_KEYB;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //floating becuse they re connected to power through resitatence
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USER_KEYB_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = USER_KEYA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //floating becuse they re connected to power through resitatence
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USER_KEYA_PORT, &GPIO_InitStructure);
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
	for (y = Y; y < Y + h; ++y) {
		//for (y = Y; y < Y + h; ++y) {
			LCD_DrawLine(X, Y, X, Y+y, color);
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
	static int counter=1;
	if(TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET) {
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
		if(counter%70==0)
			GPIO_Blink();
		else if(counter%72==0) {
			GPIO_Blink();
		}
		if (counter%80==0 || counter%88==0) {
			LED1_PORT->ODR^=LED1_PIN;
		}
		if(counter==88) counter=1;
		counter++;
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

const char *codes[29]=
{"1_111___\0",
"111_1_1_1___\0",
"111_1_111_1___\0",
"111_1_1___\0",
"1___\0",
"1_1_111_1___\0",
"111_111_1___\0",
"1_1_1_1___\0",
"1_1___\0",
"1_111_111_111___\0",
"111_1_111___\0",
"1_111_1_1___\0",
"111_111___\0",
"111_1___\0",
"111_111_111___\0",
"1_111_111_1___\0",
"111_111_1_111___\0",
"1_111_1___\0",
"1_1_1___\0",
"111___\0",
"1_1_111___\0",
"1_1_1_111___\0",
"1_111_111___\0",
"111_1_1_111___\0",
"111_1_111_111___\0",
"111_111_1_1___\0",
"111___\0",
"______\0",
" \0"};
char letters[27]={
'A',
'B',
'C',
'D',
'E',
'F',
'G',
'H',
'I',
'J',
'K',
'L',
'M',
'N',
'O',
'P',
'Q',
'R',
'S',
'T',
'U',
'V',
'W',
'X',
'Y',
'Z',
' ',
' '};

#define outputMaxSize 256
char output[outputMaxSize];

void addCharToOutput(char c) {
	char buf[2];
	buf[0]=c;
	buf[1]=0;
	if(strlen(output)<outputMaxSize)
		strcat(output, buf);
}

char morseBuf[256];
char buf[200];

int printTable(char *morseBuf) {
static int cnt=0;
int i=0;
    #ifdef STM32F103VC
    //if(cnt%(3*26)==0)
    	//LCD_DrawSquare(4,20,240, 13*15,Black);
    	//delay_ms(24);
    #endif
int ok=-2; //nie ma zadnej nadzei na znalezienie rozwiazania
for(i=0; i<26; ++i)
{
	int x=(i/14)*120;
	int y=(i%14)*15+20;
	buf[0]=letters[i];
	buf[1]=0;
	int res=strlen(morseBuf)==0 || strncmp(morseBuf, codes[i], strlen(morseBuf))==0;
	int ncmp=(strcmp(morseBuf, codes[i])==0);
	if(res && ncmp) ok=i; //i-ty element jest rozwiazaniem
	if(res && ok<0) ok=-1; //jest nadal szansa
	int werdykt=(res!=0) || (ncmp!=0);
    int j=3;
	#ifdef STM32F103VC
		//if(cnt%2==0)
			GUI_Text(x+4, y, werdykt?buf:"  ", werdykt?White:Black, Black);
	#endif
	strncpy(buf, codes[i]+((strlen(morseBuf)==0)?0:(strlen(morseBuf))), j);
/*	buf[j++]='w';
	buf[j++]='0'+werdykt;
	buf[j++]='n';
	buf[j++]='0'+ncmp;
	buf[j++]='r';
	buf[j++]='0'+res;*/
	buf[j++]=0;
    #ifdef STM32F103VC
    //if(cnt%2==1)
		//if(strlen(buf)==0)
			TIMES(4) strcatchr(buf, ' ');
    	GUI_Text(x+23, y, buf, werdykt?White:Black, Black);
    #else
	printf("werdykt=%d>letter= >%s< normalcmp=%d cmp=%d morsebuflen=%d codes[i]len=%d<",werdykt,buf,ncmp, res,strlen(morseBuf),strlen(codes[i]));
	if(werdykt){
		printf("%s",buf);
        }
	printf("\n");
	#endif
//if(!(i<26)) i++; else i=0;
}
return ok;
}

int maina() {
	//RCC_Configuration();
	GPIO_init();
	while(1) {

		/*if((GPIOB->IDR & USER_KEYB)) {
			GPIO_Blink();
			//strcat(morseBuf, '1');
			delay_ms(1000);
		}
		if((GPIOC->IDR & GPIO_Pin_13)) {
			GPIO_Blink();
			//strcat(morseBuf, '_');
			delay_ms(1000);
		}*/
		//LED2_PORT->ODR ^= LED2_PIN;
		//LED1_PORT->ODR ^= LED1_PIN;
		GPIO_Blink();
		delay();
		//GUI_Text(0, 290, morseBuf, Yellow, Black);
		delay_ms(100);
	}
}

void strcatchr(char* s, char c)
{
    int len = strlen(s);
    s[len] = c;
    s[len + 1] = '\0';
}

void demo() {
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
    strcpy(morseBuf, " ");
    printTable(morseBuf+1);
#define strcat strcatchr
    strcat(morseBuf, '1');delay_ms(1000);printTable(morseBuf+1);GUI_Text(0, 290, morseBuf, Yellow, Black);
    strcat(morseBuf, '_');delay_ms(1000);printTable(morseBuf+1);GUI_Text(0, 290, morseBuf, Yellow, Black);
    strcat(morseBuf, '1');delay_ms(1000);printTable(morseBuf+1);GUI_Text(0, 290, morseBuf, Yellow, Black);
    strcat(morseBuf, '1');delay_ms(1000);printTable(morseBuf+1);GUI_Text(0, 290, morseBuf, Yellow, Black);
    strcat(morseBuf, '1');delay_ms(1000);printTable(morseBuf+1);GUI_Text(0, 290, morseBuf, Yellow, Black);
    strcat(morseBuf, '_');delay_ms(1000);printTable(morseBuf+1);GUI_Text(0, 290, morseBuf, Yellow, Black);
    strcat(morseBuf, '_');delay_ms(1000);printTable(morseBuf+1);GUI_Text(0, 290, morseBuf, Yellow, Black);
    strcat(morseBuf, '_');delay_ms(1000);printTable(morseBuf+1);GUI_Text(0, 290, morseBuf, Yellow, Black);
#undef strcat
    while(1) delay_ms(1000);
}

int main() {
	//demo();
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
    strcpy(morseBuf, " ");
    strcpy(output, " ");
	//strcatchr(output, 'b');
	GUI_Text(0, 270, output, Green, Black);
	//while(1);
	while(1) {
		int i,r;
		while((r=printTable(morseBuf+1))!=-2) {
			if(r>=0) break;
				if(proceedBar(0)) {
					int isOn;
					isOn = (GPIOB->IDR & USER_KEYB);
					if(isOn) strcatchr(morseBuf, '1');
						else strcatchr(morseBuf, '_');
					//if((USER_KEYA_PORT->IDR & USER_KEYA))
					///	strcatchr(morseBuf, '_');
					GUI_Text(0, 290, morseBuf, Yellow, Black);
					delay_ms(30);
				}
			}
		if(r==-2) {
			strcpy(morseBuf, "                        ");
			GUI_Text(0, 290, morseBuf, Yellow, Black);
			strcpy(morseBuf, " ");
		}

		if (r>=0) {
			//strcatchr(output, letters[r]);
			//strcatchr(output, 'r');
			//strcatchr(output, 'r');
			//strcatchr(output, '0'+(r/10));
			//strcatchr(output, '0'+(r%10));
			//GUI_Text(0, 270, output, Green, Black);
			strcatchr(output, letters[r]);
			GUI_Text(0, 270, output, Green, Black);
			strcpy(morseBuf, "                        ");
			GUI_Text(0, 290, morseBuf, Yellow, Black);
			strcpy(morseBuf, " ");
			//delay(1000);
		}
	}
	return 0;
}

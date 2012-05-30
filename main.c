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


#define TIMES(n) int _i; for(_i=0; _i<(n); ++_i)

//returns if there was new turn
int proceedBar(int isPressed) {
	static int barx = 144;
	static int bary = 4;
	static const int barWidth = 90;
	static int bar=0;
	static int which=0;
	int h=5;
	int i;
	if (bar>barWidth/2) {
		bar = 0;
		for(i=h;i>=0; --i)
			LCD_DrawLine(barx, bary+i, barx + barWidth, bary+i, Green);
		return 1;
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

void Log(const char *text, int clr, int whetherCalledFromMainLoop) {
	#define bufLen (31)
	static char buf[bufLen];
	static const char *t=0;
	static int needsUpdate=0;
	static int offset=0;
	static int color=Green;
	if(text) {
			needsUpdate=1;//(strcmp(text, t)!=0);
			color=clr;
			t=text;
	}
	if(whetherCalledFromMainLoop) {
		if(needsUpdate) {
			int maxheight=6*15;
			TIMES(bufLen) buf[_i]=' ';
			buf[bufLen-1]=0;
			strncpy(buf, t, strlen(t));
			GUI_Text(0, 200+offset, buf, color, Black);
			offset+=15;
			GUI_Text(0, 200+offset%maxheight, "#                   ", Blue, Black);
			if(offset>=maxheight)
				offset=0;
			needsUpdate=0;
		}
	}
}

char interpretMorseFromBuffer() {
	return 'A';
}


int series_total=0;
int series_pressed=0;
void HandlePressed(int *total, int *pressed);

int inSerie=0;
int isThisSeriePressed=0;

#define morseBufferMax 31
char morseBuffer[morseBufferMax];

void writeMorseBuferOnScreen() {
	GUI_Text(0, 290, morseBuffer, Yellow, Black);
}
void resetMorseBuffer() {
	static char buf[morseBufferMax];
	TIMES(morseBufferMax) buf[_i]=' ';
	strncpy(morseBuffer, buf, morseBufferMax);
	writeMorseBuferOnScreen();
	morseBuffer[0]=' ';
	morseBuffer[1]=0;
	strncpy(morseBuffer, buf, morseBufferMax);
}

void addCharToMorseBuffer(char c) {
	char buf[2];
	buf[0]=c;
	buf[1]=0;
	if(strlen(morseBuffer)<morseBufferMax)
		strcat(morseBuffer, buf);
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

const char **findNext(const char **codes, const char *beg) {
    int res;
    --codes;
    do {
        codes++;
        res=strncmp(*codes,beg,strlen(beg));
    } while(res!=0 && **codes!=0);
    return codes;
}

void interpretMorseBufer();


void HandlePressed(int *total, int *pressed) {

	float percent=1.0* *pressed/(*total);
	int countAsPressed=(percent>0.50f);
	int hasStateChanged=(countAsPressed!=isThisSeriePressed);
	int wasButtonReleased=!isThisSeriePressed;
	int wasButtonPressed=isThisSeriePressed;
	int canAddTick=(morseBuffer[strlen(morseBuffer)-1]=='4' || morseBuffer[strlen(morseBuffer)-1]==' ');

	if(hasStateChanged) {
		if (wasButtonReleased) {
			if(inSerie==1 && !canAddTick) {
				addCharToMorseBuffer('_');
			} else if (inSerie==3 && !canAddTick) {
				Log("interpret", Red, 1);
				interpretMorseBufer();
			} else {
				Log("syntax error 1", Red, 1);
				//resetMorseBuffer();
			}
		} else if (wasButtonPressed) {
			if(canAddTick) {
				Log("syntax error 2", Red, 1);
				//resetMorseBuffer();
			}
			if(inSerie==1 || inSerie==1)
				addCharToMorseBuffer('0'+inSerie);
		}
		isThisSeriePressed=!isThisSeriePressed;
		inSerie=0;
	}
	if(!hasStateChanged) {
		if(wasButtonPressed && inSerie>3) {
			//resetMorseBuffer();
			Log("syntax error 3", Red, 1);
		}
		inSerie++;
	}

	*total=0;
	*pressed=0;

}

#define outputMaxSize 256
char output[outputMaxSize];

void handleDecodedString() {
	static int lastLen=0; //tricky empting string
	int needsUpdate=lastLen!=strlen(output);
	if(!needsUpdate)return;
	GUI_Text(0, 100, output, White, Black);
	lastLen=strlen(output);
}
void addCharToOutput(char c) {
	char buf[2];
	buf[0]=c;
	buf[1]=0;
	if(strlen(output)<outputMaxSize)
		strcat(output, buf);
}
//look for morse code in table and, if morse code appears to be
//valid - add this letter to output and erase morse buffer
void handleMorseBuffer() {
	char c=interpretMorseFromBuffer();
	if(c!=0) {
		addCharToOutput(c);
		Log("new letter added", Cyan, 1);
	} else {
		Log("syntax error", Cyan, 1);
	}
	resetMorseBuffer();
}

char cntr[]={'-', '=', '#'};
void shotren(int x, int y, const char *t, u16 c, u16 bk) {
	static char buf[26];
	int p=0;
	buf[0]=0;
	const char *it=t;
	while(*it!=0) {
		char b=*it;
		int cnt=1;
		if(b=='1') {
			while(*(++it)==b && *it!=0 && cnt<3)
				cnt++;
			//if(b=='1')
				buf[p++]='0'+cnt;
		}
		else {
			buf[p++]=b;
            it++;
		}
		buf[p]=0;
		//if(*t!=0) t--;
	}
	GUI_Text(x, y, buf, c, bk);
	//printf("%s\n", buf);
}

void interpretMorseBufer() {
	static char poss[27];
	TIMES(27) poss[_i]=' '; poss[27-1]=0;
	//poss[0]='c';	poss[1]='z';	poss[2]='e';poss[3]=0;
	GUI_Text(15, 100, poss, White, Black);
	//return;
	int p=0;
	poss[p++]=' ';
	poss[p]=0;
	const char **it=codes;
	char buf[2];
	buf[1]=0;
	int cnt=1;
	LCD_DrawSquare(4,20,(28/14)*120, 13*15,Black);
    while (**(it=findNext(it, morseBuffer+1))!=0) {
    	int i=it-codes;
    	//if(i>=26) break;
        //poss[p++]=letters[i];
        //poss[p]=0;
    	buf[0]=letters[i];
    	buf[1]=0;
    	int x=((i+1)/14)*120;
    	int y=(i%14)*15+20;
    	GUI_Text(x+4, y, buf, White, Black);
    	//delay_ms(1);
    	strncpy(buf, *it, 3);
    	GUI_Text(x+23, y, buf, White, Black);
    	//delay_ms(1);

    	//shotren(x+23, y, *it, White, Black);

    	++cnt;
        ++it;
    }
	//GUI_Text(15, 100, poss, White, Black);
}

void printTable() {
int i; i=0;
char buf[10];
LCD_DrawSquare(4,20,240, 13*15,Black);
//for(i=0; i<4; ++i)
{
	int x=(i/14)*120;
	int y=(i%14)*15+20;
	buf[0]=letters[i];
	buf[1]=0;
	int res=strncmp(morseBuffer+1, codes[i], strlen(morseBuffer+1))==0;
	GUI_Text(x+4, y, buf, res?White:Black, res?Black:White);
	strncpy(morseBuffer, codes[i], 4);
	GUI_Text(x+23, y, buf, res?White:Black, res?Black:White);
}
}

int main() {
	output[0]=0;
	morseBuffer[0]=' ';
	morseBuffer[1]=' ';
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
	strncpy(morseBuffer, " 11", 4);
	interpretMorseBufer();
	while(1)
	//	LCD_DrawSquare(4,20,240, 13*15,Black);
		;
	TIMES(4) Log(" ", Blue, 0); //just setting the text, not displaying it.
	while(1) {
		int isOn = GPIOB->IDR & USER_KEYB;
		series_total++;
		if(isOn) series_pressed++;
		if(proceedBar(isOn)) {
			//HandlePressed(&series_total, &series_pressed);
			if(1.0*series_pressed/series_total>0.5) {
				addCharToMorseBuffer('1');
			} else {
				addCharToMorseBuffer('_');
			}
			const char **it=codes;
			//if(**(it=findNext(it, morseBuffer+1))!=0) {
			//	resetMorseBuffer();
			//}
			//interpretMorseBufer();
			printTable();
			series_total=0;
			series_pressed=0;
		}
		goto skip;

		char buf[20];
		itoa(series_total, buf, 10);
		GUI_Text(200,30,buf, White, Black);
		itoa(series_pressed, buf, 10);
		GUI_Text(200,30+13,buf, Red, Black);

		itoa(inSerie, buf, 10);
		strcat(buf, " repeats of ");
		strcat(buf, isThisSeriePressed?"pressed ":"released");
		GUI_Text(10, 15, buf, isThisSeriePressed?Red:Green, Black);


		//handleDecodedString();
		skip:
		writeMorseBuferOnScreen();
		delay_ms(25);
	}
	return 0;
}

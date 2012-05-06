/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.c
** Descriptions:		STM32 FSMC TFT操作函数库
**						
**------------------------------------------------------------------------------------------------------
** Created by:			poweravr
** Created date:		2010-11-7
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "GLCD.h" 
#include "HzLib.h"
#include "AsciiLib.h"
#include <math.h>

/* Private variables ---------------------------------------------------------*/
static uint16_t DeviceCode;
static uint16_t TimerPeriod = 0;
static uint16_t Channel2Pulse = 1000;

/* Private define ------------------------------------------------------------*/
/* 使用总线方式时定义地址 */
/* 挂在不同的BANK,使用不同地址线时请自行换算地址 */
#define LCD_REG              (*((volatile unsigned short *) 0x60000000)) /* RS = 0 */
#define LCD_RAM              (*((volatile unsigned short *) 0x60020000)) /* RS = 1 */


/*******************************************************************************
* Function Name  : LCD_CtrlLinesConfig
* Description    : Configures LCD Control lines (FSMC Pins) in alternate function
                   Push-Pull mode.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_CtrlLinesConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable FSMC, GPIOD, GPIOE and AFIO clocks */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
                        
  /* PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9),
     PE.13(D10), PE.14(D11), PE.15(D12) */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                 GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                 GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* PD.00(D2), PD.01(D3), PD.04(RD), PD.5(WR), PD.7(CS), PD.8(D13), PD.9(D14),
     PD.10(D15), PD.11(RS) PD.14(D0) PD.15(D1) */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | 
                                 GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | 
                                 GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : LCD_FSMCConfig
* Description    : Configures the Parallel interface (FSMC) for LCD(Parallel mode)
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_FSMCConfig(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructure;
  /* FSMC读速度设置 */
  FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 30;  /* 地址建立时间  */
  FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;	   
  FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 30;	   /* 数据建立时间  */
  FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;	/* FSMC 访问模式 */

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 
  /* FSMC写速度设置 */
  FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 2;   /* 地址建立时间  */
  FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;	   
  FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 2;	   /* 数据建立时间  */
  FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;	/* FSMC 访问模式 */
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;	  

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

  /* Enable FSMC Bank1_SRAM Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}

/*******************************************************************************
* Function Name  : LCD_Configuration
* Description    : Configure the LCD Control pins and FSMC Parallel interface
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void)
{
 /* Configure the LCD Control pins --------------------------------------------*/
  LCD_CtrlLinesConfig();

/* Configure the FSMC Parallel interface -------------------------------------*/
  LCD_FSMCConfig();
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __inline void LCD_WriteReg(uint8_t LCD_Reg,uint16_t LCD_RegValue)
{
  /* Write 16-bit Index, then Write Reg */
  LCD_REG = LCD_Reg;
  /* Write 16-bit Reg */
  LCD_RAM = LCD_RegValue;
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention		 : None
*******************************************************************************/
static __inline uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
  /* Write 16-bit Index (then Read Reg) */
  LCD_REG = LCD_Reg;
  /* Read 16-bit Reg */
  return (LCD_RAM);
}

/*******************************************************************************
* Function Name  : LCD_WriteRAM_Prepare
* Description    : Prepare to write to the LCD RAM.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __inline void LCD_WriteRAM_Prepare(void)
{
  LCD_REG = R34;
}

/*******************************************************************************
* Function Name  : LCD_WriteRAM
* Description    : Writes to the LCD RAM.
* Input          : - RGB_Code: the pixel color in RGB mode (5-6-5).
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __inline void LCD_WriteRAM(uint16_t RGB_Code)					 
{
  /* Write 16-bit GRAM Reg */
  LCD_RAM = RGB_Code;
}

/*******************************************************************************
* Function Name  : LCD_ReadRAM
* Description    : Reads the LCD RAM.
* Input          : None
* Output         : None
* Return         : LCD RAM Value.
* Attention		 : None
*******************************************************************************/
static __inline uint16_t LCD_ReadRAM(void)
{
  volatile uint16_t dummy; 
  /* Write 16-bit Index (then Read Reg) */
  LCD_REG = R34; /* Select GRAM Reg */
  /* Read 16-bit Reg */
  dummy = LCD_RAM; 
  
  return LCD_RAM;
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos)
{
  if(DeviceCode==0x8989)
  {
    LCD_WriteReg(0x004e,Xpos); /* Row */
    LCD_WriteReg(0x004f,Ypos); /* Line */ 
  }
  else if(DeviceCode==0x9919)
  {
    LCD_WriteReg(0x004e,Xpos); /* Row */
    LCD_WriteReg(0x004f,Ypos); /* Line */	
  }
  else
  {
    LCD_WriteReg(0x0020,Xpos); /* Row */
    LCD_WriteReg(0x0021,Ypos); /* Line */
  }
}

/*******************************************************************************
* Function Name  : LCD_Delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Delay(uint16_t nCount)
{
 uint16_t TimingDelay; 
 while(nCount--)
   {
    for(TimingDelay=0;TimingDelay<10000;TimingDelay++);
   }
}

/*******************************************************************************
* Function Name  : LCD_Initializtion
* Description    : Initialize TFT Controller.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Initializtion()
{
  LCD_Configuration();
  LCD_Delay(5);  /* delay 50 ms */		
  DeviceCode = LCD_ReadReg(0x0000);		/* 读取屏ID	*/
  if(DeviceCode==0x9325 || DeviceCode==0x9328)	/* 不同屏驱动IC 初始化不同 */
  {
    LCD_WriteReg(0x00e7,0x0010);      
    LCD_WriteReg(0x0000,0x0001);  	/* start internal osc */
    LCD_WriteReg(0x0001,0x0100);     
    LCD_WriteReg(0x0002,0x0700); 	/* power on sequence */
	LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) ); 	/* importance */
    LCD_WriteReg(0x0004,0x0000);                                   
    LCD_WriteReg(0x0008,0x0207);	           
    LCD_WriteReg(0x0009,0x0000);         
    LCD_WriteReg(0x000a,0x0000); 	/* display setting */        
    LCD_WriteReg(0x000c,0x0001);	/* display setting */        
    LCD_WriteReg(0x000d,0x0000); 			        
    LCD_WriteReg(0x000f,0x0000);
    /* Power On sequence */
    LCD_WriteReg(0x0010,0x0000);   
    LCD_WriteReg(0x0011,0x0007);
    LCD_WriteReg(0x0012,0x0000);                                                                 
    LCD_WriteReg(0x0013,0x0000);                 
    LCD_Delay(5);  /* delay 50 ms */		
    LCD_WriteReg(0x0010,0x1590);   
    LCD_WriteReg(0x0011,0x0227);
    LCD_Delay(5);  /* delay 50 ms */		
    LCD_WriteReg(0x0012,0x009c);                  
    LCD_Delay(5);  /* delay 50 ms */		
    LCD_WriteReg(0x0013,0x1900);   
    LCD_WriteReg(0x0029,0x0023);
    LCD_WriteReg(0x002b,0x000e);
    LCD_Delay(5);  /* delay 50 ms */		
    LCD_WriteReg(0x0020,0x0000);                                                            
    LCD_WriteReg(0x0021,0x0000);           
    LCD_Delay(5);  /* delay 50 ms */		
    LCD_WriteReg(0x0030,0x0007); 
    LCD_WriteReg(0x0031,0x0707);   
    LCD_WriteReg(0x0032,0x0006);
    LCD_WriteReg(0x0035,0x0704);
    LCD_WriteReg(0x0036,0x1f04); 
    LCD_WriteReg(0x0037,0x0004);
    LCD_WriteReg(0x0038,0x0000);        
    LCD_WriteReg(0x0039,0x0706);     
    LCD_WriteReg(0x003c,0x0701);
    LCD_WriteReg(0x003d,0x000f);
    LCD_Delay(5);  /* delay 50 ms */		
    LCD_WriteReg(0x0050,0x0000);        
    LCD_WriteReg(0x0051,0x00ef);   
    LCD_WriteReg(0x0052,0x0000);     
    LCD_WriteReg(0x0053,0x013f);
    LCD_WriteReg(0x0060,0xa700);        
    LCD_WriteReg(0x0061,0x0001); 
    LCD_WriteReg(0x006a,0x0000);
    LCD_WriteReg(0x0080,0x0000);
    LCD_WriteReg(0x0081,0x0000);
    LCD_WriteReg(0x0082,0x0000);
    LCD_WriteReg(0x0083,0x0000);
    LCD_WriteReg(0x0084,0x0000);
    LCD_WriteReg(0x0085,0x0000);
      
    LCD_WriteReg(0x0090,0x0010);     
    LCD_WriteReg(0x0092,0x0000);  
    LCD_WriteReg(0x0093,0x0003);
    LCD_WriteReg(0x0095,0x0110);
    LCD_WriteReg(0x0097,0x0000);        
    LCD_WriteReg(0x0098,0x0000);  
    /* display on sequence */    
    LCD_WriteReg(0x0007,0x0133);
    
    LCD_WriteReg(0x0020,0x0000);  /* 行首址0 */                                                          
    LCD_WriteReg(0x0021,0x0000);  /* 列首址0 */     
  }
  else if(DeviceCode==0x9320 || DeviceCode==0x9300)
  {
    LCD_WriteReg(0x00,0x0000);
	LCD_WriteReg(0x01,0x0100);	/* Driver Output Contral */
	LCD_WriteReg(0x02,0x0700);	/* LCD Driver Waveform Contral */
	LCD_WriteReg(0x03,0x1018);	/* Entry Mode Set */
	
	LCD_WriteReg(0x04,0x0000);	/* Scalling Contral */
    LCD_WriteReg(0x08,0x0202);	/* Display Contral */
	LCD_WriteReg(0x09,0x0000);	/* Display Contral 3.(0x0000) */
	LCD_WriteReg(0x0a,0x0000);	/* Frame Cycle Contal.(0x0000) */
    LCD_WriteReg(0x0c,(1<<0));	/* Extern Display Interface Contral */
	LCD_WriteReg(0x0d,0x0000);	/* Frame Maker Position */
	LCD_WriteReg(0x0f,0x0000);	/* Extern Display Interface Contral 2. */
	
    LCD_Delay(10);  /* delay 100 ms */		
	LCD_WriteReg(0x07,0x0101);	/* Display Contral */
    LCD_Delay(10);  /* delay 100 ms */		

	LCD_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	/* Power Control 1.(0x16b0)	*/
	LCD_WriteReg(0x11,0x0007);								/* Power Control 2 */
	LCD_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));				/* Power Control 3.(0x0138)	*/
	LCD_WriteReg(0x13,0x0b00);								/* Power Control 4 */
	LCD_WriteReg(0x29,0x0000);								/* Power Control 7 */
	
	LCD_WriteReg(0x2b,(1<<14)|(1<<4));
		
	LCD_WriteReg(0x50,0);       /* Set X Start */
	LCD_WriteReg(0x51,239);	    /* Set X End */
	LCD_WriteReg(0x52,0);	    /* Set Y Start */
	LCD_WriteReg(0x53,319);	    /* Set Y End */
	
	LCD_WriteReg(0x60,0x2700);	/* Driver Output Control */
	LCD_WriteReg(0x61,0x0001);	/* Driver Output Control */
	LCD_WriteReg(0x6a,0x0000);	/* Vertical Srcoll Control */
	
	LCD_WriteReg(0x80,0x0000);	/* Display Position? Partial Display 1 */
	LCD_WriteReg(0x81,0x0000);	/* RAM Address Start? Partial Display 1 */
	LCD_WriteReg(0x82,0x0000);	/* RAM Address End-Partial Display 1 */
	LCD_WriteReg(0x83,0x0000);	/* Displsy Position? Partial Display 2 */
	LCD_WriteReg(0x84,0x0000);	/* RAM Address Start? Partial Display 2 */
	LCD_WriteReg(0x85,0x0000);	/* RAM Address End? Partial Display 2 */
	
    LCD_WriteReg(0x90,(0<<7)|(16<<0));	/* Frame Cycle Contral.(0x0013)	*/
	LCD_WriteReg(0x92,0x0000);	/* Panel Interface Contral 2.(0x0000) */
	LCD_WriteReg(0x93,0x0001);	/* Panel Interface Contral 3. */
    LCD_WriteReg(0x95,0x0110);	/* Frame Cycle Contral.(0x0110)	*/
	LCD_WriteReg(0x97,(0<<8));	
	LCD_WriteReg(0x98,0x0000);	/* Frame Cycle Contral */

    LCD_WriteReg(0x07,0x0173);
  }
  else if(DeviceCode==0x9331)
  {
	LCD_WriteReg(0x00E7, 0x1014);
	LCD_WriteReg(0x0001, 0x0100);   /* set SS and SM bit */
	LCD_WriteReg(0x0002, 0x0200);   /* set 1 line inversion */
	LCD_WriteReg(0x0003, 0x1030);   /* set GRAM write direction and BGR=1 */
	LCD_WriteReg(0x0008, 0x0202);   /* set the back porch and front porch */
    LCD_WriteReg(0x0009, 0x0000);   /* set non-display area refresh cycle ISC[3:0] */
	LCD_WriteReg(0x000A, 0x0000);   /* FMARK function */
	LCD_WriteReg(0x000C, 0x0000);   /* RGB interface setting */
	LCD_WriteReg(0x000D, 0x0000);   /* Frame marker Position */
	LCD_WriteReg(0x000F, 0x0000);   /* RGB interface polarity */
	/* Power On sequence */
	LCD_WriteReg(0x0010, 0x0000);   /* SAP, BT[3:0], AP, DSTB, SLP, STB	*/
	LCD_WriteReg(0x0011, 0x0007);   /* DC1[2:0], DC0[2:0], VC[2:0] */
	LCD_WriteReg(0x0012, 0x0000);   /* VREG1OUT voltage	*/
	LCD_WriteReg(0x0013, 0x0000);   /* VDV[4:0] for VCOM amplitude */
    LCD_Delay(20);                  /* delay 200 ms */		
	LCD_WriteReg(0x0010, 0x1690);   /* SAP, BT[3:0], AP, DSTB, SLP, STB	*/
	LCD_WriteReg(0x0011, 0x0227);   /* DC1[2:0], DC0[2:0], VC[2:0] */
    LCD_Delay(5);                   /* delay 50 ms */		
	LCD_WriteReg(0x0012, 0x000C);   /* Internal reference voltage= Vci	*/
    LCD_Delay(5);                    /* delay 50 ms */		
	LCD_WriteReg(0x0013, 0x0800);   /* Set VDV[4:0] for VCOM amplitude */
	LCD_WriteReg(0x0029, 0x0011);   /* Set VCM[5:0] for VCOMH */
	LCD_WriteReg(0x002B, 0x000B);   /* Set Frame Rate */
    LCD_Delay(5);                   /* delay 50 ms */		
	LCD_WriteReg(0x0020, 0x0000);   /* GRAM horizontal Address */
	LCD_WriteReg(0x0021, 0x0000);   /* GRAM Vertical Address */
	/* Adjust the Gamma Curve */
	LCD_WriteReg(0x0030, 0x0000);
	LCD_WriteReg(0x0031, 0x0106);
	LCD_WriteReg(0x0032, 0x0000);
	LCD_WriteReg(0x0035, 0x0204);
	LCD_WriteReg(0x0036, 0x160A);
	LCD_WriteReg(0x0037, 0x0707);
	LCD_WriteReg(0x0038, 0x0106);
	LCD_WriteReg(0x0039, 0x0707);
	LCD_WriteReg(0x003C, 0x0402);
	LCD_WriteReg(0x003D, 0x0C0F);
	/* Set GRAM area */
	LCD_WriteReg(0x0050, 0x0000);   /* Horizontal GRAM Start Address */
	LCD_WriteReg(0x0051, 0x00EF);   /* Horizontal GRAM End Address */
	LCD_WriteReg(0x0052, 0x0000);   /* Vertical GRAM Start Address */
	LCD_WriteReg(0x0053, 0x013F);   /* Vertical GRAM Start Address */
	LCD_WriteReg(0x0060, 0x2700);   /* Gate Scan Line */
	LCD_WriteReg(0x0061, 0x0001);   /*  NDL,VLE, REV */
	LCD_WriteReg(0x006A, 0x0000);   /* set scrolling line */
	/* Partial Display Control */
	LCD_WriteReg(0x0080, 0x0000);
	LCD_WriteReg(0x0081, 0x0000);
	LCD_WriteReg(0x0082, 0x0000);
	LCD_WriteReg(0x0083, 0x0000);
	LCD_WriteReg(0x0084, 0x0000);
	LCD_WriteReg(0x0085, 0x0000);
	/* Panel Control */
	LCD_WriteReg(0x0090, 0x0010);
	LCD_WriteReg(0x0092, 0x0600);
	LCD_WriteReg(0x0007,0x0021);		
    LCD_Delay(5);                   /* delay 50 ms */		
	LCD_WriteReg(0x0007,0x0061);
    LCD_Delay(5);                   /* delay 50 ms */		
	LCD_WriteReg(0x0007,0x0133);    /* 262K color and display ON */
    LCD_Delay(5);                   /* delay 50 ms */		
  }
  else if(DeviceCode==0x9919)
  {
    /* POWER ON &RESET DISPLAY OFF */
	LCD_WriteReg(0x28,0x0006);
	LCD_WriteReg(0x00,0x0001);		
	LCD_WriteReg(0x10,0x0000);		
	LCD_WriteReg(0x01,0x72ef);
	LCD_WriteReg(0x02,0x0600);
	LCD_WriteReg(0x03,0x6a38);	
	LCD_WriteReg(0x11,0x6874);
	LCD_WriteReg(0x0f,0x0000);    /* RAM WRITE DATA MASK */
	LCD_WriteReg(0x0b,0x5308);    /* RAM WRITE DATA MASK */
	LCD_WriteReg(0x0c,0x0003);
	LCD_WriteReg(0x0d,0x000a);
	LCD_WriteReg(0x0e,0x2e00);  
	LCD_WriteReg(0x1e,0x00be);
	LCD_WriteReg(0x25,0x8000);
	LCD_WriteReg(0x26,0x7800);
	LCD_WriteReg(0x27,0x0078);
	LCD_WriteReg(0x4e,0x0000);
	LCD_WriteReg(0x4f,0x0000);
	LCD_WriteReg(0x12,0x08d9);
	/* Adjust the Gamma Curve */
	LCD_WriteReg(0x30,0x0000);
	LCD_WriteReg(0x31,0x0104);	 
	LCD_WriteReg(0x32,0x0100);	
    LCD_WriteReg(0x33,0x0305);	
    LCD_WriteReg(0x34,0x0505);	 
	LCD_WriteReg(0x35,0x0305);	
    LCD_WriteReg(0x36,0x0707);	
    LCD_WriteReg(0x37,0x0300);	
	LCD_WriteReg(0x3a,0x1200);	
	LCD_WriteReg(0x3b,0x0800);		 
    LCD_WriteReg(0x07,0x0033);
  }
  else if(DeviceCode==0x1505)
  {
    /* second release on 3/5  ,luminance is acceptable,water wave appear during camera preview */
    LCD_WriteReg(0x0007,0x0000);
    LCD_Delay(5);                   /* delay 50 ms */		
    LCD_WriteReg(0x0012,0x011C);    /* why need to set several times?	*/
    LCD_WriteReg(0x00A4,0x0001);    /* NVM */
    LCD_WriteReg(0x0008,0x000F);
    LCD_WriteReg(0x000A,0x0008);
    LCD_WriteReg(0x000D,0x0008);    
    /* GAMMA CONTROL */
    LCD_WriteReg(0x0030,0x0707);
    LCD_WriteReg(0x0031,0x0007); 
    LCD_WriteReg(0x0032,0x0603); 
    LCD_WriteReg(0x0033,0x0700); 
    LCD_WriteReg(0x0034,0x0202); 
    LCD_WriteReg(0x0035,0x0002); 
    LCD_WriteReg(0x0036,0x1F0F);
    LCD_WriteReg(0x0037,0x0707); 
    LCD_WriteReg(0x0038,0x0000); 
    LCD_WriteReg(0x0039,0x0000); 
    LCD_WriteReg(0x003A,0x0707); 
    LCD_WriteReg(0x003B,0x0000); 
    LCD_WriteReg(0x003C,0x0007); 
    LCD_WriteReg(0x003D,0x0000); 
    LCD_Delay(5);                   /* delay 50 ms */		
    LCD_WriteReg(0x0007,0x0001);
    LCD_WriteReg(0x0017,0x0001);    /* Power supply startup enable */
    LCD_Delay(5);                   /* delay 50 ms */		
    /* power control */
    LCD_WriteReg(0x0010,0x17A0); 
    LCD_WriteReg(0x0011,0x0217);    /* reference voltage VC[2:0]   Vciout = 1.00*Vcivl */
    LCD_WriteReg(0x0012,0x011E);    /* Vreg1out = Vcilvl*1.80   is it the same as Vgama1out ?	*/
    LCD_WriteReg(0x0013,0x0F00);    /* VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl */
    LCD_WriteReg(0x002A,0x0000);  
    LCD_WriteReg(0x0029,0x000A);    /* Vcomh = VCM1[4:0]*Vreg1out    gate source voltage?? */
    LCD_WriteReg(0x0012,0x013E);    /* power supply on */
    /* Coordinates Control */
    LCD_WriteReg(0x0050,0x0000);
    LCD_WriteReg(0x0051,0x00EF); 
    LCD_WriteReg(0x0052,0x0000); 
    LCD_WriteReg(0x0053,0x013F); 
    /* Pannel Image Control */
    LCD_WriteReg(0x0060,0x2700); 
    LCD_WriteReg(0x0061,0x0001); 
    LCD_WriteReg(0x006A,0x0000); 
    LCD_WriteReg(0x0080,0x0000); 
    /* Partial Image Control */
    LCD_WriteReg(0x0081,0x0000); 
    LCD_WriteReg(0x0082,0x0000); 
    LCD_WriteReg(0x0083,0x0000); 
    LCD_WriteReg(0x0084,0x0000); 
    LCD_WriteReg(0x0085,0x0000); 
    /* Panel Interface Control */
    LCD_WriteReg(0x0090,0x0013);      /* frenqucy */	
    LCD_WriteReg(0x0092,0x0300); 
    LCD_WriteReg(0x0093,0x0005); 
    LCD_WriteReg(0x0095,0x0000); 
    LCD_WriteReg(0x0097,0x0000); 
    LCD_WriteReg(0x0098,0x0000); 
  
    LCD_WriteReg(0x0001,0x0100); 
    LCD_WriteReg(0x0002,0x0700); 
    LCD_WriteReg(0x0003,0x1030); 
    LCD_WriteReg(0x0004,0x0000); 
    LCD_WriteReg(0x000C,0x0000); 
    LCD_WriteReg(0x000F,0x0000); 
    LCD_WriteReg(0x0020,0x0000); 
    LCD_WriteReg(0x0021,0x0000); 
    LCD_WriteReg(0x0007,0x0021); 
    LCD_Delay(20);                   /* delay 200 ms */		
    LCD_WriteReg(0x0007,0x0061); 
    LCD_Delay(20);                   /* delay 200 ms */		
    LCD_WriteReg(0x0007,0x0173); 
    LCD_Delay(20);                   /* delay 200 ms */		
  }							 
  else if(DeviceCode==0x8989)
  {
    LCD_WriteReg(0x0000,0x0001);    LCD_Delay(5);   /* 打开晶振 */
    LCD_WriteReg(0x0003,0xA8A4);    LCD_Delay(5);   
    LCD_WriteReg(0x000C,0x0000);    LCD_Delay(5);   
    LCD_WriteReg(0x000D,0x080C);    LCD_Delay(5);   
    LCD_WriteReg(0x000E,0x2B00);    LCD_Delay(5);   
    LCD_WriteReg(0x001E,0x00B0);    LCD_Delay(5);   
    LCD_WriteReg(0x0001,0x2B3F);    LCD_Delay(5);   /* 驱动输出控制320*240 0x2B3F */
    LCD_WriteReg(0x0002,0x0600);    LCD_Delay(5);
    LCD_WriteReg(0x0010,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0011,0x6070);    LCD_Delay(5);   /* 定义数据格式 16位色 横屏 0x6070 */
    LCD_WriteReg(0x0005,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0006,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0016,0xEF1C);    LCD_Delay(5);
    LCD_WriteReg(0x0017,0x0003);    LCD_Delay(5);
    LCD_WriteReg(0x0007,0x0133);    LCD_Delay(5);         
    LCD_WriteReg(0x000B,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x000F,0x0000);    LCD_Delay(5);   /* 扫描开始地址 */
    LCD_WriteReg(0x0041,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0042,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0048,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0049,0x013F);    LCD_Delay(5);
    LCD_WriteReg(0x004A,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x004B,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0044,0xEF00);    LCD_Delay(5);
    LCD_WriteReg(0x0045,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0046,0x013F);    LCD_Delay(5);
    LCD_WriteReg(0x0030,0x0707);    LCD_Delay(5);
    LCD_WriteReg(0x0031,0x0204);    LCD_Delay(5);
    LCD_WriteReg(0x0032,0x0204);    LCD_Delay(5);
    LCD_WriteReg(0x0033,0x0502);    LCD_Delay(5);
    LCD_WriteReg(0x0034,0x0507);    LCD_Delay(5);
    LCD_WriteReg(0x0035,0x0204);    LCD_Delay(5);
    LCD_WriteReg(0x0036,0x0204);    LCD_Delay(5);
    LCD_WriteReg(0x0037,0x0502);    LCD_Delay(5);
    LCD_WriteReg(0x003A,0x0302);    LCD_Delay(5);
    LCD_WriteReg(0x003B,0x0302);    LCD_Delay(5);
    LCD_WriteReg(0x0023,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0024,0x0000);    LCD_Delay(5);
    LCD_WriteReg(0x0025,0x8000);    LCD_Delay(5);
    LCD_WriteReg(0x004f,0);        /* 行首址0 */
    LCD_WriteReg(0x004e,0);        /* 列首址0 */
  }
  LCD_Delay(5);  /* delay 50 ms */		
}

/******************************************************************************
* Function Name  : LCD_SetWindows
* Description    : Sets Windows Area.
* Input          : - StartX: Row Start Coordinate 
*                  - StartY: Line Start Coordinate  
*				   - xLong:  
*				   - yLong: 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetWindows(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong)
{
  LCD_SetCursor(xStart,yStart); 
  LCD_WriteReg(0x0050,xStart);         /* 水平GRAM起始位置 */
  LCD_WriteReg(0x0051,xStart+xLong-1); /* 水平GRAM终止位置 */
  LCD_WriteReg(0x0052,yStart);         /* 垂直GRAM起始位置 */
  LCD_WriteReg(0x0053,yStart+yLong-1); /* 垂直GRAM终止位置 */ 
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : 将屏幕填充成指定的颜色，如清屏，则填充 0xffff
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Clear(uint16_t Color)
{
  uint32_t index=0;
  LCD_SetCursor(0,0); 
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  for(index=0;index<76800;index++)
   {
     LCD_RAM=Color;
   }
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : 获取指定座标的颜色值
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
  LCD_SetCursor(Xpos,Ypos);
  if( DeviceCode==0x7783 || DeviceCode==0x4531 || DeviceCode==0x8989 )
    return ( LCD_ReadRAM() );
  else
    return ( LCD_BGR2RGB(LCD_ReadRAM()) );
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : 在指定座标画点
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
  if ( ( Xpos > 239 ) ||( Ypos > 319 ) ) return;
  LCD_SetCursor(Xpos,Ypos);
  LCD_WriteRAM_Prepare();
  LCD_WriteRAM(point);
}

/******************************************************************************
* Function Name  : LCD_DrawPicture
* Description    : 在指定坐标范围显示一副图片
* Input          : - StartX: Row Start Coordinate 
*                  - StartY: Line Start Coordinate  
*				   - EndX: Row End Coordinate 
*				   - EndY: Line End Coordinate   
* Output         : None
* Return         : None
* Attention		 : 图片取模格式为水平扫描，16位颜色模式
*******************************************************************************/
void LCD_DrawPicture(uint16_t StartX,uint16_t StartY,uint16_t EndX,uint16_t EndY,uint16_t *pic)
{
  uint16_t  i;
  LCD_SetCursor(StartX,StartY);  
  LCD_WriteRAM_Prepare();
  for (i=0;i<(EndX*EndY);i++)
  {
      LCD_WriteRAM(*pic++);
  }
}


/******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : 画一条直线
* Input          : - x1: 行座标开始
*                  - y1: 列座标开始 
*				   - x2: 行座标结束
*				   - y2: 列座标结束  
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_DrawLine(int x1, int y1, int x2, int y2,uint16_t bkColor)  
{ 
  int x,y,dx,dy,Dx,Dy,e,i; 
  Dx=x2-x1; 
  Dy=y2-y1; 

  dx=fabs(x2-x1); 
  dy=fabs(y2-y1); 
  x=x1; 
  y=y1; 
  if(dy>dx) 
  { 
    e=-dy; 
    for(i=0;i<dy;i++) 
    { 
      LCD_SetPoint(x,y,bkColor); 
      if(Dy>=0) y++;   
      else y--;    
      e+=2*dx; 
      if(e>=0) 
      { 
        if(Dx>=0) x++; 
        else x--;  
        e-=2*dy; 
      } 
    } 
  } 
  else 
  { 
    e=-dx; 
    for(i=0;i<dx;i++) 
    { 
      LCD_SetPoint(x,y,bkColor); 
      if(Dx>=0) x++; 
      else x--; 
      e+=2*dy; 
      if(e>=0) 
      { 
        if(Dy>=0) y++; 
        else y--;
        e-=2*dx;
      } 
    } 
  } 
} 


#if ASCII_LIB > 0 
/******************************************************************************
* Function Name  : PutChar
* Description    : 将Lcd屏上任意位置显示一个字符
* Input          : - Xpos: 水平坐标 
*                  - Ypos: 垂直坐标  
*				   - c: 显示的字符
*				   - charColor: 字符颜色   
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void PutChar(unsigned short Xpos,unsigned short Ypos,unsigned char c,unsigned short charColor,unsigned short bkColor)
{
  unsigned short i=0;
  unsigned short j=0;
  unsigned char buffer[16];
  unsigned char tmp_char=0;
  GetASCIICode(buffer,c);  /* 取字模数据 */
  for (i=0;i<16;i++)
  {
    tmp_char=buffer[i];
    for (j=0;j<8;j++)
    {
      if ( (tmp_char >> 7-j) & 0x01 == 0x01)
        {
          LCD_SetPoint(Xpos+j,Ypos+i,charColor);  /* 字符颜色 */
        }
        else
        {
          LCD_SetPoint(Xpos+j,Ypos+i,bkColor);  /* 背景颜色 */
        }
    }
  }
}

/******************************************************************************
* Function Name  : GUI_Text
* Description    : 在指定座标显示字符串
* Input          : - Xpos: 行座标
*                  - Ypos: 列座标 
*				   - str: 字符串
*				   - charColor: 字符颜色   
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
uint8_t TempChar;
 do
  {
    TempChar=*str++;  
    PutChar(Xpos,Ypos,TempChar,Color,bkColor);    
    if (Xpos<232)
    {
      Xpos+=8;
    } 
    else if (Ypos<304)
    {
      Xpos=0;
      Ypos+=16;
    }   
    else
    {
      Xpos=0;
      Ypos=0;
    }    
  }
  while (*str!=0);
}

#endif

#if HZ_LIB > 0 
/******************************************************************************
* Function Name  : PutChinese
* Description    : 将Lcd屏上任意位置显示一个中文字
* Input          : - Xpos: 水平坐标 
*                  - Ypos: 垂直坐标  
*				   - str: 显示的中文字
*				   - Color: 字符颜色   
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void PutChinese(uint16_t Xpos,uint16_t Ypos,uint8_t *str,uint16_t Color,uint16_t bkColor)
{
  uint8_t i,j;
  uint8_t buffer[32];
  uint16_t tmp_char=0;
 
  GetGBKCode(buffer,str);  /* 取字模数据 */

  for (i=0;i<16;i++)
  {
    tmp_char=buffer[i*2];
	tmp_char=(tmp_char<<8);
	tmp_char|=buffer[2*i+1];
    for (j=0;j<16;j++)
    {
      if ( (tmp_char >> 15-j) & 0x01 == 0x01)
        {
          LCD_SetPoint(Xpos+j,Ypos+i,Color);  /* 字符颜色 */
        }
        else
        {
          LCD_SetPoint(Xpos+j,Ypos+i,bkColor);  /* 背景颜色 */
        }
    }
  }
}

/******************************************************************************
* Function Name  : GUI_Chinese
* Description    : 在指定座标显示字符串
* Input          : - Xpos: 行座标
*                  - Ypos: 列座标 
*				   - str: 字符串
*				   - charColor: 字符颜色   
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Chinese(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
  do
  {
    PutChinese(Xpos,Ypos,str++,Color,bkColor);
	str++;
   if (Xpos<224)
    {
      Xpos+=16;
    }
    else if (Ypos<304)
    {
      Xpos=0;
      Ypos+=16;
    }
    else
    {
      Xpos=0;
      Ypos=0;
    }       
  }
  while(*str!=0);
}  

#endif 

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB 改为 BBBBBGGGGGGRRRRR 格式
* Input          : - color: BRG 颜色值  
* Output         : None
* Return         : RGB 颜色值
* Attention		 : 内部函数调用
*******************************************************************************/
uint16_t LCD_BGR2RGB(uint16_t color)
{
  uint16_t  r, g, b, rgb;

  b = ( color>>0 )  & 0x1f;
  g = ( color>>5 )  & 0x3f;
  r = ( color>>11 ) & 0x1f;
 
  rgb =  (b<<11) + (g<<5) + (r<<0);

  return( rgb );
}

/******************************************************************************
* Function Name  : LCD_BackLight_Init
* Description    : LCD_BackLight Initializtion 
* Input          : None  
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_BackLight_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure; 
  TIM_OCInitTypeDef TIM_OCInitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO , ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 						 
  /*GPIOB Configuration:  PB5(TIM3 CH2) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);	 /* 管脚重引射 */     

  /* -----------------------------------------------------------------------
    TIM3CLK = 36 MHz, Prescaler = 35, TIM3 counter clock = 1 MHz
    TIM3 ARR Register = 999 => TIM3 Frequency = TIM3 counter clock/(ARR + 1)
    TIM3 Frequency = 1 KHz.
    TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 
  ----------------------------------------------------------------------- */
   /* 频率为 1MHz TIM3 counter clock = 1MHz */
  TimerPeriod = (uint16_t) (SystemCoreClock / 1000000) - 1;

  /* 输出频率=时钟/(ARR+1)，所以将输出一个 1Mhz/(999 + 1 )=1kHz 频率	 */
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;  /* 使用系统基础时钟 */
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseStructure.TIM_Prescaler = TimerPeriod;  /* 时钟预分频 */
  TIM_TimeBaseStructure.TIM_Period = 999;   /* TIM3 ARR Register */
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  
  /* TIM_Pulse用来控制占空比，他实际影响TIM的CCR2寄存器，程序中可随时更改该寄存器的值，可随时更改占空比。占空比=(CCRx/ARR)*100。*/
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; /* 输出模式 */
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = Channel2Pulse; /* 占空比参数 */
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC2Init(TIM3, &TIM_OCInitStructure);
  
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);    /* 使能TIM3在CCR2的预装储存器 */

  TIM_ARRPreloadConfig(TIM3, ENABLE);  /* 使能 ARR装载 */
  TIM_Cmd(TIM3, ENABLE);			   /* 使能TIM3 */

}
/******************************************************************************
* Function Name  : LCD_BackLight
* Description    : 调整液晶背光
* Input          : - percent: 背光亮度百分比 
* Output         : None
* Return         : 返回1成功 返回0失败
* Attention		 : None
*******************************************************************************/
FunctionalState LCD_BackLight( uint8_t percent)
{

  if( percent <= 100)
  {
    Channel2Pulse=percent*10;
	LCD_BackLight_Init(); 
	return ENABLE;
  } 
  else
    return DISABLE;
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/


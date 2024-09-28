
/*
* PB14 ：SPI2_MISO（青）
* PB15 ：SPI2_MOSI（赤）
* PB13 ：SPI2_SCLK（緑）
* PB12 ：SPI2_NSS （茶）
* PB0 ：LED 　　　　（黄色）
* PB1 ：DC/RS　　　（オレンジ）
* PB2 ：RESET　　　（紫　FCT：茶）
*
* VCC：白
* GND：黒
*
* ALT function mode : 5
*/



/**********************************************************/
/*******************SPIやGPIOの共通設定*************************/
/**********************************************************/

#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"
#include "LCD.h"

//extern void initialise_monitor_handles();

/*
#define COMMAND_LED_CTRL 0x50
#define COMMAND_SENSOR_READ 0x51
#define COMMAND_LED_READ 0x52
#define COMMAND_PRINT 0x53
#define COMMAND_ID_READ 0x54
*/

//MSP3521 command codes
#define Sleep_OUT                 0x11   // パラメータ数=0
#define Normal_Display_ON         0x13   // パラメータ数=0
#define Display_On                0x29   // パラメータ数=0
#define Column_Address_Set        0x2a   // パラメータ数=4
#define Page_Address_Set          0x2b   // パラメータ数=4
#define Memory Write              0x2C   // 先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み、パラメータ数=0
#define Memory_access_control     0x36   // パラメータ数=1、(MY=0,MX=1,MV=0)
#define Idle_Mode_ON              0x39   // パラメータ数=0
#define Interface_Pixel_Format    0x3A   // パラメータ数=1、 8色  (3 bits/pixel) (MCU interface format (SPI))
#define Memory_Write_Continue     0x3C   // パラメータ数=0
#define Frame_Rate_Control        0xB2   // In Idle Mode/8 Colors　パラメータ数=2
#define Display_Function_Control  0xB6   // パラメータ数=2(Extend Command)


#define LED_ON 1
#define LED_OFF 0


//たぶん、 約1ms of delay
void delay(uint32_t x)
{
	uint32_t i;
	i=20000*x;
for(uint32_t j = 0 ; j < i ; j ++);
}



void SPI2_GPIOInits(void)
{
GPIO_Handle_t SPIPins;

SPIPins.pGPIOx = GPIOB;
SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

//SCLK
SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
GPIO_Init(&SPIPins);

//MOSI
SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
GPIO_Init(&SPIPins);

//MISO
SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
GPIO_Init(&SPIPins);


//NSS
SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
GPIO_Init(&SPIPins);


}

void SPI2_Inits(void)
{

SPI_Handle_t SPI2handle;

SPI2handle.pSPIx = SPI2;
SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;    //全二重線
SPI2handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
SPI2handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV2; // 16/2 = 8MHzで通信する。
//ILI9488ではアイドル時はクロック=High、クロックの立ち上がり時にデータを取り込むので、SPIモード3(CPOL=1,CPHA=1)を使用します。
SPI2handle.SPIConfig.SPI_CPOL = SPI_CPOL_HIGH;
SPI2handle.SPIConfig.SPI_CPHA = SPI_CPHA_HIGH;
SPI2handle.SPIConfig.SPI_SSM = SPI_SSM_DI; //ハードウェアモードである。Hardware slave management enabled for NSS pin
SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS; //8ビットフォーマット

SPI_Init(&SPI2handle);
}

void GPIO_LcdInit(void)
{
	/*
	* PB0 ：LED 　　　　（黄色）
	* PB1 ：DC/RS　　　（オレンジ）
	* PB2 ：RESET　　　（紫　FCT：茶）
	* */

GPIO_Handle_t GpioLed,GpioDc,GpioReset;

//this is led gpio configuration
GpioLed.pGPIOx = GPIOB;
GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;


//this is DC gpio configuration
GpioDc.pGPIOx = GPIOB;
GpioDc.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_1;
GpioDc.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
GpioDc.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
GpioDc.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
GpioDc.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;


//this is Reset gpio configuration
GpioReset.pGPIOx = GPIOB;
GpioReset.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
GpioReset.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
GpioReset.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
GpioReset.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
GpioReset.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

/*
SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
 */

GPIO_PeriClockControl(GPIOB,ENABLE);

GPIO_Init(&GpioLed);
GPIO_Init(&GpioDc);
GPIO_Init(&GpioReset);

GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_0,ENABLE);

}


int main(void)
{


GPIO_LcdInit();

//this function is used to initialize the GPIO pins to behave as SPI2 pins
SPI2_GPIOInits();

//This function is used to initialize the SPI2 peripheral parameters
SPI2_Inits();


/*
* making SSOE 1 does NSS output enable.
* The NSS pin is automatically managed by the hardware.
* i.e when SPE=1 , NSS will be pulled to low
* and NSS pin will be high when SPE=0
*/
//SSOEは、マスターモードのハードウェアモードの時のみ関係。CR2>SSOEがセットされていると、NSSからローレベルが出力される。セットされていない場合、NSSは入力タイプになる（ローになるとハードウェアフォルトを引き起こしマスターになる。マルチマスタ機能で役立つ。）
//ハードウェアモードは、CR1>SSM=0の時。ハードウェアモードかは、SPI2handle.SPIConfig.SPI_SSMで書き込む。

SPI_SSOEConfig(SPI2,ENABLE);

//enable the SPI2 peripheral。SPIモジュールの有効化。
SPI_PeripheralControl(SPI2,ENABLE);

ILI9488_Reset();	// LCD のリセット

ILI9488_Init();		// LCDの初期化

while(1){

	color_bar();	// 画面　8色表示(白,黄色,シアン,緑,マゼンタ,赤,青,黒 ) 86.5[msec]

	delay(5);

	disp_black();		// 画面　黒  ( 80 [msec] at 16[MHz] ) 80[msecc]

	delay(5);
}

/*
//lets confirm SPI is not busy
while( SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG) );

//Disable the SPI2 peripheral
SPI_PeripheralControl(SPI2,DISABLE);

*/



return 0;

}

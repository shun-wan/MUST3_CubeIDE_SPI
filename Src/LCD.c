

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
/*******************60歳のアプリの改良*************************/
/**********************************************************/


#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"
#include "LCD.h"

uint8_t rgb111_data_buf[57600]; // 表示用バッファ RGB-111のデータを格納 (1pixelで3バイト)
			                   //  320*480*3 = 460800byte /8 = 57600


/*
 * Sleep OUT              (11h) , パラメータ数=0
 * Normal Display ON      (13h)
 * Display On コマンドレジスタ   (29h) , パラメータ数=0
 * コマンドレジスタ              (36h) (MY=0,MX=1,MV=0)
 * Idle Mode ON           (39h)
 * Interface Pixel Format (3Ah)
 * Frame Rate Control     (B2h)　パラメータ数=2 (In Idle Mode/8 Colors)
 * Display Function Control(B6h)　パラメータ数=2
 *
 */
void ILI9488_Init(void)
{
	uint8_t para[4];

	 spi_cmd_send(0x13,0,&para[0] );  // Normal Display ON (13h) , パラメータ数=0

	 spi_cmd_send(0x39,0,&para[0] );  // Idle Mode ON (39h) , パラメータ数=0

	 para[0] = 0x66;		  // Interface Pixel Format (3Ah)のパラメータ , 8色  (16bit/pixel) (MCU interface format (SPI))
	 spi_cmd_send(0x3A,1,&para[0] );  // Interface Pixel Format (3Ah), パラメータ数=1


	para[0] = 0x48;	   		   //  コマンドレジスタ 0x36用パラメータ (MY=0,MX=1,MV=0)
	spi_cmd_send( 0x36,1,&para[0]);    // Memory access control  コマンドレジスタ 0x36 , パラメータ数=1、(MY=0,MX=1,MV=0)


				          // Frame Rate Control (In Idle Mode/8 Colors) (B2h)　パラメータ数=2
	 para[0] = 0x00;    		  //  コマンドレジスタ 0xB1用パラメータ
	 para[1] = 0x10;
	 spi_cmd_send(0xB2 ,2,&para[0] );      // コマンドレジスタ 0xB2, パラメータ数=2


	 			   // (Extend Command)Display Function Control (B6h)　パラメータ数=2
	para[0] = 0x02;    	   //  コマンドレジスタ 0xB6用パラメータ
	para[1] = 0x02;            //
	para[2] = 0x3b;             //
	spi_cmd_send(0xB6,3,&para[0]);      //  コマンドレジスタ 0xB6 , パラメータ数=2


				    // Display ON (29h)
	spi_cmd_send(0x29,0,&para[0]);       // Display On コマンドレジスタ 0x29 , パラメータ数=0

	spi_cmd_send(0x11,0,&para[0]);      // Sleep OUT (11h) , パラメータ数=0

	delay(5);	    	    	   //  5msec待ち

}





//  ILI9488  LCD カラーバー(8色)
//   (320x480)
//
//   1byte に 2pixel分のRGB情報
//   1byte = **RG BRGB


// 白      11111 111111 11111 →　FF FF
// 黄色    11111 111111 00000 →　FF E0
// シアン    00000 111111 11111 →　7 FF
// 緑      00000 111111 00000 →　7 E0
// マゼンタ  11111 000000 11111 →　F8 1F
// 赤     11111 000000 00000 →　F8 00
// 青     00000 000000 11111 → 00 1F
// 黒                          00 00

void color_bar(void)
{
	uint32_t i;
	uint32_t num;

	uint8_t para[4];

	lcd_adrs_set(0,0, 319,479);	  // 書き込み範囲指定(コマンド 2aとコマンド 2b) (開始カラム=0, 開始ページ=0, 終了カラム=319, 終了ページ=479)

	num = 57600; 		// 8分割で57600

    for ( i = 0; i < num ; i++)	// ピクセルデータを流し込む (60ページ分)
    {
    	if(i%3==0){
    	rgb111_data_buf[i] = 0xFC;
    	}else if(i%3==1){
    	rgb111_data_buf[i] = 0x00;
    	}else if(i%3==2){
    	rgb111_data_buf[i] = 0x00;
    	}

    }

	 spi_cmd_send(0x2c,0,&para[0]);		// Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み
	 spi_data_send(num, &rgb111_data_buf[0]);  // データ送信

	 delay(5);		// 5 usec待ち


	 for(int j = 0; j<7;j++){

	    for ( i = 0; i < num ; i++)	// ピクセルデータを流し込む (60ページ分)
	    {
	    	if(i%3==0){
	    	rgb111_data_buf[i] = 0x3F;
	    	}else if(i%3==1){
	    	rgb111_data_buf[i] = 0x00;
	    	}else if(i%3==2){
	    	rgb111_data_buf[i] = 0x00;
	    	}
	    }
		 spi_cmd_send(0x3c,0,&para[0]);		// Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み
		 spi_data_send(num, &rgb111_data_buf[0]);  // データ送信

		 delay(5);		// 5 usec待ち

	 }

    	/*

   	 //白
   	 if(i<9600){
   	 if((i%2)==0){
    rgb111_data_buf[i] = 0xFF;
   	 }else{
    rgb111_data_buf[i] = 0xE0;
   	 }
    }
   	 //黄色
   	 else if(i<19200){
   	 if((i%2)==0){
    rgb111_data_buf[i] = 0xFF;
   	 }else{
    rgb111_data_buf[i] = 0xE0;
   	 }
    }
   	 //シアン
   	 else if(i<28800){
   	 if((i%2)==0){
    rgb111_data_buf[i] = 0x7;
   	 }else{
    rgb111_data_buf[i] = 0xFF;
   	 }
    }
   	 //緑
   	 else if(i<38400){
   	 if((i%2)==0){
    rgb111_data_buf[i] = 0x7;
   	 }else{
    rgb111_data_buf[i] = 0xE0;
   	 }
    }
   	 //マゼンダ
   	 else if(i<48000){
   	 if((i%2)==0){
    rgb111_data_buf[i] = 0xF8;
   	 }else{
    rgb111_data_buf[i] = 0x1F;
   	 }
    }
   	 //赤
   	 else if(i<57600){
   	 if((i%2)==0){
    rgb111_data_buf[i] = 0xF8;
   	 }else{
    rgb111_data_buf[i] = 0x00;
   	 }
    }
   	 //青
   	 else if(i<67200){
   	 if((i%2)==0){
    rgb111_data_buf[i] = 0x00;
   	 }else{
    rgb111_data_buf[i] = 0x1F;
   	 }
    }
   	 //黒
   	 else if(i<76800){
   	 if((i%2)==0){
    rgb111_data_buf[i] = 0x00;
   	 }else{
    rgb111_data_buf[i] = 0x00;
   	 }
    }

   	 */
}


//
// 画面を黒にする
//
void disp_black(void)
{
	uint32_t i;
	uint8_t para[4];

	lcd_adrs_set(0,0, 319,479);	  // 書き込み範囲指定(コマンド 2aとコマンド 2b) (開始カラム=0, 開始ページ=0, 終了カラム=319, 終了ページ=479)

         for ( i = 0; i < 57600 ; i++)	// ピクセルデータを流し込む (40ページ分)  (1page書くのに 480 / 2 = 240byte必要 , (1 pixel=2 byte) )
         {
	     rgb111_data_buf[i] = 0xFF;        //  黒 (2pixel 分)
         }

	 spi_cmd_send(0x2c,0,&para[0]);		// Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み
	 spi_data_send(57600, &rgb111_data_buf[0]);  // データ送信


	 for(int j = 0; j<7;j++){

	 spi_cmd_send(0x3c,0,&para[0]);	// Memory Write Continue (3Ch)
	 spi_data_send(57600, &rgb111_data_buf[0]);  // データ送信

	 }

}

// LCD のリセット
//
void ILI9488_Reset(void)
{


	GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_2,RESET);      // LCD リセット状態にする
	delay(1);		        // 1[msec] 待つ

	GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_2,SET);      //  セット状態にする
	delay(200);	        // 200[msec] 待つ


}


//  表示範囲の設定
// 入力:
//  col: 開始カラム(x), page(row):開始ページ(y)
//  col2:終了カラム, page2(row2): 終了ページ
//
void lcd_adrs_set( uint16_t col, uint16_t page, uint16_t col2, uint16_t page2)
{
	uint8_t para[8];

	 para[0] = (uint8_t) ((0xff00 & col) >> 8);     //  SC[15:8]　スタートカラム(16bit)の上位バイト
	 para[1] = (uint8_t) (0x00ff & col);            //  SC[7:0]         :　　　　　　　　下位バイト
	 para[2] = (uint8_t) ((0xff00 & col2) >> 8);    //  EC[15:8]　終了カラム(16bit)の上位バイト
	 para[3] = (uint8_t) (0x00ff & col2);           //  EC[7:0]         :　　　　　　　　下位バイト

	 spi_cmd_send(0x2a,4,&para[0]);      // Column Address Set コマンドレジスタ 0x2a , パラメータ数=4


	 para[0] = (uint8_t) ((0xff00 & page ) >> 8);    //  SP[15:8]　スタートページ(16bit)の上位バイト
	 para[1] = (uint8_t) (0x00ff & page);            //  SP[7:0]         :　　　　　　　　下位バイト
	 para[2] = (uint8_t) ((0xff00 & page2 ) >> 8);    // EP[15:8]　終了ページ(16bit)の上位バイト
	 para[3] = (uint8_t) (0x00ff & page2);            // EP[7:0]         :　　　　　　　　下位バイト

	 spi_cmd_send(0x2b,4,&para[0]);      // Page Address Set コマンドレジスタ 0x2b , パラメータ数=4

}

// LCDコントローラ(ILI9488)への表示データを送信 　(送信割り込み使用)
void spi_data_send( uint32_t para_num, uint8_t *data_pt)
{
	GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_1,ENABLE);   // データ指定 (D/Cポート= High)

	SPI_SendData(SPI2,data_pt,para_num);                 //データ送信

	}


// LCDコントローラ(ILI9488)へのコマンドとパラメータ(データ)送信
//
// コマンドに送信パラメータが無い場合、cmdだけ送信。para_num = 0 設定。
// コマンドに送信パラメータがある場合、cmd送信後、para[]をpara_num分送信。
//
//　　入力: cmd コマンド(=レジスタID) ( 8 bit)
//          sd_num :送信パラメータ数
//          *para_pt: 送信パラメータの格納位置。送信パラメータは、para[]に格納されている。
//
//
void spi_cmd_send( uint8_t cmd, uint32_t para_num, uint8_t *para_pt)
{

	GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_1,DISABLE);   // コマンド送信用 (D/Cポート= LOW)

	SPI_SendData(SPI2,&cmd,1);                 //コマンド送信

	if ( para_num >  0 ) {    // 送信パラメータ付きの場合

		GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_1,ENABLE);   // パラメータ指定 (D/Cポート= High)

		SPI_SendData(SPI2,para_pt,para_num);                 //コマンド送信

	}
}







/**********************************************************/
/********************平坂の方のアプリ改良************************/
/**********************************************************/

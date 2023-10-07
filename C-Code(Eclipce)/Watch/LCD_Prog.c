/*
 * LCD_Prog.c
 *
 *  Created on: ???/???/????
 *      Author: vmk9p-'
 */


#include <avr\delay.h>

#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "DIO_Int.h"
#include "LCD_Int.h"
#include "LCD_Confg.h"
#include "LCD_Priv.h"


#if (BIT_MODE == BIT_MODE8)


/******************************************************************************/

void LCD_vidSendCommand(u8 u8CmdCpy)
{
	/*Set 0 for RS for COMMAND*/
	DIO_vidSetPinVal(CONTROL_PORT,REG_SELC_RS,DIO_LOW);

	/*Set 0 for W/R for Write*/
	DIO_vidSetPinVal(CONTROL_PORT,Read_Write_SELEC_RW,DIO_LOW);

	/*copy the command to The Data Port*/
	DIO_vidSetPortVal(DATA_PORT,u8CmdCpy);

	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);
	/*Make aPulse 0->1*/
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);

}

void LCD_vidWriteCharctr(u8 u8DataCpy)
{
	/*Set 1 for RS for DATA*/
	DIO_vidSetPinVal(CONTROL_PORT,REG_SELC_RS,DIO_HIGH);

	/*Set 0 for W/R for Write*/
	DIO_vidSetPinVal(CONTROL_PORT,Read_Write_SELEC_RW,DIO_LOW);

	/*copy the DATA to The Data Port*/
	DIO_vidSetPortVal(DATA_PORT,u8DataCpy);

	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);
	/*Make aPulse 0->1*/
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);


	u8 u8Address = LCD_u8ReadBusyFlagAndAddress();
	if(u8Address == END_LINE_1_ADD)
	{
		LCD_vidGotoxy(1,0);
	}
	else if(u8Address == END_LINE_2_ADD)
	{
		LCD_vidGotoxy(0,0);
	}
}

void LCD_vidGotoxy (u8 Y,u8 X)
{
	if((Y<2)&&(X<16))
	{
	u8 u8Address = (Y*0x40)+X;
	SET_BIT(u8Address,7);
	LCD_vidSendCommand(u8Address);
	_delay_us(50);
	}
}

void LCD_vidWriteStringAtPossition (u8* pu8StringCpy, u8 Y ,u8 X)
{
	/*Go To The XY Cell*/
	LCD_vidGotoxy(Y,X);
	/*Check If The Coordinates Is Outside The LCD */
	if((Y<2)&&(X<16))
		{
			u8 u8Index =0;
			/*Check If The String Finished OR The LCD Cells Finished */
			while(pu8StringCpy[u8Index]!='\0')
			{
				LCD_vidWriteCharctr(pu8StringCpy[u8Index]);
				u8Index++;
			}
		}
}

void LCD_vidWriteString (u8* pu8StringCpy)
{
	/*Check If The Coordinates Is Outside The LCD */
	u8 u8Index =0;
	/*Check If The String Finished OR The LCD Cells Finished */
	while(pu8StringCpy[u8Index]!='\0')
	{
		LCD_vidWriteCharctr(pu8StringCpy[u8Index]);
		u8Index++;
	}

}

void LCD_vidInit(void)
{
	/*Define The Port Of the Data And The 3 Pins of Control */
	DIO_vidSetPortDir(DATA_PORT,DIO_OUTPUT);
	DIO_vidSetPinDir(CONTROL_PORT,REG_SELC_RS,DIO_OUTPUT);
	DIO_vidSetPinDir(CONTROL_PORT,Read_Write_SELEC_RW,DIO_OUTPUT);
	DIO_vidSetPinDir(CONTROL_PORT,ENABLE_SIG,DIO_OUTPUT);

	_delay_ms(30);

	/* Function Set */
	LCD_vidSendCommand(0x38);
	_delay_us(50);

	/* Display ON/OFF Control */
	LCD_vidSendCommand(0x0f);
	_delay_us(50);

	/* Display Clear */
	LCD_vidSendCommand(0x01);
	_delay_ms(5);

	/* Entry Mode Set */
	LCD_vidSendCommand(0x06);
	_delay_ms(5);
}

void LCD_vidClearScrean(void)
{
	LCD_vidSendCommand(0x01);
	_delay_ms(2);
}

void LCD_vidReturnHome(void)
{
	LCD_vidSendCommand(0b00000010);
	_delay_ms(2);
}

u8 LCD_u8ReadBusyFlagAndAddress(void)
{
	DIO_vidSetPortDir(DATA_PORT,DIO_INPUT);
	DIO_vidSetPinVal(CONTROL_PORT,REG_SELC_RS,DIO_LOW);
	DIO_vidSetPinVal(CONTROL_PORT,Read_Write_SELEC_RW,DIO_HIGH);
	//DIO_vidSetPortVal(DATA_PORT,0x01);
	_delay_ms(5);
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);
	_delay_ms(5);
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);

	u8 u8Address =DIO_u8GetPortVal(DATA_PORT);
	CLR_BIT(u8Address,7);

	DIO_vidSetPortDir(DATA_PORT,DIO_OUTPUT);
	return u8Address;
}

void LCD_vidTurnOff(void)
{
	LCD_vidSendCommand(0x08);//00001000
	_delay_us(50);
}

void LCD_vidTurnOn(void)
{
LCD_vidSendCommand(0x0f);
	_delay_us(50);
}

void LCD_vidStoreSpecialChar(u8 *poiPattarn , u8 u8CGRAMIndex)
{
	/*The CGRAM has 64 location And Every 8 Bytes Used For Store One Char
	 * Address Of Char1 = 0x00
	 * Address Of Char1 = 0x08
	 * Address Of Char1 = 0x10(16)
	 * and so on*/
	if(u8CGRAMIndex<8)
	{
		u8 u8OldAddress = LCD_u8ReadBusyFlagAndAddress();
		SET_BIT(u8OldAddress,7);
		u8 u8Address = 8 *u8CGRAMIndex;
		SET_BIT(u8Address,6);
		CLR_BIT(u8Address,7);
		//01AddressCounter
		LCD_vidSendCommand(u8Address);

		for(u8 u8Index = 0;u8Index < 8 ;u8Index++)
		{
			LCD_vidWriteCharctr(poiPattarn[u8Index]);
		}
		//LCD_vidReturnHome();
		LCD_vidSendCommand(u8OldAddress);
	}
}

void LCD_vidDisplaySpecialChar(u8 u8CGRAMIndex)

{
	if(u8CGRAMIndex<8)
		{
			LCD_vidWriteCharctr(u8CGRAMIndex);
		}
}

/******************************************************************************/
/******************************************************************************/

#elif (BIT_MODE == BIT_MODE4)

/******************************************************************************/
static u8 LocalInitFunc =0;

void LCD_vidSendCommand(u8 u8CmdCpy)
{

	/*Set 0 for RS for COMMAND*/
	DIO_vidSetPinVal(CONTROL_PORT,REG_SELC_RS,DIO_LOW);

	/*Set 0 for W/R for Write*/
	DIO_vidSetPinVal(CONTROL_PORT,Read_Write_SELEC_RW,DIO_LOW);

	/*copy the command to The Data Port*/
	u8 temp = GET_BIT(u8CmdCpy,7);
	if(temp == 1)
		temp = 0xff;
	DIO_vidSetPinVal(DATA_PORT,DIO_PIN7,temp);

	temp = GET_BIT(u8CmdCpy,6);
		if(temp == 1)
			temp = 0xff;
	DIO_vidSetPinVal(DATA_PORT,DIO_PIN6,temp);

	temp = GET_BIT(u8CmdCpy,5);
			if(temp == 1)
				temp = 0xff;
	DIO_vidSetPinVal(DATA_PORT,DIO_PIN5,temp);

	temp = GET_BIT(u8CmdCpy,4);
			if(temp == 1)
				temp = 0xff;
	DIO_vidSetPinVal(DATA_PORT,DIO_PIN4,temp);

	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);
	/*Make aPulse 0->1*/
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);


	if(LocalInitFunc)
	{
		temp = GET_BIT(u8CmdCpy,3);
			if(temp == 1)
				temp = 0xff;
			DIO_vidSetPinVal(DATA_PORT,DIO_PIN7,temp);

			temp = GET_BIT(u8CmdCpy,2);
				if(temp == 1)
					temp = 0xff;
			DIO_vidSetPinVal(DATA_PORT,DIO_PIN6,temp);

			temp = GET_BIT(u8CmdCpy,1);
					if(temp == 1)
						temp = 0xff;
			DIO_vidSetPinVal(DATA_PORT,DIO_PIN5,temp);

			temp = GET_BIT(u8CmdCpy,0);
					if(temp == 1)
						temp = 0xff;
			DIO_vidSetPinVal(DATA_PORT,DIO_PIN4,temp);

		DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);
		/*Make aPulse 0->1*/
		DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);

	}
}

void LCD_vidTurnOn(void)
{
LCD_vidSendCommand(0x0f);
	_delay_us(50);
}

void LCD_vidTurnOff(void)
{
	LCD_vidSendCommand(0x08);//00001000
	_delay_us(50);
}

void LCD_vidInit(void)
{
	/*Define The Port Of the Data And The 3 Pins of Control */
	DIO_vidSetPinDir(DATA_PORT,DIO_PIN7,DIO_OUTPUT);
	DIO_vidSetPinDir(DATA_PORT,DIO_PIN6,DIO_OUTPUT);
	DIO_vidSetPinDir(DATA_PORT,DIO_PIN5,DIO_OUTPUT);
	DIO_vidSetPinDir(DATA_PORT,DIO_PIN4,DIO_OUTPUT);

	DIO_vidSetPinDir(CONTROL_PORT,REG_SELC_RS,DIO_OUTPUT);
	DIO_vidSetPinDir(CONTROL_PORT,Read_Write_SELEC_RW,DIO_OUTPUT);
	DIO_vidSetPinDir(CONTROL_PORT,ENABLE_SIG,DIO_OUTPUT);

	_delay_ms(30);

	/* Function Set */
	LCD_vidSendCommand(0x20);
	LCD_vidSendCommand(0x20);
	LCD_vidSendCommand(0x80);
	_delay_us(50);

	/* Display ON/OFF Control */
	LCD_vidSendCommand(0x00);
	LCD_vidSendCommand(0xf0);
	_delay_us(50);

	/* Display Clear */
	LCD_vidSendCommand(0x00);
	LCD_vidSendCommand(0x10);
	_delay_ms(5);

	/* Entry Mode Set */
	LCD_vidSendCommand(0x00);
	LCD_vidSendCommand(0x60);
	_delay_ms(5);
	LocalInitFunc = 1;
}

void LCD_vidGotoxy (u8 Y,u8 X)
{
	if((Y<2)&&(X<16))
	{
	u8 u8Address = (Y*0x40)+X;
	SET_BIT(u8Address,7);
	LCD_vidSendCommand(u8Address);
	_delay_us(50);
	}
}

void LCD_vidWriteStringAtPossition (u8* pu8StringCpy, u8 Y ,u8 X)
{
	/*Go To The XY Cell*/
	LCD_vidGotoxy(Y,X);
	/*Check If The Coordinates Is Outside The LCD */
	if((Y<2)&&(X<16))
		{
			u8 u8Index =0;
			/*Check If The String Finished OR The LCD Cells Finished */
			while(pu8StringCpy[u8Index]!='\0')
			{
				LCD_vidWriteCharctr(pu8StringCpy[u8Index]);
				u8Index++;
			}
		}
}

void LCD_vidWriteCharctr(u8 u8DataCpy)
{
	/*Set 1 for RS for DATA*/
	DIO_vidSetPinVal(CONTROL_PORT,REG_SELC_RS,DIO_HIGH);

	/*Set 0 for W/R for Write*/
	DIO_vidSetPinVal(CONTROL_PORT,Read_Write_SELEC_RW,DIO_LOW);

	/*copy the DATA to The Data Port*/
	u8 temp = GET_BIT(u8DataCpy,7);
		if(temp == 1)
			temp = 0xff;
		DIO_vidSetPinVal(DATA_PORT,DIO_PIN7,temp);

		temp = GET_BIT(u8DataCpy,6);
			if(temp == 1)
				temp = 0xff;
		DIO_vidSetPinVal(DATA_PORT,DIO_PIN6,temp);

		temp = GET_BIT(u8DataCpy,5);
				if(temp == 1)
					temp = 0xff;
		DIO_vidSetPinVal(DATA_PORT,DIO_PIN5,temp);

		temp = GET_BIT(u8DataCpy,4);
				if(temp == 1)
					temp = 0xff;
		DIO_vidSetPinVal(DATA_PORT,DIO_PIN4,temp);


	/*Make aPulse 0->1*/
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);


	temp = GET_BIT(u8DataCpy,3);
			if(temp == 1)
				temp = 0xff;
			DIO_vidSetPinVal(DATA_PORT,DIO_PIN7,temp);

			temp = GET_BIT(u8DataCpy,2);
				if(temp == 1)
					temp = 0xff;
			DIO_vidSetPinVal(DATA_PORT,DIO_PIN6,temp);

			temp = GET_BIT(u8DataCpy,1);
					if(temp == 1)
						temp = 0xff;
			DIO_vidSetPinVal(DATA_PORT,DIO_PIN5,temp);

			temp = GET_BIT(u8DataCpy,0);
					if(temp == 1)
						temp = 0xff;
			DIO_vidSetPinVal(DATA_PORT,DIO_PIN4,temp);

	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);
	/*Make aPulse 0->1*/
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);

	u8 u8Address = LCD_u8ReadBusyFlagAndAddress();
	if(u8Address == END_LINE_1_ADD)
	{
		LCD_vidGotoxy(1,0);
	}
	else if(u8Address == END_LINE_2_ADD)
	{
		LCD_vidGotoxy(0,0);
	}
}

u8 LCD_u8ReadBusyFlagAndAddress(void)
{
	DIO_vidSetPinDir(DATA_PORT,DIO_PIN7,DIO_INPUT);
	DIO_vidSetPinDir(DATA_PORT,DIO_PIN6,DIO_INPUT);
	DIO_vidSetPinDir(DATA_PORT,DIO_PIN5,DIO_INPUT);
	DIO_vidSetPinDir(DATA_PORT,DIO_PIN4,DIO_INPUT);

	DIO_vidSetPinVal(CONTROL_PORT,REG_SELC_RS,DIO_LOW);
	DIO_vidSetPinVal(CONTROL_PORT,Read_Write_SELEC_RW,DIO_HIGH);
	_delay_us(50);

	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);
	_delay_ms(5);
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);


	u8 u8Address = DIO_u8GetPortVal(DATA_PORT);

	u8Address = u8Address & (0x70);

	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_LOW);
	_delay_ms(5);
	DIO_vidSetPinVal(CONTROL_PORT,ENABLE_SIG,DIO_HIGH);


	u8 u8Address0 =DIO_u8GetPortVal(DATA_PORT);

	u8Address|= u8Address0>>4;

	CLR_BIT(u8Address,7);

	DIO_vidSetPortDir(DATA_PORT,DIO_OUTPUT);
	return u8Address;
}

void LCD_vidStoreSpecialChar(u8 *poiPattarn , u8 u8CGRAMIndex)
{
	/*The CGRAM has 64 location And Every 8 Bytes Used For Store One Char
	 * Address Of Char1 = 0x00
	 * Address Of Char1 = 0x08
	 * Address Of Char1 = 0x10(16)
	 * and so on*/
	if(u8CGRAMIndex<8)
	{
		u8 u8OldAddress = LCD_u8ReadBusyFlagAndAddress();
		SET_BIT(u8OldAddress,7);
		u8 u8Address = 8 *u8CGRAMIndex;
		SET_BIT(u8Address,6);
		CLR_BIT(u8Address,7);
		//01AddressCounter
		LCD_vidSendCommand(u8Address);

		for(u8 u8Index = 0;u8Index < 8 ;u8Index++)
		{
			LCD_vidWriteCharctr(poiPattarn[u8Index]);
		}
		//LCD_vidReturnHome();
		LCD_vidSendCommand(u8OldAddress);
	}
}

void LCD_vidDisplaySpecialChar(u8 u8CGRAMIndex)

{
	if(u8CGRAMIndex<8)
		{
			LCD_vidWriteCharctr(u8CGRAMIndex);
		}
}

void LCD_vidClearScrean(void)
{
	LCD_vidSendCommand(0x01);
	_delay_ms(2);
}

void LCD_vidReturnHome(void)
{
	LCD_vidSendCommand(0b00000010);
	_delay_ms(2);
}

void LCD_vidWriteString (u8* pu8StringCpy)
{
	/*Check If The Coordinates Is Outside The LCD */
	u8 u8Index =0;
	/*Check If The String Finished OR The LCD Cells Finished */
	while(pu8StringCpy[u8Index]!='\0')
	{
		LCD_vidWriteCharctr(pu8StringCpy[u8Index]);
		u8Index++;
	}

}

void LCD_vidWriteflt(f32 f32DataCpy)
{
	lu32 u32DataCpyInt ;
if(f32DataCpy<0)
{
	LCD_vidWriteCharctr('-');
	u32DataCpyInt =  (lu32)(0-(f32DataCpy *Get_Power(10,FLOAT_NUMS)));
}
else
{
	u32DataCpyInt =  (lu32)(f32DataCpy *Get_Power(10,FLOAT_NUMS));
}

	if(u32DataCpyInt != 0)
	{
		u8 array[10] = {0};
		u8 index = 0;
		while(u32DataCpyInt !=0)
		{
			array[index] = (u32DataCpyInt%10)+('0');
			u32DataCpyInt=u32DataCpyInt/10;
			index++;
		}

		u8 c0 = index;
		for(;c0>0;c0--)
		{
			if(c0 != FLOAT_NUMS)
			{
			LCD_vidWriteCharctr(array[c0-1]);
			}
			else
			{
			LCD_vidWriteCharctr('.');
			LCD_vidWriteCharctr(array[c0-1]);
			}
		}
	}
	else
	{//LCD_vidWriteCharctr('0');
	//LCD_vidWriteCharctr('.');
	LCD_vidWriteCharctr('0');}

}

u32 Get_Power(u8 value ,u8 power)
{
	u32 Ret = 1;
	for(u8 counter = power;counter>0;counter--)
	{
		Ret = Ret *value;
	}
	return Ret;
}


#endif

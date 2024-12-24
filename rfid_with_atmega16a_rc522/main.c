#define FRE 8
#define F_CPU 8000000UL
#include <stdio.h>
#include <util/delay.h>
#include "rc522.h"
#include "lcd.h"


void displayCardID(uint8_t *data);

int main(void)
{
	// Khoi tao SPI
	SPI_MasterInit();
	
	//Khoi tao LCD
	DDRD |= (1<<PD5);
	PORTD &= ~(1<<PD5);
	PORTC |= 0x0F;
	LCD4_INIT(0,0);
	
	// Khoi tao RC522
	RC522_init();
	RC522_begin();
	while(1)
	{
		uint8_t status;
		uint8_t data[MAX_LEN];
		status = RC522_requestTag(MF1_REQIDL, data);
		if (status == MI_OK)
		{
			status = RC522_antiCollision(data);
			displayCardID(data);
			RC522_selectTag(data);
			// Stop the tag and get ready for reading a new tag.
			RC522_haltTag();
		}
	}
}

void displayCardID(uint8_t *data) {
	char hex[11];
	int offset = 0;
	for (int i = 0; i < 5; i++) {
		offset += sprintf(hex + offset, "%02X", data[i]);
	}
	hex[offset] = '\0';
	LCD4_CUR_GOTO(1,0);
	LCD4_OUT_STR(hex);
}

#include "rc522.h"


int _sad = 4;    // Select pin
int _reset = 0; // Reset pin
/************************************************************************/
/*						SPI			                                    */
/************************************************************************/
void SPI_MasterInit(void)
{
	DDRB |= (1<<SCK_PIN)|(1<<MOSI_PIN)|(1<<SS);
	//PORTB|=(1<<MISO_PIN);
	SPCR |=	(1<<SPE)|(1<<MSTR)|(1<<SPR0);
	sbi(PORTB,SS);
}

uint8_t SPI_MasterTransmit(uint8_t cData)
{
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	/* Return data register */
	return SPDR;
}

/************************************************************************/
/*					RC522                                               */
/************************************************************************/

void RC522_init(){
	sbi(DDRB, _sad);       // Set digital as OUTPUT to connect it to the RFID /ENABLE pin
	sbi(PORTB,_sad);

	sbi(DDRB, _reset);       // Set digital as OUTPUT to connect it to the RFID /ENABLE pin
	sbi(PORTB,_reset);
}

uint8_t RC522_getFirmwareVersion() {
	uint8_t response;
	response = RC522_readFromRegister(0x37);
	return response;
}

void RC522_writeToRegister(uint8_t addr, uint8_t val) {
	cbi(PORTB,_sad);
	//Address format: 0XXXXXX0
	//send((addr<<1)&0x7E);
	SPI_MasterTransmit((addr<<1)&0x7E);
	//send(val);
	SPI_MasterTransmit(val);
	sbi(PORTB,_sad);
}

uint8_t RC522_readFromRegister(uint8_t addr) {
	uint8_t val;
	cbi(PORTB, _sad);
	SPI_MasterTransmit(((addr<<1)&0x7E) | 0x80);
	val =SPI_MasterTransmit(0x00);
	sbi(PORTB,_sad);
	return val;
}

void RC522_reset() {
	RC522_writeToRegister(CommandReg, PCD_SoftReset);
}

void RC522_setBitMask(uint8_t addr, uint8_t mask) {
	uint8_t current;
	current = RC522_readFromRegister(addr);
	RC522_writeToRegister(addr, current | mask);
}

void RC522_clearBitMask(uint8_t addr, uint8_t mask) {
	uint8_t current;
	current = RC522_readFromRegister(addr);
	RC522_writeToRegister(addr, current & (~mask));
}

void RC522_begin() {
	sbi(PORTB,_sad);

	RC522_reset();

	//Timer: TPrescaler*TreloadVal/6.78MHz = 24ms == 3390 * 48 / 6.78
	RC522_writeToRegister(TModeReg, 0x8D);       // Tauto=1; f(Timer) = 6.78MHz/TPreScaler
	RC522_writeToRegister(TPrescalerReg, 0x3E);  // TModeReg[3..0] + TPrescalerReg
	RC522_writeToRegister(TReloadRegL, 30);		//48
	RC522_writeToRegister(TReloadRegH, 0);

	RC522_writeToRegister(TxAutoReg, 0x40);      // 100%ASK
	RC522_writeToRegister(ModeReg, 0x3D);        // CRC initial value 0x6363

	RC522_setBitMask(TxControlReg, 0x03);        // Turn antenna on.
}

int RC522_commandTag(uint8_t cmd, uint8_t *data, int dlen, uint8_t *result, int *rlen) {
	int status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits, n;
	int i;

	switch (cmd) {
		case PCD_MFAuthent:
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		case PCD_Transceive:
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		default:
			break;
	}

	RC522_writeToRegister(CommIEnReg, irqEn|0x80);    // interrupt request
	RC522_clearBitMask(CommIrqReg, 0x80);             // Clear all interrupt requests bits.
	RC522_setBitMask(FIFOLevelReg, 0x80);             // FlushBuffer=1, FIFO initialization.

	RC522_writeToRegister(CommandReg, PCD_Idle);  // No action, cancel the current command.

	// Write to FIFO
	for (i=0; i < dlen; i++) {
		RC522_writeToRegister(FIFODataReg, data[i]);
	}

	// Execute the command.
	RC522_writeToRegister(CommandReg, cmd);
	if (cmd == PCD_Transceive) {
		RC522_setBitMask(BitFramingReg, 0x80);  // StartSend=1, transmission of data starts
	}

	// Waiting for the command to complete so we can receive data.
	i = 25; // Max wait time is 25ms.
	do {
		_delay_ms(5);
		// CommIRqReg[7..0]
		// Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = RC522_readFromRegister(CommIrqReg);
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	RC522_clearBitMask(BitFramingReg, 0x80);  // StartSend=0

	if (i != 0) { // Request did not time out.
		if(!(RC522_readFromRegister(ErrorReg) & 0x1D)) {  // BufferOvfl Collerr CRCErr ProtocolErr
			status = MI_OK;
			if (n & irqEn & 0x01) {
				status = MI_NOTAGERR;
			}

			if (cmd == PCD_Transceive) {
				n = RC522_readFromRegister(FIFOLevelReg);
				lastBits = RC522_readFromRegister(ControlReg) & 0x07;
				if (lastBits) {
					*rlen = (n-1)*8 + lastBits;
					} else {
					*rlen = n*8;
				}

				if (n == 0) {
					n = 1;
				}

				if (n > MAX_LEN) {
					n = MAX_LEN;
				}

				// Reading the recieved data from FIFO.
				for (i=0; i<n; i++) {
					result[i] = RC522_readFromRegister(FIFODataReg);
				}
			}
			} else {
			status = MI_ERR;
		}
	}
	return status;
}

int RC522_requestTag(uint8_t mode, uint8_t *data) {
	int status, len;
	RC522_writeToRegister(BitFramingReg, 0x07);  // TxLastBists = BitFramingReg[2..0]

	data[0] = mode;
	status = RC522_commandTag(PCD_Transceive, data, 1, data, &len);

	if ((status != MI_OK) || (len != 0x10)) {
		status = MI_ERR;
	}

	return status;
}

int RC522_antiCollision(uint8_t *serial) {
	int status, i, len;
	uint8_t check = 0x00;

	RC522_writeToRegister(BitFramingReg, 0x00);  // TxLastBits = BitFramingReg[2..0]

	serial[0] = MF1_ANTICOLL;
	serial[1] = 0x20;
	status = RC522_commandTag(PCD_Transceive, serial, 2, serial, &len);
	len = len / 8; // len is in bits, and we want each byte.
	if (status == MI_OK) {
		// The checksum of the tag is the ^ of all the values.
		for (i = 0; i < len-1; i++) {
			check ^= serial[i];
		}
		// The checksum should be the same as the one provided from the
		// tag (serial[4]).
		if (check != serial[i]) {
			status = MI_ERR;
		}
	}

	return status;
}

void RC522_calculateCRC(uint8_t *data, int len, uint8_t *result) {
	int i;
	uint8_t n;

	RC522_clearBitMask(DivIrqReg, 0x04);   // CRCIrq = 0
	RC522_setBitMask(FIFOLevelReg, 0x80);  // Clear the FIFO pointer

	//Writing data to the FIFO.
	for (i = 0; i < len; i++) {
		RC522_writeToRegister(FIFODataReg, data[i]);
	}
	RC522_writeToRegister(CommandReg, PCD_CalcCRC);

	// Wait for the CRC calculation to complete.
	i = 0xFF;
	do {
		n = RC522_readFromRegister(DivIrqReg);
		i--;
	} while ((i != 0) && !(n & 0x04));  //CRCIrq = 1

	// Read the result from the CRC calculation.
	result[0] = RC522_readFromRegister(CRCResultRegL);
	result[1] = RC522_readFromRegister(CRCResultRegM);
}

uint8_t RC522_selectTag(uint8_t *serial) {
	int i, status, len;
	uint8_t sak;
	uint8_t buffer[9];

	buffer[0] = MF1_SELECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i+2] = serial[i];
	}
	RC522_calculateCRC(buffer, 7, &buffer[7]);

	status = RC522_commandTag(PCD_Transceive, buffer, 9, buffer, &len);

	if ((status == MI_OK) && (len == 0x18)) {
		sak = buffer[0];
	}
	else {
		sak = 0;
	}

	return sak;
}

int RC522_haltTag() {
	int status, len;
	uint8_t buffer[4];

	buffer[0] = MF1_HALT;
	buffer[1] = 0;
	RC522_calculateCRC(buffer, 2, &buffer[2]);
	status = RC522_commandTag(PCD_Transceive, buffer, 4, buffer, &len);
	RC522_clearBitMask(Status2Reg, 0x08);  // turn off encryption
	return status;
}
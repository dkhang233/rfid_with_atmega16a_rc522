#ifndef RC522_H_
#define RC522_H_

#define F_CPU 8000000UL // Define CPU frequency for <util/delay.h> 
#include <avr/io.h>
#include <util/delay.h>
//#include <avr/sfr_defs.h>

//	--------------------------------------------------------------------
//	|							RC522									|
//	--------------------------------------------------------------------
//	|  SDA  |  SCK  |  MOSI  |  MISO  |  IRQ  |  GND  |  RST  |  3.3 V  | 
//	---------------------------------------------------------------------
//	|  PB4  |  PB7  |  PB5   |  PB6   |   -   |  GND  |  PB0  |  3.3 V  |  
//	---------------------------------------------------------------------
//	|						  ATmega16a									|
//	---------------------------------------------------------------------



#define sbi(PORT,bit) {PORT |=(1<<bit);}
#define cbi(PORT,bit) {PORT &=~(1<<bit);}
#define SCK_PIN		7
#define MOSI_PIN	5
#define MISO_PIN 	6
#define SS			4


#define     MAX_LEN               16        // Maximum length of an array.

// MFRC522 error codes.
#define     MI_OK                 0         // Everything A-OK.
#define     MI_NOTAGERR           1         // No tag error
#define     MI_ERR                2         // General error

// MFRC522 commands. Described in chapter 10 of the data-sheet.
#define PCD_Idle				 0x00	// no action, cancels current command execution
#define PCD_Mem					 0x01		// stores 25 bytes into the internal buffer
#define PCD_GenerateRandomID	 0x02		// generates a 10-byte random ID number
#define PCD_CalcCRC				 0x03		// activates the CRC co-processor or performs a self-test
#define PCD_Transmit			 0x04		// transmits data from the FIFO buffer
#define PCD_NoCmdChange			 0x07		// no command change, can be used to modify the CommandReg register bits without affecting the command, for example, the PowerDown bit
#define PCD_Receive				 0x08		// activates the receiver circuits
#define PCD_Transceive 			 0x0C		// transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
#define PCD_MFAuthent 			 0x0E		// performs the MIFARE standard authentication as a reader
#define PCD_SoftReset			 0x0F		// resets the MFRC522

// Commands sent to the PICC.
#define     MF1_REQIDL            0x26      // find the antenna area does not enter hibernation
#define     MF1_REQALL            0x52      // find all the tags antenna area
#define     MF1_ANTICOLL          0x93      // anti-collision
#define     MF1_SELECTTAG         0x93      // election tag
#define     MF1_AUTHENT1A         0x60      // authentication key A
#define     MF1_AUTHENT1B         0x61      // authentication key B
#define     MF1_READ              0x30      // Read Block
#define     MF1_WRITE             0xA0      // write block
#define     MF1_DECREMENT         0xC0      // debit
#define     MF1_INCREMENT         0xC1      // recharge
#define     MF1_RESTORE           0xC2      // transfer block data to the buffer
#define     MF1_TRANSFER          0xB0      // save the data in the buffer
#define     MF1_HALT              0x50      // Sleep


//------------------ MFRC522 registers---------------
//Page 0:Command and Status
#define     Reserved00            0x00
#define     CommandReg            0x01
#define     CommIEnReg            0x02
#define     DivIEnReg             0x03
#define     CommIrqReg            0x04
#define     DivIrqReg             0x05
#define     ErrorReg              0x06
#define     Status1Reg            0x07
#define     Status2Reg            0x08
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F
//Page 2:CFG
#define     Reserved20            0x20
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg              0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39
#define     TestDAC2Reg           0x3A
#define     TestADCReg            0x3B
#define     Reserved31            0x3C
#define     Reserved32            0x3D
#define     Reserved33            0x3E
#define     Reserved34            0x3F
//-----------------------------------------------

/* SPI Function*/
void SPI_MasterInit(void);
uint8_t SPI_MasterTransmit(uint8_t cData);

/* RC522 Function*/
void RC522_init();
void RC522_writeToRegister(uint8_t addr, uint8_t val);
uint8_t RC522_readFromRegister(uint8_t addr);
void RC522_setBitMask(uint8_t addr, uint8_t mask);
void RC522_clearBitMask(uint8_t addr, uint8_t mask);
void RC522_begin();
void RC522_reset();
uint8_t RC522_getFirmwareVersion();
int RC522_commandTag(uint8_t command, uint8_t *data, int dlen, uint8_t *result, int *rlen);
int RC522_requestTag(uint8_t mode, uint8_t *type);
int RC522_antiCollision(uint8_t *serial);
void RC522_calculateCRC(uint8_t *data, int len, uint8_t *result);
uint8_t RC522_selectTag(uint8_t *serial);
int RC522_haltTag();
#endif /* RFID_H_ */
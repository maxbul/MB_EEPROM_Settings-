#ifndef EEPROM_Settings_MB_h__
#define EEPROM_Settings_MB_h__
#include <Arduino.h>



//HOW IT WORKS:
//1) create object
//2) create in the main code a  callback function to fill an array by parameters (so the list is customizable)
//2.1) use one or more "add checksum" functions to create the block
//3) to save/restore/check settings we create an array LOCALLY (so it is not in memory when not needed)
//4) fill that array by callback function
//5) process reading/writing/check

//TO DO:
// !!!!! make template
//-!!!! add boolean values(convert to byte)
//-check if an array of items is owerflown
//-create an option "write blank settings"
//-make function to get a total number of blocks
//-restore all valid blocks (when checksum of partial block is OK)

#define MAX_SETTINGS_ARRAY_LENGHT 40


#define SETTINGS_SAVE 1
#define SETTINGS_LOAD 2
#define SETTINGS_CHECK_CHECKSUM 3

#define SETTINGS_ALL_BLOCKS ((int)-1)

#define SETTINGS_ITS_A_CHECKSUM -77
#if defined(__AVR_ATmega168__)\
||defined(__AVR_ATmega168P__)\
||defined(__AVR_ATmega328P__)\
||defined(__AVR_ATmega1280__)\
||defined(__AVR_ATmega2560__)
	#define EEPROM_SETTINGS_ARDUINO
#elif defined(ESP8266)
	#error ESP8266
#elif defined(ESP32)
	#define EEPROM_SETTINGS_ESP32
	#define EEPROM_SIZE 2048
	#include "EEPROM.h"
#else
	#error EEPROM SETTINGS: UNKNOWN MCU TYPE
#endif

#ifndef ARRAYSIZE
	#define ARRAYSIZE(x)  (sizeof(x) / sizeof(x[0]))
#endif

struct settingsItem{
	void *_pointer;	
	int _size;
	char _checksumBlockNo;
};

class MB_EEPROM_Settings{
	public:
		MB_EEPROM_Settings();
		bool setup(bool (* callbackFillSettingsAray)(MB_EEPROM_Settings *parThisObject, size_t parArraySize), unsigned int parBaseEEPROM_Address);
		
		template <typename T>
		bool addItem(const T *parValuePointer) {
				_addItem((void *)parValuePointer, sizeof(T));
				return true;
		}

		//void addItem(int *parValuePointer);
		///void addItem(unsigned int *parValuePointer);
		//void addItem(uint32_t *parValuePointer);
		//void addItem(uint16_t *parValuePointer);
	//	void addItem(unsigned long *parValuePointer);
		//void addItem(byte *parValuePointer);
		//void addItem(float *parValuePointer);
		//void addItem(char * parValuePointer, size_t parSize );
		void addMemoryArea(void * parValuePointer, size_t parSize );
		void addString(char * parValuePointer, size_t parSize );
		
		void addChecksum();
		
		//void saveSettings(int8_t parBlockNum = SETTINGS_ALL_BLOCKS);
		//bool  restoreSettings(int8_t parBlockNum=SETTINGS_ALL_BLOCKS, bool parCheckOnly=false);
		//bool  checkSumValidate(int8_t parBlockNum=SETTINGS_ALL_BLOCKS);
		
		void saveSettings();
		void saveSettings(int8_t parBlockNum);
		
		bool  restoreSettings(int8_t parBlockNum, bool parCheckOnly);
		bool  restoreSettings();
		bool  checkSumValidate(int8_t parBlockNum);
	
	private:
	
		bool (* callbackFillSettingsFunction)(MB_EEPROM_Settings * parThisObject, size_t parArraySize);
		unsigned int _baseEEPROM_Address;
		void _addItem(void *parPointer, int parSize);
		uint8_t _nextItemIndex;
		settingsItem * _itemsArray;
		uint8_t _checksum;
	
	
};

//EEPROM_Settings_MB extern settings;

#endif // EEPROM_Settings_MB_h__
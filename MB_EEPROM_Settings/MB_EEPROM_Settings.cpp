//#include <mbDebug.h>
#include <MB_EEPROM_Settings.h>

MB_EEPROM_Settings::MB_EEPROM_Settings(){
	callbackFillSettingsFunction=NULL;
			
}
//-----------------------------------------
//
//-----------------------------------------
bool MB_EEPROM_Settings::setup(bool (* parCallbackFillSettingsArayFunction)(MB_EEPROM_Settings *parThisObject, size_t parArraySize), unsigned int parBaseEEPROM_Address){
	
	callbackFillSettingsFunction=parCallbackFillSettingsArayFunction;
	_baseEEPROM_Address=parBaseEEPROM_Address;
	_nextItemIndex=0;
	#if defined(EEPROM_SETTINGS_ARDUINO)
		
	#elif defined(EEPROM_SETTINGS_ESP32)
			if (!EEPROM.begin(EEPROM_SIZE))
			{
				return false;
			}
	#endif
	return true;

}


//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::_addItem(void *parValuePointer, int parSize){
		
	if (_nextItemIndex<MAX_SETTINGS_ARRAY_LENGHT){
		_itemsArray[_nextItemIndex]._pointer=parValuePointer;
		_itemsArray[_nextItemIndex]._size=parSize;
		_nextItemIndex++;
	}else{
		//Serial.println(F("SETTINGS OWFL"));
	}//!!!put a warning about owerflow here
	
}
/*
//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::addItem(unsigned int *parValuePointer){
	_addItem((void *)parValuePointer, sizeof(unsigned int));
	
}
*/
/*
//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::addItem(uint32_t *parValuePointer){
	_addItem((void *)parValuePointer, sizeof(uint32_t));
	
}

//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::addItem(int *parValuePointer){
	_addItem((void *)parValuePointer, sizeof(int));
	
}
*/
//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::addMemoryArea(void * parValuePointer, size_t parSize ){
	//_debug("Memory area ptr:", (uint32_t)parValuePointer);
	//_debug("Area size:", parSize);
	//_debugPrintDump(parValuePointer, parSize, &Serial);
	_addItem(parValuePointer, parSize);
}
void MB_EEPROM_Settings::addString(char * parValuePointer, size_t parSize ){
	_addItem((void *)parValuePointer, parSize);
}
/*
//-----------------------------------------
//
//-----------------------------------------
void EEPROM_Settings_MB::addItem(unsigned long *parValuePointer){
	_addItem((void *)parValuePointer, sizeof(unsigned long));
}
*/
/*
//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::addItem(float *parValue){
	_addItem((void *)parValue, sizeof(float));
}

//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::addItem(byte *parValue){
	_addItem((void *)parValue, sizeof(byte));
}	
//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::addItem(void * parValuePointer, size_t parSize ){
	_addItem(parValuePointer, parSize);
}
*/
//-----------------------------------------
//
//-----------------------------------------
void MB_EEPROM_Settings::addChecksum(){
	_addItem((void *)&_checksum, SETTINGS_ITS_A_CHECKSUM );
}

//======================================
//save to EEPROM
//======================================
void MB_EEPROM_Settings::saveSettings(){
	saveSettings(SETTINGS_ALL_BLOCKS);
}
void MB_EEPROM_Settings::saveSettings(int8_t parBlockNum){
	if (callbackFillSettingsFunction==NULL){
		//_message(F("callback is NULL"));
		return;
	}
		settingsItem settingsItemsArray[MAX_SETTINGS_ARRAY_LENGHT];
		_itemsArray=settingsItemsArray;
		_nextItemIndex=0;
		if(!callbackFillSettingsFunction(this, MAX_SETTINGS_ARRAY_LENGHT)){
			//_message(F("callback returns error"));
			return ;
		}
		
		_checksum=0;
		uint8_t curBlock=0;
		byte curByte;
		size_t curSize;
		//_debug("next Idx",_nextItemIndex);
		unsigned int curAddress=_baseEEPROM_Address;
		for (int i = 0; i <_nextItemIndex  ; i++)
		{
		//_debug(F("\n\tsave item idx"), i);
		
				//===scan all bytes
				if (_itemsArray[i]._size==SETTINGS_ITS_A_CHECKSUM ){
					//_message(F("CHECKSUM"));
					curSize= 1;
				}else{
					 curSize= _itemsArray[i]._size;
				}
				//_debug(F("\tsize"),curSize);
				
				for (size_t curByteOffset = 0; curByteOffset < curSize ; curByteOffset++){
					curByte=*(byte *)(_itemsArray[i]._pointer+curByteOffset);
					//_debug("\t\t save byte Offset",curByteOffset );					
					//_debug("\t\tbyte", curByte);
					//_debugPrintChar(curByte);
					//====write all blocks or only one specifyied					
					if ((parBlockNum == SETTINGS_ALL_BLOCKS) || (parBlockNum==curBlock))	{
							#if defined(EEPROM_SETTINGS_ARDUINO)
								while (!eeprom_is_ready())
								;
								cli();
								eeprom_update_byte((uint8_t*)curAddress, curByte);
								sei();
							#elif defined(EEPROM_SETTINGS_ESP32)
								EEPROM.write(curAddress, curByte);
							#else 
								#error EEPROM settings:unknown board
							#endif
					} //\if ((parBlockNum == SETTINGS_ALL_BLOCKS) || (parBlockNum==curBlock))....
					//_debug("\tsaved to EEPROM addr", curAddress);
					if (_itemsArray[i]._size ==SETTINGS_ITS_A_CHECKSUM ){
						//this byte is a checksum, prepare for next block
						//_debug("\t\t final saved checksum",_checksum);		
						_checksum=0;
						curBlock++;	
					}else{
						++_checksum+= curByte;
						//_debug("\t\t tmp calc checksum",_checksum);		
					}
					curAddress++;
				}//\for (size_t curByteOffset = 0; curByteOffset < curSize ; curByteOffset++)...
		}
		#if defined(EEPROM_SETTINGS_ESP32)||defined(EEPROM_SETTINGS_ESP8266)
			EEPROM.commit();
		#endif
}

//======================================
//save to EEPROM
//======================================
bool MB_EEPROM_Settings::restoreSettings(){
	//all blocks, not only checksum validation (really restore)
	return restoreSettings(SETTINGS_ALL_BLOCKS, false);
}

bool MB_EEPROM_Settings::restoreSettings(int8_t parBlockNum, bool parCheckOnly){
	if (callbackFillSettingsFunction==NULL){
		//Serial.println(F("Callback is null"));
		return false;
	}
	settingsItem settingsItemsArray[MAX_SETTINGS_ARRAY_LENGHT];
	_itemsArray=settingsItemsArray;
	_nextItemIndex=0;

	if(!callbackFillSettingsFunction(this, MAX_SETTINGS_ARRAY_LENGHT)){
		//_message(F("restore callback error"));
		return false;
	};
	//_debug("total items",_nextItemIndex);
	bool resultOK=true;
	_checksum=0;
	uint8_t curBlock=0;
	byte curByte;
	unsigned int curAddress=_baseEEPROM_Address;
	for (int i = 0; i <_nextItemIndex  ; i++){
		//===scan all bytes
		int curSize;
		if (_itemsArray[i]._size ==SETTINGS_ITS_A_CHECKSUM)
		{
			curSize=1;
		} 
		else
		{
			curSize=_itemsArray[i]._size;
		}
		//_debug("\n\trest.index",i);
		//_debug("\trest.size",curSize);
		for (int curByteOffset = 0; curByteOffset < curSize ; curByteOffset++){
			//get  all blocks or only one specifyied
			if ((parBlockNum == SETTINGS_ALL_BLOCKS) || (parBlockNum==curBlock))	{
//Serial.println("\t!\t");							
				#if defined(EEPROM_SETTINGS_ARDUINO)
					while (!eeprom_is_ready())
					;
					cli();
					curByte=eeprom_read_byte((uint8_t*)curAddress);
					sei();
				#elif defined(EEPROM_SETTINGS_ESP32)
					curByte=EEPROM.read(curAddress);
				#else
					#error EEPROM settings:unknown board
				#endif
				//_debug("cur EEPROM addr", curAddress);
				//_debug("cur byte", curByte);
				//_debugPrintChar(curByte);
				//
				if (_itemsArray[i]._size ==SETTINGS_ITS_A_CHECKSUM ){
					//_message(F("\nCHECKSUM!"));
					//this byte is a CHECKSUM, check it  and prepare for the next block
 					if (_checksum!= curByte){
 						//_message(F("\nwrong checksum"));
 						//_debug("calculated", _checksum);
						 //_debug("readed", curByte);
 						resultOK=false;
 //						return resultOK;
 					}else{
						//_message(F("chksum OK"));												
 					}
					_checksum=0;
					curBlock++;
				}else{
					++_checksum+= curByte;
					//_debug("curByteOffset",curByteOffset);
					//_debug("\ntmp checksum", _checksum)
					if (!parCheckOnly){
						memset(_itemsArray[i]._pointer+curByteOffset, curByte,1);
					}
				}
			} //\if ((parBlockNum == SETTINGS_ALL_BLOCKS) || (parBlockNum==curBlock))	
			curAddress++;
		}  //\for (int curByteOffset = 0; curByteOffset < curSize ; curByteOffset++)
	}  //\for (int i = 0; i <_nextItemIndex  ; i++)
//Serial.println(resultOK);
	return resultOK;
}
//=========
//validate checksum
bool  MB_EEPROM_Settings::checkSumValidate(int8_t parBlockNum){
	return restoreSettings(parBlockNum, true);
}


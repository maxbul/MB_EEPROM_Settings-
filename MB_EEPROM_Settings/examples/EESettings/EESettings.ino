#include <MBCommandParser_v3.h>
#include <MB_EEPROM_Settings.h>

#define EEPROM_SETTINGS_START_ADDRESS 10

int testInt1=2;
int testInt2=765;
uint8_t testUint8= 123;
float testFloat=93.710;
char testString[20]="Test string";

MB_EEPROM_Settings settings;

SerialCommandParser commandParser(&Serial);
//======================================
//
//======================================
void setup(){
	Serial.begin(115200);
	
	pinMode(13, OUTPUT);
	
	commandParser.addCommand('h', F("show help (this screen)"), showHelp, 0);
	commandParser.addCommand('m', F("modify settings"), modifyValues, 0);
	commandParser.addCommand('p', F("print values"), printValues, 0);
	
	commandParser.addCommand('s', F("save settings"), saveSettings, 0);
	commandParser.addCommand('r', F("restore settings"), restoreSettings, 0);
	commandParser.addCommand('c', F("checksum"), checkSums, 0);
	

	digitalWrite(13, 1);
	commandParser.PrintCommandList();
	settings.setup(&calbackFillSettings, EEPROM_SETTINGS_START_ADDRESS);
}


//======================================
//
//======================================
void loop(){
	commandParser.processSerialReading();
	commandParser.parseSerialCommand();
}

//-----------------------------------------
//parBlockNo: -1 for all
//-----------------------------------------
boolean calbackFillSettings(EEPROM_Settings_MB * parSettingsInstance, size_t parArraySize){
	parSettingsInstance->addItem(&testInt1);
//===some object
	//settings.addItem((void *)&object._somedata, sizeof(object._somedata));

	parSettingsInstance->addItem(&testInt2);
	parSettingsInstance->addChecksum();
	parSettingsInstance->addItem(&testUint8);
	parSettingsInstance->addItem(&testFloat);
	parSettingsInstance->addItem((char *)testString, 20);
	parSettingsInstance->addChecksum();
}

void checkSums(){
	settings.restoreSettings(SETTINGS_ALL_BLOCKS);
}
//-----------------------------------------
//
//-----------------------------------------
void saveSettings(){
	settings.saveSettings(SETTINGS_ALL_BLOCKS);
}

//-----------------------------------------
//
//-----------------------------------------
void restoreSettings(){
	settings.restoreSettings(SETTINGS_ALL_BLOCKS);
}

//-----------------------------------------
//
//-----------------------------------------
void modifyValues(){
	testInt1 +=1;
	testInt2 =800;
	testUint8-= 2;
	testFloat+=1.1;
	testString[5]='!';	
	testString[7]='*';	
}

//-----------------------------------------
//
//-----------------------------------------
void printValues(){
	Serial.print("testInt1=");
	Serial.print(testInt1);
	Serial.print("\t testInt2=");
	Serial.print(testInt2);
	Serial.print("\t testUint8=");
	Serial.print(testUint8);
	Serial.print("\t teststr=");
	Serial.print(testString);
	Serial.print("\t testFloat=");
	Serial.print(testFloat);
	
}

//-----------------------------------------
//
//-----------------------------------------
void showHelp(){
	commandParser.PrintCommandList();
}

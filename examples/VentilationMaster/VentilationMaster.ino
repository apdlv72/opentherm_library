/*
OpenTherm example to simulate a Home Ventilation System master (like Vitovent 300 by Viessmann)
Based on the OpenTherm_Demo example by Ihor Melnyk

NOTE:

This sketch compiles, however is competely UNTESTED (!!)
since I did not yet receive the hardwar e to test with.
Use at your own risk!

By: Artur Pogoda de la Vega
Date: November, 2019

Uses the OpenTherm library to send those commands to the ventilation unit that would
normally be sent by the 2-wire remote controle.
Open serial monitor at 115200 baud to see output.

Hardware Connections (OpenTherm Adapter (http://ihormelnyk.com/pages/OpenTherm) to Arduino/ESP8266):
-OT1/OT2 = Boiler X1/X2
-VCC = 5V or 3.3V
-GND = GND
-IN  = Arduino (3) / ESP8266 (5) Output Pin
-OUT = Arduino (2) / ESP8266 (4) Input Pin

Controller(Arduino/ESP8266) input pin should support interrupts.
Arduino digital pins usable for interrupts: Uno, Nano, Mini: 2,3; Mega: 2, 3, 18, 19, 20, 21
ESP8266: Interrupts may be attached to any GPIO pin except GPIO16,
but since GPIO6-GPIO11 are typically used to interface with the flash memory ICs on most esp8266 modules, applying interrupts to these pins are likely to cause problems
*/


#include <Arduino.h>
#include <OpenTherm.h>

const int inPin = 2; //4
const int outPin = 3; //5
OpenTherm ot(inPin, outPin);

void handleInterrupt() {
	ot.handleInterrupt();
}

void setup()
{
	Serial.begin(115200);
	Serial.println("Start");
	
	ot.begin(handleInterrupt);
}

static unsigned int masterProductVersionHi = 18;
static unsigned int masterProductVersionLo =  2;

static unsigned int masterConfigurationHi  =  0;
static unsigned int masterConfigurationLo  = 18;

static VentilationLevel ventilationLevel = VL_NORMAL;

static unsigned int configurationMemberId = 0;
static unsigned int slaveProductVersionHi = 0;
static unsigned int slaveProductVersionLo = 0;
static unsigned int relativeVentilation   = 0;
static unsigned int ventilationStatus    = 0;
static unsigned int tspIndex = 0;

static long supplyInletTemp  = -300;
static long exhaustInletTemp = -300;

static short int tsps[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

float toTemperature(unsigned long response) {
    int hi = response & 0xff00;
    unsigned lo = response & 0x00ff;

    // TODO: Negative values probably wrong since I have no examples (my own supply inlet goes never below 0C)
    float result = 0.01*(100*lo/255) + hi;
    return result;
}

void loop()
{	
    unsigned long response;
    OpenThermResponseStatus responseStatus;

    ot.getVentilationSlaveProductVersion();

    response = ot.setVentilationMasterProductVersion(masterProductVersionHi, masterProductVersionLo);
    if (ot.getLastResponseStatus() == OpenThermResponseStatus::SUCCESS) {
        slaveProductVersionHi = (response & 0xffff)>8;
        slaveProductVersionLo = (response & 0x00ff);
    }

    response = ot.getVentilationTSPSetting(tspIndex);
    if (ot.getLastResponseStatus() == OpenThermResponseStatus::SUCCESS) {
        tsps[tspIndex] = response & 0xff;
    }

    tspIndex++;
    if (tspIndex>63) {
        tspIndex = 0;
    }

    ot.setVentilationMasterConfiguration(masterConfigurationHi, masterConfigurationLo);

    response = ot.getVentilationStatus();
    if (ot.getLastResponseStatus() == OpenThermResponseStatus::SUCCESS) {
        ventilationStatus = response & 0xffff;
        Serial.println("Ventilation status:         " + String(ventilationStatus)   + " degrees C");
        if (ot.isFilterCheck(ventilationStatus)) {
            Serial.println("*** CHECK FILTER ***");
        }
    }

    response = ot.setVentilationControlSetpoint(ventilationLevel);

    response = ot.getVentilationConfigurationMemberId();
    if (ot.getLastResponseStatus() == OpenThermResponseStatus::SUCCESS) {
        configurationMemberId = response & 0xff;
    }

    response = ot.getVentilationRelativeVentilation();
    if (ot.getLastResponseStatus() == OpenThermResponseStatus::SUCCESS) {
        relativeVentilation = response & 0xff;
        Serial.println("Relative ventilation level: " + String(relativeVentilation) + " degrees C");
    }

    response = ot.getSupplyInletTemperature();
    if (ot.getLastResponseStatus() == OpenThermResponseStatus::SUCCESS) {
        supplyInletTemp = toTemperature(response);
        Serial.println("Supply  inlet  temperature: " + String(supplyInletTemp)     + " degrees C");
    }

    response = ot.getExhaustInletTemperature();
    if (ot.getLastResponseStatus() == OpenThermResponseStatus::SUCCESS) {
        exhaustInletTemp = toTemperature(response);
        Serial.println("Exhaust inlet  temperature: " + String(exhaustInletTemp)    + " degrees C");
    }

    // probably unsupported (at least by Vitovent300?)
    /*
    float supplyOutletTemp  = ot.getSupplyInletTemperature();
    float exhaustOutletTemp = ot.getExhaustInletTemperature();
    Serial.println("Supply  outlet temperature: " + String(supplyOutletTemp)  + " degrees C");
    Serial.println("Exhaust outlet temperature: " + String(exhaustOutletTemp) + " degrees C");
    */

/*  log from a vitovent 300:

    T:OTMessage[WRITE_DATA,id:126,hi:18,lo:2,Master product version]:0
    T:OTMessage[READ_DATA,id:127,hi:0,lo:0,Slave product version]:0
    T:OTMessage[READ_DATA,id:89,hi:18,lo:0,TSP setting V/H]:0
    T:OTMessage[WRITE_DATA,id:2,hi:0,lo:18,Master configuration]:0
    T:OTMessage[READ_DATA,id:70,hi:1,lo:0,Status V/H]:0
    T:OTMessage[WRITE_DATA,id:71,hi:0,lo:2,Control setpoint V/H]:0
    T:OTMessage[READ_DATA,id:74,hi:0,lo:0,Configuration/memberid V/H]:0
    T:OTMessage[READ_DATA,id:77,hi:0,lo:0,Relative ventilation]:0
    T:OTMessage[READ_DATA,id:80,hi:0,lo:0,Supply inlet temperature]:0
    T:OTMessage[READ_DATA,id:82,hi:0,lo:0,Exhaust inlet temperature]:0

    T:OTMessage[WRITE_DATA,id:126,hi:18,lo:2,Master product version]:0
    T:OTMessage[READ_DATA,id:127,hi:0,lo:0,Slave product version]:0
    T:OTMessage[READ_DATA,id:89,hi:19,lo:0,TSP setting V/H]:0
    T:OTMessage[WRITE_DATA,id:2,hi:0,lo:18,Master configuration]:0
    T:OTMessage[READ_DATA,id:70,hi:1,lo:0,Status V/H]:0
    T:OTMessage[WRITE_DATA,id:71,hi:0,lo:2,Control setpoint V/H]:0
    T:OTMessage[READ_DATA,id:74,hi:0,lo:0,Configuration/memberid V/H]:0
    T:OTMessage[READ_DATA,id:77,hi:0,lo:0,Relative ventilation]:0
    T:OTMessage[READ_DATA,id:80,hi:0,lo:0,Supply inlet temperature]:0
    T:OTMessage[READ_DATA,id:82,hi:0,lo:0,Exhaust inlet temperature]:0

    T:OTMessage[WRITE_DATA,id:126,hi:18,lo:2,Master product version]:0
    T:OTMessage[READ_DATA,id:127,hi:0,lo:0,Slave product version]:0
    T:OTMessage[READ_DATA,id:89,hi:20,lo:0,TSP setting V/H]:0
    T:OTMessage[WRITE_DATA,id:2,hi:0,lo:18,Master configuration]:0
    T:OTMessage[READ_DATA,id:70,hi:1,lo:0,Status V/H]:0
    T:OTMessage[WRITE_DATA,id:71,hi:0,lo:2,Control setpoint V/H]:0
    T:OTMessage[READ_DATA,id:74,hi:0,lo:0,Configuration/memberid V/H]:0
    T:OTMessage[READ_DATA,id:77,hi:0,lo:0,Relative ventilation]:0
    T:OTMessage[READ_DATA,id:80,hi:0,lo:0,Supply inlet temperature]:0
    T:OTMessage[READ_DATA,id:82,hi:0,lo:0,Exhaust inlet temperature]:0

    T:OTMessage[WRITE_DATA,id:126,hi:18,lo:2,Master product version]:0
    T:OTMessage[READ_DATA,id:127,hi:0,lo:0,Slave product version]:0
    T:OTMessage[READ_DATA,id:89,hi:23,lo:0,TSP setting V/H]:0
    T:OTMessage[WRITE_DATA,id:2,hi:0,lo:18,Master configuration]:0
    T:OTMessage[READ_DATA,id:70,hi:1,lo:0,Status V/H]:0
    T:OTMessage[WRITE_DATA,id:71,hi:0,lo:2,Control setpoint V/H]:0
    T:OTMessage[READ_DATA,id:74,hi:0,lo:0,Configuration/memberid V/H]:0
    T:OTMessage[READ_DATA,id:77,hi:0,lo:0,Relative ventilation]:0
    T:OTMessage[READ_DATA,id:80,hi:0,lo:0,Supply inlet temperature]:0
    T:OTMessage[READ_DATA,id:82,hi:0,lo:0,Exhaust inlet temperature]:0

    T:OTMessage[WRITE_DATA,id:126,hi:18,lo:2,Master product version]:0
    T:OTMessage[READ_DATA,id:127,hi:0,lo:0,Slave product version]:0
    T:OTMessage[READ_DATA,id:89,hi:24,lo:0,TSP setting V/H]:0
    T:OTMessage[WRITE_DATA,id:2,hi:0,lo:18,Master configuration]:0
    T:OTMessage[READ_DATA,id:70,hi:1,lo:0,Status V/H]:0
    T:OTMessage[WRITE_DATA,id:71,hi:0,lo:2,Control setpoint V/H]:0
    T:OTMessage[READ_DATA,id:74,hi:0,lo:0,Configuration/memberid V/H]:0
    T:OTMessage[READ_DATA,id:77,hi:0,lo:0,Relative ventilation]:0
    T:OTMessage[READ_DATA,id:80,hi:0,lo:0,Supply inlet temperature]:0
    T:OTMessage[READ_DATA,id:82,hi:0,lo:0,Exhaust inlet temperature]:0

    T:OTMessage[WRITE_DATA,id:126,hi:18,lo:2,Master product version]:0
    T:OTMessage[READ_DATA,id:127,hi:0,lo:0,Slave product version]:0
    T:OTMessage[READ_DATA,id:89,hi:48,lo:0,TSP setting V/H]:0
    T:OTMessage[WRITE_DATA,id:2,hi:0,lo:18,Master configuration]:0
*/

	Serial.println();
	delay(1000);
}

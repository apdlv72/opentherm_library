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

// transparent client parameters.
// 0: relative motor speed for VentilationLevel VL_REDUCED
// 2: relative motor speed for VentilationLevel VL_NORMAL
// 4: relative motor speed for VentilationLevel VL_HIGH
// TODO: Find out what all other TSPs stand for.
static short int tsps[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

float toTemperature(unsigned long response) {
    int hi = response & 0xff00;
    unsigned lo = response & 0x00ff;

    // TODO: Negative values probably wrong since I have no examples (my own supply inlet goes never below 0 degrees Celsius)
    float result = 0.01*(100*lo/255) + hi;
    return result;
}

static long unsigned loop_counter = 0;
static long unsigned last_command_sent = 0;

void loop()
{	
    int step = loop_counter++;

    unsigned long response;
    OpenThermResponseStatus responseStatus;

    last_command_sent = millis();

    switch (step) {

    case 0:
        Serial.println("-> getVentilationSlaveProductVersion()");
        response = ot.getVentilationSlaveProductVersion();
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            slaveProductVersionHi = (response & 0xffff)>8;
            slaveProductVersionLo = (response & 0x00ff);
            Serial.println("Slave product version:      " + String(slaveProductVersionHi)  + "/" +  String(slaveProductVersionLo));
        }
        break;

    case 1:
        Serial.println(String("-> setVentilationMasterProductVersion(") + masterProductVersionHi + "," + masterProductVersionLo + ")");
        response = ot.setVentilationMasterProductVersion(masterProductVersionHi, masterProductVersionLo);
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            // TODO: Response ignored .. what should we do with the unit's answer?
        }
        break;

    case 2:
        Serial.println(String("-> getVentilationTSPSetting(") + tspIndex + ")");
        response = ot.getVentilationTSPSetting(tspIndex);
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            tsps[tspIndex] = response & 0xff;
        }
        // Vitovent requests TSP values for indeces 0,...,63 in a row an and starts over at index 0
        tspIndex++;
        if (tspIndex>63) {
            tspIndex = 0;
        }
        break;

    case 3:
        Serial.println(String("-> setVentilationMasterConfiguration(") + masterConfigurationHi + "," + masterConfigurationLo + ")");
        response = ot.setVentilationMasterConfiguration(masterConfigurationHi, masterConfigurationLo);
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            // TODO: Response ignored .. what should we do with the unit's answer?
        }
        break;

    case 4:
        Serial.println("-> getVentilationStatus()");
        response = ot.getVentilationStatus();
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            ventilationStatus = response & 0xffff;
            Serial.println("Ventilation status:         " + String(ventilationStatus)   + " degrees C");
            if (ot.isFilterCheck(ventilationStatus)) {
                Serial.println("*** CHECK FILTER ***");
            }
        }
        break;

    case 5:
        Serial.println(String("-> setVentilationControlSetpoint(") + ventilationLevel + ")");
        response = ot.setVentilationControlSetpoint(ventilationLevel);
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            // TODO: Response ignored .. what should we do with the unit's answer? Check if accepted? Just print/dump?
        }
        break;

    case 6:
        Serial.println("-> getVentilationConfigurationMemberId()");
        response = ot.getVentilationConfigurationMemberId();
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            configurationMemberId = response & 0xff;
            // TODO: Split config member ID into hi/lo?
            Serial.println("Configuration member ID:    " + String(configurationMemberId));
        }
        break;

    case 7:
        Serial.println("-> getVentilationRelativeVentilation()");
        response = ot.getVentilationRelativeVentilation();
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            relativeVentilation = response & 0xff;
            Serial.println("Relative ventilation level: " + String(relativeVentilation) + " degrees C");
        }
        break;

    case 8:
        Serial.println("-> getSupplyInletTemperature()");
        response = ot.getSupplyInletTemperature();
        if (ot.getLastResponseStatus() == OpenThermResponseStatus::SUCCESS) {
            supplyInletTemp = toTemperature(response);
            Serial.println("Supply  inlet  temperature: " + String(supplyInletTemp)     + " degrees C");
        }
        break;

    case 9:
        Serial.println("-> getExhaustInletTemperature()");
        response = ot.getExhaustInletTemperature();
        if ((responseStatus = ot.getLastResponseStatus()) == OpenThermResponseStatus::SUCCESS) {
            exhaustInletTemp = toTemperature(response);
            Serial.println("Exhaust inlet  temperature: " + String(exhaustInletTemp)    + " degrees C");
        }
        break;

    /*
     * Probably unsupported (by my Vitovent300?) or at least never sent.
     * But maybe with those devices that support a summer bypass?
    case 10:
        response = ot.getSupplyInletTemperature();
        break;

    case 11:
        response = ot.getExhaustInletTemperature();
        break:
    */

    default:
        // start over with step 0
        loop_counter = 0;
        return;
    }

    Serial.println(String("<- response=") + response + ", status=" + responseStatus);

    unsigned long time_to_wait = 100; // ms
    unsigned long now = millis();
    if (now>last_command_sent) { // overflow possible on system time after 42 days
        unsigned long execution_time = now-last_command_sent;
        if (execution_time<900) {
            time_to_wait = 900-execution_time;
        }
    }

    delay(time_to_wait);

}

/*  Log from a Vitovent 300:

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


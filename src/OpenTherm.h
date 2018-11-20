/*
OpenTherm.h - OpenTherm Library for the ESP8266/Arduino platform
https://github.com/ihormelnyk/OpenTherm
http://ihormelnyk.com/pages/OpenTherm
Licensed under MIT license
Copyright 2018, Ihor Melnyk

Frame Structure:
P MGS-TYPE SPARE DATA-ID  DATA-VALUE
0 000      0000  00000000 00000000 00000000
*/

#ifndef OpenTherm_h
#define OpenTherm_h

#include <Arduino.h>

enum OpenThermResponseStatus {
	NONE,
	SUCCESS,
	INVALID,
	TIMEOUT
};

enum OpenThermRequestType {
	READ,
	WRITE
};

enum OpenThermMessageID {
	Status, // flag8 / flag8  Master and Slave Status flags. 
	TSet, // f8.8  Control setpoint  ie CH  water temperature setpoint (°C)
	MConfigMMemberIDcode, // flag8 / u8  Master Configuration Flags /  Master MemberID Code 
	SConfigSMemberIDcode, // flag8 / u8  Slave Configuration Flags /  Slave MemberID Code 
	Command, // u8 / u8  Remote Command 
	ASFflags, // / OEM-fault-code  flag8 / u8  Application-specific fault flags and OEM fault code 
	RBPflags, // flag8 / flag8  Remote boiler parameter transfer-enable & read/write flags 
	CoolingControl, // f8.8  Cooling control signal (%) 
	TsetCH2, // f8.8  Control setpoint for 2e CH circuit (°C)
	TrOverride, // f8.8  Remote override room setpoint 
	TSP, // u8 / u8  Number of Transparent-Slave-Parameters supported by slave 
	TSPindexTSPvalue, // u8 / u8  Index number / Value of referred-to transparent slave parameter. 
	FHBsize, // u8 / u8  Size of Fault-History-Buffer supported by slave 
	FHBindexFHBvalue, // u8 / u8  Index number / Value of referred-to fault-history buffer entry. 
	MaxRelModLevelSetting, // f8.8  Maximum relative modulation level setting (%) 
	MaxCapacityMinModLevel, // u8 / u8  Maximum boiler capacity (kW) / Minimum boiler modulation level(%) 
	TrSet, // f8.8  Room Setpoint (°C)
	RelModLevel, // f8.8  Relative Modulation Level (%) 
	CHPressure, // f8.8  Water pressure in CH circuit  (bar) 
	DHWFlowRate, // f8.8  Water flow rate in DHW circuit. (litres/minute) 
	DayTime, // special / u8  Day of Week and Time of Day 
	Date, // u8 / u8  Calendar date 
	Year, // u16  Calendar year 
	TrSetCH2, // f8.8  Room Setpoint for 2nd CH circuit (°C)
	Tr, // f8.8  Room temperature (°C)
	Tboiler, // f8.8  Boiler flow water temperature (°C)
	Tdhw, // f8.8  DHW temperature (°C)
	Toutside, // f8.8  Outside temperature (°C)
	Tret, // f8.8  Return water temperature (°C)
	Tstorage, // f8.8  Solar storage temperature (°C)
	Tcollector, // f8.8  Solar collector temperature (°C)
	TflowCH2, // f8.8  Flow water temperature CH2 circuit (°C)
	Tdhw2, // f8.8  Domestic hot water temperature 2 (°C)
	Texhaust, // s16  Boiler exhaust temperature (°C)
	TdhwSetUBTdhwSetLB = 48, // s8 / s8  DHW setpoint upper & lower bounds for adjustment  (°C)
	MaxTSetUBMaxTSetLB, // s8 / s8  Max CH water setpoint upper & lower bounds for adjustment  (°C)
	HcratioUBHcratioLB, // s8 / s8  OTC heat curve ratio upper & lower bounds for adjustment  
	TdhwSet = 56, // f8.8  DHW setpoint (°C)    (Remote parameter 1)
	MaxTSet, // f8.8  Max CH water setpoint (°C)  (Remote parameters 2)
	Hcratio, // f8.8  OTC heat curve ratio (°C)  (Remote parameter 3)
	// message IDs for home ventilation system (Vitovent 300)
	StatusVH = 70, // Status V/H
	ControlSetpointVH, // Control setpoint V/H
	FaultFlagsVH, // Fault flags/code V/H (otgw matrix only. not (yet) seen on own Vitovent 300)
	DiagnosticCodeVH, // VHDiagnosticCode
	ConfigurationMemberidVH,  // Configuration memberid V/H
	RelativeVentilationVH = 77, // Relative ventilation
	RelativeHumidityVH, // RelativeHumidity
	CO2LevelVH, // CO2Level
	TsupplyInletVH, // Supply inlet temperature"
	TsupplyOutletVH, // SupplyOutletTemperature
	TexhaustInletVH, // Exhaust inlet temperature
	TexhaustOutletVH, // Exhaust outlet temperature
	ExhaustFanSpeedVH = 84,
	InletFanSpeedVH,
	VHRemoteParameterVH ,
	NominalVentilationVH,
	TSPSizeVH, // (hb): VHFHBIndex; (lb): VHFHBValue
	TspSettingsVH, // (hb): VHTSPIndex; (lb): VHTSPValue, transparent slave parameter setting V/H
	FHBSizeVH, // (hb): VHFHBSize
	FHBIndexVH, // (lb): VHFHBValue //(hb): VHFHBIndex; (lb): VHFHBValue
	RemoteOverrideFunction = 100, // flag8 / -  Function of manual and program changes in master and remote room setpoint. 
	OEMDiagnosticCode = 115, // u16  OEM-specific diagnostic/service code 
	BurnerStarts, // u16  Number of starts burner 
	CHPumpStarts, // u16  Number of starts CH pump 
	DHWPumpValveStarts, // u16  Number of starts DHW pump/valve 
	DHWBurnerStarts, // u16  Number of starts burner during DHW mode 
	BurnerOperationHours, // u16  Number of hours that burner is in operation (i.e. flame on) 
	CHPumpOperationHours, // u16  Number of hours that CH pump has been running 
	DHWPumpValveOperationHours, // u16  Number of hours that DHW pump has been running or DHW valve has been opened 
	DHWBurnerOperationHours, // u16  Number of hours that burner is in operation during DHW mode 
	OpenThermVersionMaster, // f8.8  The implemented version of the OpenTherm Protocol Specification in the master. 
	OpenThermVersionSlave, // f8.8  The implemented version of the OpenTherm Protocol Specification in the slave. 
	MasterVersion, // u8 / u8  Master product version number and type 
	SlaveVersion, // u8 / u8  Slave product version number and type
};

enum OpenThermStatus {
	NOT_INITIALIZED,
	READY,
	DELAY,
	REQUEST_SENDING,
	RESPONSE_WAITING,
	RESPONSE_START_BIT,
	RESPONSE_RECEIVING,
	RESPONSE_READY,
	RESPONSE_INVALID	
};

enum VentilationLevel {
    VL_OFF     = 0,
    VL_REDUCED = 1,
    VL_NORMAL  = 2,
    VL_HIGH    = 3
};

class OpenTherm
{
private:
	const int inPin;
	const int outPin;	

	volatile OpenThermStatus status;
	volatile unsigned long response;
	volatile OpenThermResponseStatus responseStatus;
	volatile unsigned long responseTimestamp;
	volatile byte responseBitIndex;
	
	int readState();
	void setActiveState();
	void setIdleState();	
	void activateBoiler();

	void sendBit(bool high);
	bool parity(unsigned long frame);
	bool isValidResponse(unsigned long response);
	void(*handleInterruptCallback)();
	void(*processResponseCallback)(unsigned long, OpenThermResponseStatus);
public:	
	OpenTherm(int inPin = 4, int outPin = 5);
	void begin(void(*handleInterruptCallback)(void));
	void begin(void(*handleInterruptCallback)(void), void(*processResponseCallback)(unsigned long, OpenThermResponseStatus));
	bool isReady();
	unsigned long sendRequest(unsigned long request);
	bool sendRequestAync(unsigned long request);
	unsigned long buildRequest(OpenThermRequestType type, OpenThermMessageID id, unsigned int data);
	OpenThermResponseStatus getLastResponseStatus();
	void handleInterrupt();	
	void process();
	void end();

	//building requests
	unsigned long buildSetBoilerStatusRequest(bool enableCentralHeating, bool enableHotWater = false, bool enableCooling = false, bool enableOutsideTemperatureCompensation = false, bool enableCentralHeating2 = false);
	unsigned long buildSetBoilerTemperatureRequest(float temperature);
	unsigned long buildGetBoilerTemperatureRequest();

	//parsing responses
	bool isFault(unsigned long response);
	bool isCentralHeatingEnabled(unsigned long response);
	bool isHotWaterEnabled(unsigned long response);
	bool isFlameOn(unsigned long response);
	bool isCoolingEnabled(unsigned long response);
	bool isDiagnostic(unsigned long response);	
	float getTemperature(unsigned long response);
	unsigned int temperatureToData(float temperature);

	//basic requests
	unsigned long setBoilerStatus(bool enableCentralHeating, bool enableHotWater = false, bool enableCooling = false, bool enableOutsideTemperatureCompensation = false, bool enableCentralHeating2 = false);	
	bool setBoilerTemperature(float temperature);
	float getBoilerTemperature();

	//building requests for home ventilation system
	unsigned long buildSetVentilationMasterProductVersion(unsigned int hi, unsigned int lo);
	unsigned long buildGetVentilationTSPSetting(unsigned int index);
	unsigned long buildSetVentilationMasterConfiguration(unsigned int hi, unsigned int lo);
	unsigned long buildSetVentilationControlSetpoint(unsigned int level);

	//basic requests for home ventilation system
	unsigned long setVentilationMasterProductVersion(unsigned int hi, unsigned int lo);
	unsigned long getVentilationSlaveProductVersion();
	unsigned long getVentilationTSPSetting(unsigned int index);
	unsigned long setVentilationMasterConfiguration(unsigned int hi, unsigned int lo);

	unsigned long getVentilationStatus();
	bool isFilterCheck(unsigned int ventilationStatus);

	unsigned long setVentilationControlSetpoint(VentilationLevel level);
	unsigned long getVentilationConfigurationMemberId();
	unsigned long getVentilationRelativeVentilation();

	unsigned long getSupplyInletTemperature();
	unsigned long getExhaustInletTemperature();

	unsigned long getSupplyOutletTemperature();
	unsigned long getExhaustOutletTemperature();
};

#endif // OpenTherm_h

/*Greg's Pool Controller
This controller is for my swimming pool.

Operational Functions:
Solar pool heating - this measures the roof and pool water temperature and if its swimming season and its hot enough then it will switch on the pumps and heat the pool
The controller can also monitor the pH level and run a perilstatic pump to add HCL to the pool.
It has a level of intelegance on when to operate the pumps and chlorinator so as to save power. During summer months the filter needs to operate for approx 8hrs a day
but in winter, just 4hrs is enough. So during summer, if the pool runs for 2hrs to heat the pool during the day, then it will wait until off peak power time begins, and run the remaining 6hrs then.
During winter, it will only run the pumps during off peak hours.
There is a button on the controller to allow the pump to be stopped/started if required - to backwash for instance.

This version talks MQTT allowing it to be controlled from Openhab. It reports back the status, temperatures,pH level, pump runtimes as well as being able to be controlled via MQTT.
Control functions are pump start/stop ( like the button allows), set the desired water temperature, how long in minutes the acid feeder should run each hour(as needed to keep pH in check)
Whilst the arduino has an accurate DS1307 RTC, it will also pull the time from Openhab - this function was initally only so as to adjust for when daylight savings started/stopped...but i sync it more frequently now.
There is a 20x4 LCD display to show Time, Status, Roof/Water temps, pH as well as runtimes for the filterpump and acid pump.
For communications to MQTT, im now using the excellent MySensors system ( mysensors.org ), i was using an ethernet arduino and an old wifi router in bridge mode, but the bridge was unreliable.
MySensors has been rock solid on this for 18months or so. There are references to Vera - where i was using mySensors to interface it to Vera, but no more...now MQTT and Openhab.

Hardware:
Arduino Nano running on Optiboot - at some point i was reallllly tight for flash!
This sits on a Nano IO shield - makes radio and i/o connections really easy.
There is a RTC and LCD on the I2C bus.
Manual mode button
Its got a piezo beeper to alert when its in manual mode - so i dont forget.
Its got 5 SSR relays - filter pump, chlorinatior, solar pump, acid pump and one for the pool lights.

This is about the 5th version/rewrite of the code. Its the first project i started on about 2009...and actually what introduced me to the wonderful world of Arduino.
Its a mess, ive borrowed code from all edges of the internet (thanks everyone!), its likely full of bugs and highly unoptimised....but it works well enough ;-)


*/

/*

Todo:
- Interupt function for the pH tuning
- Different Beep Types - Error - frequent beeping ( lost temp sensor)
- ph calibration values - set via controller/service port to get voltages etc.


*/



#define TESTBED 1 //Set to 1 for benchtest hardware - different id, parameters and Temp sensor addresses.
#define PHCAL 0 // for calibration routine - not currently working.


#if TESTBED == 1
#define MY_DEBUG // Enable debug prints to serial monitor for mySensors library.
#define MY_NODE_ID 4 //Set the MySensors NODE id... static cause i use Openhab/MQTT.
#define RADIO_CH 96  // Sets MYSensors to use Radio Channel 96
#endif

#if TESTBED == 0
#define MY_DEBUG // Enable debug prints to serial monitor for mySensors library.
#define MY_NODE_ID 6 //Set the MySensors NODE id... static cause i use Openhab/MQTT.
#define RADIO_CH 96  // Sets MYSensors to use Radio Channel 96
#endif


//Set the error/tx/rx pins to 20 - i dont need them.
#define MY_DEFAULT_TX_LED_PIN 20
#define MY_DEFAULT_RX_LED_PIN 20
#define MY_DEFAULT_ERR_LED_PIN 20


// Enable and select radio type attached
#define MY_RADIO_NRF24


#include <SPI.h>
#include <MySensor.h>  
#include <DallasTemperature.h>
#include <OneWire.h>
#include <DS1307RTC.h> //for RTC
#include <bv4208.h> //for LCD
#include <I2c_bv.h> //for LCD
#include <Time.h> //for RTC
#include <Wire.h> //for LCD and RTC



// Connections to Arduino
//SDA  = A4 
//SCL = A5
//Radio 9,10,13,11,12,2 , A0=14 A1=15 A2=16 A3=17 A4=18 A5=19 A6=20
#define ONE_WIRE_BUS 4			// Dallas Temp Sensors on D4
#define phPin 16				// analog pH probe sensor on A2
#define beepPin 17				// Piezo Beeper - A3 
#define solarpumpPin 5			// Solar Pump SSR
#define acidpumpPin 6			// Acid Pump SSR
#define filterpumpPin 7			// Filter Pump SSR
#define chlorinatorPin 8		// Chlorinator SSR

#define remotePin 3				//this is D3 which Interrupt 1 for the Overide/Manual button
#define lightPin 14				//pin A0 for the Pool Light SSR
#define SPAREPin 15				//SPARE pin A1 WIRED TO CONNECTOR PIN7


//set parameters for Overide Remote
boolean remoteIsOn	= FALSE;
int remoteMode		= 1;

//time object and hardcode metric
//boolean metric = true; // not sure i need this now with mQTT
tmElements_t tm;


//Set LCD Address
BV4208 lcd(0x21);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature dallasSensors(&oneWire);

//set Testing vs Productions variables and settings
#if TESTBED == 1
char sketch_name[] = "Pool Controller-TESTBED";
char sketch_ver[]  = "2016.02.22b1";
int desiredPoolTemp = 31; //set initial desired/max pool temperature - this can be overwritten by the controller
int differential = 2; // Set the differential to switch on the pumps. In production this should be about 15, on testbed 2
int acidpumpOnTime = 5; //default minutes on the hour to run the acid feeder for - this can be overwritten by the controller
int veraRequestFreq = 30; //how many cycles to ask vera for data
#define SUMMERMAXPUMPRUNTIME 25000  // 28800 seconds = 8 hours, 36000 = 10 HOURS
#define WINTERMAXPUMPRUNTIME 14400 // 4 Hours only during winter. - Winter could also be enabled when pooltemp drops to 18deg
// Set Temp Sensor addresses
DeviceAddress roofThermometer = { 
  0x28, 0x61, 0x17, 0x1B, 0x04, 0x00, 0x00, 0xF5 };
DeviceAddress poolThermometer = { 
  0x28, 0xEB, 0x68, 0xE3, 0x02, 0x00, 0x00, 0x6E };
#endif

#if TESTBED == 0
char sketch_name[] = "Pool Controller";
char sketch_ver[]  = "2016.03.01r1";
int desiredPoolTemp = 27; //set initial desired/max pool temperature - this can be overwritten by the controller
int differential = 15; // Set the differential to switch on the pumps. In Production this should be about 15, on testbed 0
int acidpumpOnTime = 05; //how many minutes on the hour to run the acid feeder for.
int veraRequestFreq = 60; //how many cycles to ask vera for data
#define SUMMERMAXPUMPRUNTIME 28800  // 28800 seconds = 8 hours, 36000 = 10 HOURS
#define WINTERMAXPUMPRUNTIME 14400 // 4 Hours only during winter. - Winter could also be enabled when pooltemp drops to 18deg
// Set Temp Sensor addresses
DeviceAddress roofThermometer = { 
  //0x28, 0x76, 0x30, 0xE3, 0x02, 0x00, 0x00, 0x3E };
0x28, 0x61, 0x17, 0x1B, 0x04, 0x00, 0x00, 0xF5 }; //dev
DeviceAddress poolThermometer = { 
  //0x28, 0xBE, 0xFF, 0x0C, 0x02, 0x00, 0x00, 0x9E };
0x28, 0xEB, 0x68, 0xE3, 0x02, 0x00, 0x00, 0x6E }; //dev
#endif




boolean filterpumpRunning = false;
boolean solarpumpRunning = false;
boolean veraRemote = false;
boolean timeReceived = false;

int maxPumpRunTime;
int veraLoopCount = 0; //counter for Vera update period

int currentMonth;
int currentHour;
int currentMinute;
int currentDay;
int currentSec;
float idealpH = 7.30; // set the ideal pH level for the pool
float acidpumpRunTime = 0;
int SolarStartcount = 0;
int SolarStartDelay = 6; // delay for solar pump startup
int ItsWinter = 0;

unsigned long yesterdaysPRT;
unsigned long pumpRunTime = 0;
unsigned long loopTime;
unsigned long loopstartmillis = 0;

long lastPumpstart = 0L;
long lastPumpstop = 50000L; // give it 50 secs for initial starting.
long huristicdelay = 60000L; //~60 sec delay between pump restarts


float lastroofTemp = 0;
float lastpoolTemp = 0;
float lastpHval = 0;
int lastpTime = 0;
int lastacidTime = 0;


int pTime ;
int bTime = 0;
int beepState = LOW;
long  pBeepMillis = 0;

//pH Settings - change when calibrating
//would be good to calibrate from vera
float volt4 = 3.341;
float volt7 = 2.304;
float calibrationTempC = 31.1;
float avgMeasuredPH = 0;


// Array of Status codes 0-9
char* strStatus[]= {
  "OFFPEAK","MAX PRT","POOLMAXTEMP","HEATING", "DELAY", "NO SUN", "READY", "ERR-SHIT","REMOTE ON","WINTER"};
int statuscode; // a number for each of the above 0-9
int lastStatusCode; //used for sending status to Vera


//Define MySensors child devices
#define Roof_CHILD_ID 1		// Id of the sensor child
#define Pool_CHILD_ID 2		// Id of the sensor child
#define pH_CHILD_ID 3		// Id of the sensor child
#define prt_CHILD_ID 4		//id of the PumpRunTime child
#define hcl_CHILD_ID 5		//id of the Acid Pump Runtime child
#define Pump_CHILD_ID 6		//id of the Pump Overide switch child
#define Light_CHILD_ID 7		//id of the Pool Light switch child
#define Controller_CHILD_ID 8	//id for Controller - for sending status.

// Initialize temperature message
MyMessage roofTempmsg(Roof_CHILD_ID, V_TEMP);
MyMessage poolTempmsg(Pool_CHILD_ID, V_TEMP);
MyMessage pHValuemsg(pH_CHILD_ID, V_TEMP);
MyMessage prtValuemsg(prt_CHILD_ID, V_TEMP);
MyMessage hclValuemsg(hcl_CHILD_ID, V_TEMP);
MyMessage PumpCntrlmsg(Pump_CHILD_ID, V_STATUS);
MyMessage LightCntrlmsg(Light_CHILD_ID, V_STATUS);
MyMessage ControlModemsg(Controller_CHILD_ID, V_TEXT);


unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 30000;  // 30s delay between updates to Vera




void presentation() {
	// Send the sketch version information to the gateway and Controller

	sendSketchInfo(sketch_name, sketch_ver);

	//Describe the device type...probably want this to be HVAC??
	present(Roof_CHILD_ID, S_TEMP);
	present(Pool_CHILD_ID, S_TEMP);
	present(pH_CHILD_ID, S_TEMP);
	present(prt_CHILD_ID, S_TEMP);
	present(hcl_CHILD_ID, S_TEMP);
	present(Pump_CHILD_ID, S_LIGHT);
	present(Light_CHILD_ID, S_LIGHT);
	present(Controller_CHILD_ID, S_INFO);



}

void setup(void)
{
	//Set the RTC as the timekeeper - vera just for sync.
	setSyncProvider(RTC.get);

//request data from vera
	//gw.request(Pool_CHILD_ID,V_VAR1); // ask vera for pool set temp
	requestTime(); //get time in case rtc isnt set. Openhab needs a rule to look for this msg and send it. The vera plugin sent the time automatically.


  //Setup LCD
	lcd.bl(0); // turn BL on
	lcd.cls(); // clear screen

  //Set Pin Modes
	pinMode (acidpumpPin, OUTPUT);
	pinMode (solarpumpPin, OUTPUT);
	pinMode (filterpumpPin, OUTPUT);
	pinMode (chlorinatorPin, OUTPUT); 
	pinMode (beepPin, OUTPUT); 
	pinMode(lightPin, OUTPUT);
	pinMode(remotePin,INPUT);
	digitalWrite(remotePin,HIGH);

	//Create Interrupt routine for Remote on D3
	attachInterrupt(1,isrRemote, LOW);


  // Start up the Dallas Temp library and set the resolution to 9 bit
	dallasSensors.begin();
	dallasSensors.setResolution(poolThermometer, 12);
	dallasSensors.setResolution(roofThermometer, 12);

	// Get temperature values
	dallasSensors.requestTemperatures();
	float init_poolTemp = dallasSensors.getTempC(poolThermometer);
	Serial.println(init_poolTemp);

//End of setup
}





void loop(void)
{ 
   
  loopstartmillis = millis(); //start a counter for the pump runtimes  
  veraLoopCount ++; //this counter for requesting data only periodically
  
   
  //Send data to controller periodically in case it was missed/or they are out of sync
 if (veraLoopCount == veraRequestFreq){
		send(PumpCntrlmsg.set(filterpumpRunning == true ? 1 : 0));  // Send pump status to Vera

	 veraLoopCount = 0; //reset the counter
  }

//Send Status to MQTT
if (lastStatusCode != statuscode){
	send(ControlModemsg.set(strStatus[statuscode]));  // Send pump status to Vera
lastStatusCode = statuscode;
}




//Sync the time with Vera - typically for DST changes.
 if ((currentDay == 0) && (currentHour == 3) && (currentMinute == 01) && (currentSec == 5)) { // Sync time on Sunday at 3:01:05 from Vera - DST times change on Sundays 2/3AM
   requestTime();
 }




// Get temperature values
  dallasSensors.requestTemperatures();
  //if (filterpumpRunning){
  float poolTemp = dallasSensors.getTempC(poolThermometer); //should only really do this when the pump is runnign as the pipes can be different temp to the pool. Would need to set inital value and turn on pump if its set..
  //}
  float roofTemp = dallasSensors.getTempC(roofThermometer);


//Send metrics to MQTT
  if (millis() - lastConnectionTime > postingInterval) {
	  Serial.println("Publishing data...");
	  sendData(roofTemp, poolTemp, avgMeasuredPH, pumpRunTime, acidpumpRunTime);
  }



  //Print Data to LCD
  lcd.bl(0);   //Ensure Backlight is turned on!
  digitalClockDisplay();
  printRoofTemp(roofTemp);
  printPoolTemp(poolTemp);
  printStatus(statuscode);
  printAcidtime(acidpumpRunTime);
  printPtime(pumpRunTime);
  printPHInfo(avgMeasuredPH);



//remote overide control section
  //if switch in on then Isremoteonoff=true
  //if remote is on toggle a state remotemode
    if (IsRemoteOnOff()) {
   // Serial.println("Remote Mode");
		if (remoteMode == 1) {
		}
		else if (filterpumpRunning == true)  {
			//Serial.println("Filter Pump is running... will stop it");
			stopFilterPump();
			stopSolarPump();
			// reset any timers
			lastPumpstart = 0;
			lastPumpstop = 0;
			remoteMode = 1;
		}
		else if (filterpumpRunning == false)  {
			//Serial.println("Filter Pump is Stopped... will Start it");
			startFilterPump();
			lastPumpstart = 0;
			lastPumpstop = 0;
			remoteMode = 1;
		}
		
		doBeep();  //beep beeper to indicate remote is active
    
	}  
	else {
	//Serial.println("Remote is Off");
		remoteMode = 0 ;
  }
	
  //Check if its Winter or Summer
  if (IsItWinter()) {
    ItsWinter = 1;
    maxPumpRunTime = WINTERMAXPUMPRUNTIME;
   }
  else {
    ItsWinter = 0;
    maxPumpRunTime = SUMMERMAXPUMPRUNTIME;
   }

  
  // we want to reset the pump runtime counter at 7 am everyday
  // we will dedicate the entire 0 minute of 7 am to the reset to ensure we catch it.
  if ((currentHour == 7) && (currentMinute == 0)) {
    yesterdaysPRT = pumpRunTime;
    pumpRunTime = 0;
    
    delay(5000);    // rethink about this delay, maybe longer?
    return;         // don't know if this will break out of the loop()
  }


//log amount of acid delivered....


//Begin pH stuff
  int x;
  int sampleSize = 500;
  float avgRoomTempMeasuredPH =0;
  float avgTemp = 0;
  float avgPHVolts =0;
  float avgVoltsPerPH =0;
  float phTemp = 0;

  float tempAdjusted4 = getTempAdjusted4();

  for(x=0;x< sampleSize;x++)
  {
    float measuredPH = measurePH();
    float phTemp = poolTemp;
    float roomTempPH = doPHTempCompensation(measuredPH, phTemp);
    float phVolt = measurePHVolts();

    avgMeasuredPH += measuredPH;
    avgRoomTempMeasuredPH += roomTempPH;
    avgTemp += phTemp;
    avgPHVolts += phVolt;
  }

  avgMeasuredPH /= sampleSize;
  avgRoomTempMeasuredPH /= sampleSize;
  avgTemp /= sampleSize;
  avgPHVolts /= sampleSize;


  // *****Begin pH monitoring section ****
  //get the ph and start adding if needed. 
  //Run the acid pump for a count of X - reset the count every 30mins
  //need to add a timer so only runs for 5mins or so each hour.

  if ((avgMeasuredPH > idealpH) && ( currentMinute < acidpumpOnTime )) { //Turn on acid pump if needed and first few minutes of the hour.
    startAcidPump();
  }	else {
    stopAcidPump();	//Turn off the acid pump if the ph is OK or not the right time.
  }
  //perhaps fire an alarm if PH really OUT!



                                                                                                                                                                                                                                                                                 
// *****Begin Heating and Pump control logic****
  if ((pumpRunTime >= maxPumpRunTime) && (remoteMode == 0)){
    statuscode = 1; //MaxPumpTime
	    stopFilterPump();
		stopSolarPump(); //this may not be necessary but better to leave here in case pool is heating in OP hours!
    return; //not sure what this return does???
  }


 else if ((IsOffPeak()) && (pumpRunTime < maxPumpRunTime) && (remoteMode == 0))  { // If its Off Peak then run the pump for the remaining required time
		startFilterPump();
		statuscode = 0; //Off Peak
	 }


 
   else if ((poolTemp >= (desiredPoolTemp + 0.5)) && (remoteMode == 0)) { // This condition is when the pool has reached MAX temperature. No pumps running during peak hours
	statuscode = 4; // heuristic - temporary state until timers expire
	if (millis() - lastPumpstart > huristicdelay) { //stop with huristics
	  	stopSolarPump();
		stopFilterPump();
		statuscode = 2; //Pool Max Temp
	 } 
  }

      
  else if ((roofTemp > poolTemp + differential) && (poolTemp < desiredPoolTemp) && (ItsWinter == 0) && (remoteMode == 0)) { // This condition is when the pool will be heated - both pumps running.  
    statuscode = 4; // heuristic - temporary state until timers expire
	if (millis() - lastPumpstop > huristicdelay) { //start with huristics
		startFilterPump();
		startSolarPump();
		statuscode = 3; // HEATING
	  }
  }
  

  else if ((roofTemp <= poolTemp + differential) && (remoteMode == 0)) { //This condition is for when there is not enough solar heat to switch on the pumps.  
    if (pumpRunTime == 0 ){ //bodge to set initial statuscode
		statuscode = 6; // "READY"
    }

    
  else if (remoteMode == 0)  {
	  statuscode = 4; // heuristic - temporary state until timers expire
	  if (millis() - lastPumpstart > huristicdelay) { //stop with huristics
		stopFilterPump();
		stopSolarPump();
		statuscode = 5; //NO SUN
    } 
	  }
  }
	
  else if (remoteMode == 1){
	statuscode = 8; //Remote ON
}

  else if (ItsWinter == 1) {
    statuscode = 9; // WINTER
  }
  else {
    // if we get to here for whatever reason, we should just stop the pumps
    //this will occur if poolTemp>maxpooltemp and roofTemp>pool+differential
    //    stopFilterPump();
    //    stopSolarPump();
        statuscode = 7; // ERR-SHIT
    //statuscode = 4; // heuristic
  }





  //Generate the Pump Runtime for how long the pump needs to run each day.
  if (filterpumpRunning == true){
		loopTime = (millis() - loopstartmillis)/1000; //calculate how long the loop took to run
		pumpRunTime =(pumpRunTime+loopTime); //calculate the new pumpRunTime value
  }
 
}  //end of sketch


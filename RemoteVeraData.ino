
//send the Values to MQTT - send every postingInterval
void sendData( float roofTemp,float poolTemp, float pH, int pTime,int acidTime) {
	if (lastroofTemp != roofTemp && roofTemp != -127) {
		send(roofTempmsg.set(roofTemp, 1));	
	}
	if (lastpoolTemp != poolTemp && poolTemp != -127) {
		send(poolTempmsg.set(poolTemp, 1));
	}
	if (lastpHval != avgMeasuredPH) {
		send(pHValuemsg.set(avgMeasuredPH, 1));
	}
	if (lastpTime != pTime) {
		send(prtValuemsg.set((pTime/60 )));
	}
	if (lastacidTime != acidTime) {	
		send(hclValuemsg.set((acidTime/60)));
	}
	
	lastConnectionTime = millis(); // note the time that the connection was made or attempted:

}

void isrRemote() //interrupt routine for remote button press
{                
	//debounce the button
	static unsigned long lastDebounceTime;
	if ((millis() - lastDebounceTime) > 200) {   //the delay (debounce) time, button is completley closed.
		lastDebounceTime = millis();
		//Button has been pressed, so toggle the remote status
			remoteIsOn = !remoteIsOn;
			lastPumpstop = 0L;
			lastPumpstart = 60000L;
	}

}


void receive(const MyMessage &message) {
	// We only expect one type of message from controller. But we better check anyway.
	Serial.println(message.data);


	if (message.isAck()) {
		Serial.println("This is an ack from gateway");
	}

	if ((message.sensor==Pump_CHILD_ID) && (message.type == V_STATUS)) { //if the Vera switch is ON then the pumps must be running as the pumps will update the status so any message here would mean the over-ride is wanted
			//so toggle the remote state
			remoteIsOn = !remoteIsOn;
			Serial.print("Vera Remote: ");
			Serial.println(remoteIsOn);
	}
	
	if ((message.sensor==Light_CHILD_ID) && (message.type==V_STATUS)) {  // Turn the pool light on or off - only controlled by Vera.
		digitalWrite(lightPin, message.getBool()?1:0);
		Serial.print("Pool Light status: ");
		Serial.println(message.getBool());
		}
	
	
	if ((message.sensor==Pool_CHILD_ID) && (message.type==V_VAR1) && (message.getInt() >25 )) {  // this is for the desired pool water temp.
				desiredPoolTemp = (message.getInt()); // over-ride the default value
				Serial.print("Desired Pool Temp received: ");
				Serial.println(message.getInt());
			}
 //dont want to do this for now....
	if (message.sensor==pH_CHILD_ID) {
		//	if (message.type==V_VAR1) && (message.getFloat() > 7.0) {		//this is for the desired pH level set in VAR1 
			//	Serial.print("pH reqested level from Vera received : ");
			//	Serial.println(message.getFloat());
			//}
			
		if (message.type==V_VAR2) {		//this is for the desired acid feeder runtime per hour
			acidpumpOnTime = (message.getInt()); // over-ride the default value
			Serial.print("AcidFeeder runtime from Vera received : ");
			Serial.println(message.getInt());
			}
	}
	
}


boolean IsRemoteOnOff()
{
		if (remoteIsOn)
		{
			return true;  //remote is active
		}
		else
		{
			return false; //remote is not active
		}
		
}


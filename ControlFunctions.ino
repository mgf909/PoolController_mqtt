//control functions
void startAcidPump() {
	// this should only run if filter pump is running
	if (filterpumpRunning == true) {
		//start Acid Pump;
		digitalWrite (acidpumpPin, HIGH);
		//Update Acid Runtime
		acidpumpRunTime =(acidpumpRunTime+loopTime);
		}

	// dont want this as acid will run continuously if off peak hours.
	if (filterpumpRunning == false) {
		//stop Acid Pump;
		digitalWrite (acidpumpPin, LOW);
	}
}

void stopAcidPump() {
	digitalWrite (acidpumpPin, LOW);
}

void startSolarPump() {
	// Need a delay here so as to allow the filter pump to get the water flowing before starting the solar pump
	SolarStartcount++;
	if (SolarStartcount >= SolarStartDelay) {
		if (solarpumpRunning == false) {
			//start Solar Pump;
			digitalWrite (solarpumpPin, HIGH);
			solarpumpRunning = true;
		}
	}
}

void stopSolarPump() {
	if (solarpumpRunning == true) {
		// stop pump
		digitalWrite(solarpumpPin, LOW);
		//Reset the status and counters
		solarpumpRunning = false;
		SolarStartcount = 0;
	}
}

void startFilterPump() {
	if (filterpumpRunning == false) {
		lastPumpstart = millis();
		send(PumpCntrlmsg.set(1));  // Send pump status to MQTT
			digitalWrite(filterpumpPin, HIGH);
			digitalWrite(chlorinatorPin, HIGH);
		filterpumpRunning = true;
	}
}

void stopFilterPump() {
	if (filterpumpRunning == true) {
		// stop pump
		lastPumpstop = millis();
		send(PumpCntrlmsg.set(0));   // Send pump status to MQTT
			digitalWrite(filterpumpPin, LOW);
			digitalWrite(chlorinatorPin, LOW);
		filterpumpRunning = false;

	}
}

void doBeep() {
	//Serial.print(bTime);
	if (bTime == 20 ) { //this sets the time between beeps
		digitalWrite(beepPin, HIGH);
		delay(400);
		digitalWrite(beepPin, LOW);
		bTime = 0 ;
	}
	else
	bTime = bTime++ ;
}

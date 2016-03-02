
// This is called when a new time value was received
void receiveTime(unsigned long VeraTime) {
	Serial.println("time received!!");

	RTC.set(VeraTime); // this sets the RTC to the time from Vera - which i do want periodically
	timeReceived = true;
}
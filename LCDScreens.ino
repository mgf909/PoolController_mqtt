//LCD Display Functions
void digitalClockDisplay(){
	tmElements_t tm;
	(RTC.read(tm));
	{
		lcd.position(0,0);
		lcd.print("                    ");  // clear the previous whatever
		lcd.position(0,0);
		print2digits(tm.Hour);
		lcd.print (tm.Hour);
		currentHour = tm.Hour;            //currentHour used for resetting prt timer daily
		lcd.dat(0x3A);  //prints a ":"
		print2digits(tm.Minute);
		lcd.print(tm.Minute);
		currentMinute = tm.Minute;        //currentMinute used for resetting prt timer daily
		lcd.dat(0x3A);  //prints a ":"
		print2digits(tm.Second);
		currentSec= tm.Second;				//used for checking to see if the time needs to be synced
		lcd.print (tm.Second);

		lcd.position(0,10);
		print2digits(tm.Day);
		currentDay= tm.Wday;				//used for checking to see if the time needs to be synced
		lcd.print(tm.Day);
		lcd.dat(0x2F); //prints a "/"
		print2digits(tm.Month);
		lcd.print(tm.Month);
		currentMonth = tm.Month; //set for use in sketch for Winter
		lcd.dat(0x2F); //prints a "/"
		lcd.print(tmYearToCalendar(tm.Year));
	}
}

void print2digits(int number) {
	if (number >= 0 && number < 10) {
		// Serial.write('0');
		lcd.print('0');
	}
	//Serial.print(number);
}

void printRoofTemp(float tempA) {
	lcd.position(2,0);
	lcd.print("          ");  // clear 10spaces
	lcd.position(2,0);
	lcd.print("Roof=");
	//Convert float to string - float,width,precision,buffer
	char buffer[5];
	dtostrf(tempA,4,1, buffer);
	lcd.print(buffer);
}

void printPoolTemp(float tempA) {
	lcd.position(3,0);
	lcd.print("          ");  // clear 10spaces
	lcd.position(3,0);
	lcd.print("Pool=");
	//Convert float to string - float,width,precision,buffer
	char buffer[5];
	dtostrf(tempA,4,1, buffer);
	lcd.print(buffer);
}

void printStatus(int statusCode) {
	lcd.position(1,0);
	lcd.print("          ");  // clear 10spaces
	lcd.position(1,0);
	lcd.print(strStatus[statuscode]); //Prints status to LCD
}

void printPHInfo(float ph) {
	lcd.position(3,10);
	lcd.print("          ");  // clear 10spaces
	lcd.position(3,10);
	lcd.print("pH =");
	//Convert float to string - float,width,precision,buffer
	char buffer[5];
	dtostrf(ph,4,1, buffer);
	lcd.print(buffer);
}

void printAcidtime(int acidTime) {
	acidTime = (acidpumpRunTime/60 ); // Change to minutes
	lcd.position(1,10);
	lcd.print("          ");  // clear 10spaces
	lcd.position(1,10);
	lcd.print("HCL=");
	lcd.print(acidTime); //Prints Pump Runtime min to LCD
}

void printPtime(int pTime) {
	pTime = (pumpRunTime/60 ); // Change to minutes
	lcd.position(2,10);
	lcd.print("          ");  // clear 10spaces
	lcd.position(2,10);
	lcd.print("PRT=");
	lcd.print(pTime); //Prints Pump Runtime min to LCD
}


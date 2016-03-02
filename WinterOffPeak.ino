
boolean IsItWinter() {
	if (
	(currentMonth == 4 ) or
	(currentMonth == 5 ) or
	(currentMonth == 6 ) or
	(currentMonth == 7 ) or
	(currentMonth == 8) ){ ///change back to 8 when finished testing
		return true;
	}
	else {
		return false;
	}
}

boolean IsOffPeak() {
	if (
	(currentHour == 23 ) or
	(currentHour == 00 ) or
	(currentHour == 01 ) or
	(currentHour == 02 ) or
	(currentHour == 03 ) or
	(currentHour == 04 ) or
	(currentHour == 05 ) or
	(currentHour == 06 ) ){
		return true;
	}
	else {
		return false;
	}
}


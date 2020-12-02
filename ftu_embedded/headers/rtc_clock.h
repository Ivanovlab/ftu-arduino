/*
 * IncFile1.h
 *
 * Created: 2020-12-02 11:11:36 AM
 *  Author: Rohit
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

/************************************************************************/
/* RTC FUNCTIONS                                                        */
/************************************************************************/
/**
 * \@brief Function is raised when RTC clock alarm goes off
 * 
 * \@param void
 * 
 * \@return void
 */
RTCZero configure_rtc_timer(){
	// Create an rtc object
	RTCZero rtc;
	// Start the rtc object
	rtc.begin();
	// Set the time in the form of (hours, minutes, seconds)
	rtc.setTime(0,0,0);
	return rtc;	
}

/**
 * \@brief Function is raised when RTC clock alarm goes off
 * 
 * \@param void
 * 
 * \@return void
 */
void alarmMatch(){
	Serial.println("Alarm Match!");
}



/**
 * \@brief Attaches an alarm interrupt to go off in x seconds 
 * 
 * \@param RTC clock, timer length
 * 
 * \@return RTC clock
 */
RTCZero attach_alarm_interrupt(RTCZero rtc, int seconds){
  rtc.begin();
  rtc.setTime(0, 0, 0);
  rtc.setAlarmTime(0,0,seconds);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(alarmMatch);
  return rtc;
}


#endif /* INCFILE1_H_ */
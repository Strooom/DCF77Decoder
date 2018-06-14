DCF77Decoder::DCF77Decoder()
	{
	pinMode(DCFsignalInputPin, INPUT);
	attachInterrupt(DCFinterrupt, intHandler, CHANGE);
	}
	

DCF77Decoder::~DCF77Decoder()
	{
	// Detach interrupt
	}
	

void DCF77Decoder::intHandler()
  {
  long width;
  if (!digitalRead(DCFsignalInputPin))
    {																		 	// signal went 'back to 0' so we just had the end of a pulse
	width = millis() - msUp;													// measure pulse width as (now() - rising flank)
    // adjustSignalQuality((width < DCFMaxPulseWidth) 
    // && (width > DCFMinPulseWidth));											// could additionally check if the width of the pulse is valid

 	if ((signalQuality > signalQualityHigh) && msZero)
 		{
		iSecond = ((millis()-msZero) / 1000) % 60;								// seconds is calculated as now - beginning of minute
 		DCFbit[iSecond] = (width > DCFMidPulseWidth);							// write signal bit into buffer
 		}
	ready=true;																	// set flag to proceed in loop()
	}
  else
	{  																		 	// signal went up to '1' so we just had the end of a pause
	width = millis()-msUp;														// measure time between pulses as now() - previous rising flank
	msUp = millis();															// remember now() for future calculation
    adjustSignalQuality((width < DCFMaxWidth) && (width > DCFMinWidth));		// check width between pulses to assess signal quality  -should be around 1000 ms
 	if ((signalQuality > signalQualityHigh) && (width > DCFMinPause59Width))
 		{																		// detecting decent signalquality and extralong pause -> must be start of a 1-minute cycle
		msZero = msUp;															// remember 'absolute' time to later calculate offset in seconds of subsequent pulses
		syncLevel = getSyncLevel();												// perform all built-in checks on decoded signal to assess syncQuality
		if (syncLevel >= 9)														// if syncQuality is good enough, store decoded time
			{
			hours = calcFromBCD(29, 34);
			minutes = calcFromBCD(21, 27);
			day = calcFromBCD(36, 41);
			month = calcFromBCD(45, 49);
			year = calcFromBCD(50, 57);
			dayOfWeek = calcFromBCD(42, 44);
			}
 		}
	}
	}
  
  
  void DCF77Decoder::adjustSignalQuality(boolean valid)
    {
    if (valid)
    	{																		// if pulses/pauses have valid widths (between min AND max), then increase signalQuality counter
    	signalQuality++;
    	}
    else																		// if pulses/pauses have INvalid widths (outside min OR max), then decrease signalQuality counter
		{
    	signalQuality--;
    	signalQuality--;														// signalQuality decrease faster (-2 for each invalid puls/pause), so random pulses definitely decrease it
		}
    if (signalQuality > signalQualityMAX)
    	{
    	signalQuality = signalQualityMAX;										// max out on (eg) 15 - more levels could be used
    	}
    if (signalQuality < 0)
    	{
    	signalQuality=0;														// don't go under zero
    	}
	}

  int DCF77Decoder::getSyncLevel()
    {
	syncLevel=0;
	boolean canCompare=true;
	
 	if (signalQuality >= signalQualityHigh)										// if we don't receive decent pulses, then syncLevel is down to nothing...
 		{
																				// doing every possible check on the received bits. Every positive result increases syncLevel
		if (!DCFbit[0])															// should be 0
			{
			syncLevel++;
			}
		if (DCFbit[20])															// should be 1
			{
			syncLevel++;
			}
		if (!calcParity(21, 28))												// should be even parity
			{
			syncLevel++;
			}
		if (!calcParity(29, 35))												// should be even parity
			{
			syncLevel++;
			}
		if (!calcParity(36, 58))												// should be even parity
			{
			syncLevel++;
			}
		if (calcFromBCD(29, 34) < 60)											// hours should be < 60
			{
			syncLevel++;
			}
		if (calcFromBCD(21, 27) < 60)											// minutes should be < 60
			{
			syncLevel++;
			}
		if (calcFromBCD(36, 41) < 32)											// day of month should be < 32
			{
			syncLevel++;
			}
		if (calcFromBCD(45, 49) < 13)											// month should be < 13
			{
			syncLevel++;
			}
		if (hours == calcFromBCD(29, 34))										// does previous and current hours value match ?
			{
			syncLevel++;
			}
		if (day == calcFromBCD(36, 41))											// does previous and current day value match ?
			{
			syncLevel++;
			}
		if (dayOfWeek == calcFromBCD(42, 44))									// does previous and current dayOfWeek value match ?
			{
			syncLevel++;
			}
		if (month == calcFromBCD(45, 49))										// does previous and current month value match ?
			{
			syncLevel++;
			}
		if (year == calcFromBCD(50, 57))										// does previous and current year value match ?
			{
			syncLevel++;
			}
		}
	return syncLevel;
	}

boolean DCF77Decoder::calcParity(int from, int to)
	{
	boolean parity=false;
	int i=0;
	for(i=from;i<=to;i++)
		{
		if (DCFbit[i])
			parity = !parity;
		}
	return parity;
	}
	
int DCF77Decoder::calcFromBCD(int from, int to)
	{
	int result=0;
	int i=0;
	for(i=from;i<=to;i++)
		{
		if (DCFbit[i])
			result = result + BCD[i-from];
		}
	return result;
	}
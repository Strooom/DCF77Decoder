/* ##########################################################################
   ### DCF77 Library for Arduino                                          ###
   ### Author : Pascal Roobrouck                                          ###
   ### License : Creative Commons BY-NC-SA                                ###
   ### V0.1 - 13-Jan-2015                                                 ###
   ###                                                                    ###
   ###                                                                    ###
   ### Credits/CC : Inspired by Thijs Elenbaas (2012) and                 ###
   ### ST6 code/experience from Pascal Roobrouck (1989)                   ###
   ###                                                                    ###
   ########################################################################## */
    
// Constants - Parameters

#define DCFShortPulseWidth 103	// width of a short pulse [ms] - Short pulse should be ~100ms so default = 100
#define DCFLongPulseWidth 185	// width of a long pulse [ms] - Long pulse should be ~200ms so default = 200
								// 'measure' these pulses (by adding debug statements) and adjust to your HW readings
#define DCFMidPulseWidth 144	// set to (DCFShortPulseWidth + DCFLongPulseWidth)/2 Default = 150
#define DCFMinPause59Width 1800	// any pause longer than 1800 is considered to be the missing 59th second

#define DCFMinPulseWidth 50		// pulses should be min 100ms, shorter than 50ms indicates bad signal reception..
#define DCFMinPauseWidth 750	// pause should be min 800ms, shorter than 750ms indicates bad signal reception..
#define DCFMaxPulseWidth 250	// pulses should be max 200ms, longer than 250ms indicates bad signal reception..
#define DCFMaxPauseWidth 1950	// pause should be max 1900ms, longer than 1950ms indicates bad signal reception..
								// all these thresholds take an error margin of 50ms, you could adjust these
#define DCFMinWidth 950			// pulse should be spaced 1000 ms, (except for bit 59->0), shorter indicated bad signal reception
#define DCFMaxWidth 1050		// pulse should be spaced 1000 ms, (except for bit 59->0), longer indicated bad signal reception

#define signalQualityMAX 15		// maximum level of signalQuality variable
#define signalQualityLow 3		// threshold for 'insufficient' signal quality
#define signalQualityHigh 12	// threshold for 'sufficient' signal quality

// HardWare

#define DCFsignalInputPin 2	 	// pin on which the digital signal from DCF receiver is to be read
#define DCFinterrupt 0		 	// Interrupt number associated with DCFsignalInputPin


// Classes - Variables

class DCF77Decoder
	{
	private:
		// private data
		const int BCD[8] = {1, 2, 4, 8, 10, 20, 40, 80};						// array of constants, to make simple BCD calculations
		int seconds;															// counter used to number the pulses in the 1-minute cycle. Set to 0 after detecting the missing pulse at 59, then incremented on each pulse
		long msUp;																// last time in ms there was a rising signal 0->1
																				// consecutive values of msUp can calculate spacing of bits, which should be 1000 ms
																				// millis() - msUp yields the width of a pulse
		long msZero;															// millis() value at start of a 1-minute cycle, used to easily calculate the relative time-position of signal pulses
		boolean dataBuffer[60] = {true};										// buffer holding all bits for 1 minute. Initialized to all '1', so it fails most built-in check validations


int hours;
int minutes;
int day;
int month;
int year;
int dayOfWeek;

		
		// private methods
		void adjustSignalQuality(boolean valid);
		int getSyncLevel();
		boolean calcParity(int from, int to);
		int calcFromBCD(int from, int to);
		void intHandler();
	public:
		// public data
		int signalQuality = 0;													// indicates how 'stable' the digitized antenna signal is. By measuring pulse-width, pause-width and puls-spacing and comparing this to preset boundaries
		int syncQuality = 0;													// indicates how good the sync is, calculated by doing error-checks on decoded signal
		// public methods - interface
		DCF77Decoder();															// declaration of constructor
	};




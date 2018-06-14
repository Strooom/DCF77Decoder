// ##########################################################################
// ### DCF77 Library for Arduino                                          ###
// ### Author : Pascal Roobrouck                                          ###
// ### License : Creative Commons BY-NC-SA                                ###
// ### V1.0.0  15-May-2018                                                ###
// ##########################################################################

#ifndef DCF77Decoder_h
#define DCF77Decoder_h

#include <Arduino.h>
#include <Time.h>
#include <TimeLib.h>

enum class DCF77SyncState
    {
    searchSteadyPulses,
    searchCycleStart,
    readingMinute1,
    readingMinute2,
    synced,
    nmbrSyncStates
    };

class DCF77Decoder
    {
    public:
        DCF77Decoder(uint8_t thePinNmbr);					// Constructor
        ~DCF77Decoder();									// Destructor

    private:
        uint8_t pinNmbr;									// pin number to which our DCF signal is read

        boolean prevSignal = false;							// what was the prvious signal (high or low), so we can detect flanks (rising or falling)
        uint32_t lastEdge = 0;								// what time was the last rising or falling edge - so we can measure the pulse width / gap width

        const static uint32_t minPulseWidth = 75;			// minimum width of a pulse considered as valid
        const static uint32_t maxPulseWidth = 225;			// maximum width of a pulse considered as valid
        const static uint32_t minGapWidth = 775;			// minimum width of a gap considered as valid
        const static uint32_t maxGapWidth = 925;			// maximum width of a gap considered as valid
        const static uint32_t minGapWidth59 = 1775;			// minimum width of a gap considered as the missing pulse in last second

        uint8_t signalQuality = 0;							// if pulses have proper timings, this value goes up.. if not, the value goes down.
        const static uint8_t maxSignalQuality = 24;			// this is the max value for signalQuality
        const static uint8_t signalQualityThreshold = 16;	// above this value, we consider the DCF-signal to be good enough to start cycle decoding

        DCF77SyncState syncState = DCF77SyncState::searchSteadyPulses;

        uint32_t cycleStart = 0;							// time in millis() when we detected the start of a 1-minute cycle
        boolean data[60] = { true };						// array holding all bits for 1 minute.

        static const int BCD[8];							// array of constants, to make simple BCD calculations

        time_t lastMinute;									// I thought this was a funny variable name :-)
        boolean s1, s2, s3;
        uint32_t sampleTime;

        // private methods
        void adjustSignalQuality(boolean valid);			// increase/decrease signalQuality when condition is true/false
        void adjustSyncState();								// TBC

        boolean checkParity(uint8_t from, uint8_t to);		// Calculate even parity over data[from] to data[to]
        uint8_t calcFromBCD(uint8_t from, uint8_t to);		// Calculate value encoded in data[from] to data[to]
        boolean checkData();								// Check is data[] contains valid info, based on fixed bits, parity, etc...
		time_t DCF77Decoder::encodeTime();

    public:
        void run();											// main workhorse of the decoder : run every 10ms or faster
        DCF77SyncState getSyncState();						// answers what is the current syncState
        time_t getTime();									// answers current decoded time, or 0 in case of no sync
    };



#endif // !DCF77Decoder_h

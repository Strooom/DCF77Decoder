#include "DCF77Decoder.h"
#include <Time.h>
#include <TimeLib.h>

const int DCF77Decoder::BCD[8] = { 1, 2, 4, 8, 10, 20, 40, 80 };				// initialization of a static member of the class

// Public:

DCF77Decoder::DCF77Decoder(uint8_t thePinNmbr) : pinNmbr(thePinNmbr)
    {
    pinMode(pinNmbr, INPUT);													// could be INPUT_PULLUP if your antenna has an open-collector output..
    }

DCF77Decoder::~DCF77Decoder()
    {
    }

DCF77SyncState DCF77Decoder::getSyncState()
    {
    return syncState;
    }

time_t DCF77Decoder::getTime()
    {

    }

void DCF77Decoder::run()
    {
    boolean currentSignal = digitalRead(pinNmbr);	// what is the signal now

    if (currentSignal && !prevSignal)				// is there a rising edge ?
        {
        uint32_t thisRisingEdge = millis();
        uint32_t gapWidth = thisRisingEdge - lastEdge;
        adjustSignalQuality((gapWidth > minGapWidth) && (gapWidth < maxGapWidth));
        adjustSyncState();

        lastEdge = thisRisingEdge;

        if ((signalQuality > signalQualityThreshold) && (gapWidth > minGapWidth59))
            {
            switch (syncState)
                {
                case DCF77SyncState::searchCycleStart:
                    cycleStart = thisRisingEdge;
                    syncState = DCF77SyncState::readingMinute1;
                    Serial.println("cycle start found");
                    break;
                case DCF77SyncState::readingMinute1:
                    cycleStart = thisRisingEdge;
                    if (checkData())
                        {
                        lastMinute = encodeTime();
                        Serial.println("first cycle ok");
                        syncState = DCF77SyncState::readingMinute2;
                        }
                    else
                        {
                        Serial.print("invalid data");
                        }
                    break;
                case DCF77SyncState::readingMinute2:
                    cycleStart = thisRisingEdge;
                    if (checkData())
                        {
                         time_t now = encodeTime();
                        if ((now - lastMinute) == 60)
                            {
                            syncState = DCF77SyncState::synced;
							Serial.println("sync!");
                            }
                        else
                            {
                            syncState = DCF77SyncState::readingMinute1;
							Serial.println("cycles mismatch");
						}
                        Serial.println(lastMinute);
                        }
                    else
                        {
                        Serial.print("invalid data");
                        syncState = DCF77SyncState::readingMinute1;
                        }
                    break;
                case DCF77SyncState::synced:
                    break;
                default:
                    break;
                }
            }
        }
    else if (!currentSignal && prevSignal)				// is there a falling egde ?
        {
        uint32_t thisFallingEdge = millis();
        uint32_t pulseWidth = thisFallingEdge - lastEdge;

        adjustSignalQuality((pulseWidth > minPulseWidth) && (pulseWidth < maxPulseWidth));
        adjustSyncState();

        lastEdge = thisFallingEdge;
        }
    prevSignal = currentSignal;

    if ((DCF77SyncState::readingMinute1 == syncState) || (DCF77SyncState::readingMinute2 == syncState))
        {
        uint32_t now = millis();
        uint32_t ms = (now - cycleStart) % 1000;						// How much time has passed since the beginning of the second

        if (ms < 50)
            {
            s2 = false;
            }

        if ((ms > 137) && !s2)
            {
            data[((now - cycleStart) / 1000) % 60] = currentSignal;		// Store the sample in the data array
            sampleTime = millis();
            s2 = true;
            }
        }
    }

// Private:

void DCF77Decoder::adjustSignalQuality(boolean valid)
    {
    if (valid)
        {
        if (signalQuality < maxSignalQuality)
            {
            ++signalQuality;
            }
        }
    else if (signalQuality > 0)
        {
        --signalQuality;
        }
    }

void DCF77Decoder::adjustSyncState()
    {
    if ((signalQuality > signalQualityThreshold) && (DCF77SyncState::searchSteadyPulses == syncState))
        {
        syncState = DCF77SyncState::searchCycleStart;
        Serial.println("stable pulses found");
        }

    if ((signalQuality < signalQualityThreshold) && (DCF77SyncState::searchSteadyPulses != syncState))
        {
        syncState = DCF77SyncState::searchSteadyPulses;
        Serial.println("stable pulses lost");
        }
    }

boolean DCF77Decoder::checkParity(uint8_t from, uint8_t to)
    {
    boolean parity=true;
    for(uint8_t i=from; i<=to; i++)
        {
        if (data[i])
            {
            parity = !parity;
            }
        }
    return parity;
    }

uint8_t DCF77Decoder::calcFromBCD(uint8_t from, uint8_t to)
    {
    uint8_t result=0;
    for(uint8_t i=from; i<=to; i++)
        {
        if (data[i])
            {
            result = result + BCD[i - from];
            }
        }
    return result;
    }

boolean DCF77Decoder::checkData()
    {
    boolean result = true;
    // Check fixed value bits
    if (data[0] || (!data[20]) || data[59])
        {
        result = false;
        }

    // Cannot be both CET and CEST
    if (data[17] && (data[18]))
        {
        result = false;
        }

    // Check Parities
    if (!checkParity(21, 28) || !checkParity(29, 35) || !checkParity(36, 58))
        {
        result = false;
        }

    // Check time components versus min / max values
    if (calcFromBCD(21, 27) > 60)
        {
        result = false;
        }
    if (calcFromBCD(29, 34) > 23)
        {
        result = false;
        }
    if ((calcFromBCD(36, 41) > 31) || (calcFromBCD(36, 41) == 0))
        {
        result = false;
        }
    if ((calcFromBCD(45, 49) > 12) || (calcFromBCD(45, 49) == 0))
        {
        result = false;
        }
    if (calcFromBCD(50, 57) < 18)
        {
        result = false;
        }

    return result;
    }

time_t DCF77Decoder::encodeTime()
    {
    tmElements_t tm;
    tm.Second = 0;
    tm.Minute = calcFromBCD(21, 27);
    tm.Hour = calcFromBCD(29, 34);
    tm.Day = calcFromBCD(36, 41);
    tm.Month = calcFromBCD(45, 49);
    tm.Year = 2000 - 1970 + calcFromBCD(50, 57);
    return makeTime(tm);
    }


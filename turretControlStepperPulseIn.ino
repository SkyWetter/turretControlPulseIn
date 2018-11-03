// **********   S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
// turret control firmware for esp32 dev kit C
//  october 31, 2018


// *********   P R E P R O C E S S O R S
#include <Stepper.h>
#include <BluetoothSerial.h>
#include <soc\rtc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys/time.h"

#include "sdkconfig.h"

#define GPIO_INPUT_IO_TRIGGER     0  // There is the Button on GPIO 0

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep 30 seconds and then wake up



// ********* P I N   A S S I G N M E N T S
// flow meter
#define pulsePin 23

// dome stepper
#define stepperDomeDirPin 19
#define stepperDomeStpPin 18
#define stepperDomeSlpPin 2
#define hallSensorDome 16
#define stepperDomeCrtPin 14

// valve stepper
#define stepperValveDirPin 5
#define stepperValveStpPin 17
#define stepperValveSlpPin 15
#define hallSensorValve 4
#define stepperValveCrntPin 12

// wake-up push button
#define wakeUpPushButton GPIO_NUM_13

// rgb led
#define rgbLedBlue 27
#define rgbLedGreen 26
#define rgbLedRed 25

// solar panel
#define currentSense A6
#define solarPanelVoltage A7

// states
#define sleep 0
#define solar 1
#define program 2
#define water 3
#define lowPower 4

int state = sleep;

// ********    P R O T O T Y P E S
void inputCase();

void solarPowerTracker();

void stepperOneStepHalfPeriod(byte x, byte y, byte z, int *q, int h);
void stepperDomeOneStepHalfPeriod(int hf);
void stepperValveOneStepHalfPeriod(int hf);
void stepperGoHome(byte x, byte y, byte z, byte s);
void domeGoHome();
void valveGoHome();

void stepperDomeDirCW();
void stepperDomeDirCCW();

void toggleStepperValveDir();
void valveStepperOneStep();

// ************* U S E R   D E F I N E D   V A R I A B L E S
// bluetooth
BluetoothSerial SerialBT;
byte stepperCase;

// steppers
int stepCountDome = 0;
int stepCountValve = 0;

byte hallSensorDomeVal;
byte hallSensorValveVal;

// pulse counter
double duration;

// power		
unsigned long currentSenseVal1, currentSenseVal2;		
float solarPanelVoltageVal;											// VALUE READ FROM GPIO 3   OR ADC7

// power management
// RTC_DATA_ATTR int bootCount = 0;									// this will be saved in deep sleep memory (RTC mem apprently == 8k)
RTC_DATA_ATTR static time_t last;									// remember last boot in RTC Memory
struct timeval now;

void setup()
{
	// bluetooth 
	Serial.begin(115200);
	SerialBT.begin("Rain|)Bow");							// RainBow is name for Bluetooth device

	// pin assignments
	pinMode(pulsePin, INPUT);								// pin to read pulse frequency										// init timers need for pulseCounters

	pinMode(stepperDomeDirPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper DIRECTION
	pinMode(stepperDomeStpPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper STEP
	pinMode(stepperDomeSlpPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper ENABLE
	pinMode(hallSensorDome, INPUT);

	pinMode(stepperValveDirPin, OUTPUT);					// OUTPUT pin setup for MP6500 to control VALVE stepper DIRECTION
	pinMode(stepperValveStpPin, OUTPUT);					// OUTPUT pin setup for MP6500 to control VALVE stepper STEP
	pinMode(stepperValveSlpPin, OUTPUT);					// OUTPUT pin setup for MP6500 to control VALVE stepper ENABLE
	pinMode(hallSensorValve, INPUT);
	
	pinMode(wakeUpPushButton, INPUT);

	pinMode(currentSense, INPUT);
	pinMode(solarPanelVoltage, INPUT);

	pinMode(rgbLedBlue, OUTPUT);
	pinMode(rgbLedRed, OUTPUT);
	pinMode(rgbLedGreen, OUTPUT);

	// init pin states
	digitalWrite(stepperDomeStpPin, LOW);
	digitalWrite(stepperValveStpPin, LOW);

	digitalWrite(stepperDomeDirPin, LOW);
	digitalWrite(stepperValveDirPin, LOW);

	digitalWrite(stepperDomeSlpPin, LOW);
	digitalWrite(stepperValveSlpPin, LOW);

	digitalWrite(rgbLedBlue, LOW);
	digitalWrite(rgbLedRed, LOW);
	digitalWrite(rgbLedGreen, LOW);

	// power management
	esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);


	// setup serial messages
	Serial.println("*S*KY*W*E*T");
	Serial.println("Rain|)Bow");
	//Serial.println("Adafruit AM2320 Sensor...");

	// setup serial BLUETOOTH messages
	SerialBT.println("*S*KY*W*E*T");
	SerialBT.println("Rain|)Bow");

	// init turret conditions
	domeGoHome();
	

}


void loop()
{

	if (Serial.available() > 0)
	{
		stepperCase = Serial.read();
		inputCase();
	}

	if (SerialBT.available() > 0)
	{
		stepperCase = SerialBT.read();
		inputCase();
	}

	//state machine

	//interupt: if low battery from BMS -> state = lowPower
	//interupt: if pushbutton depressed -> state = program

	switch (state)
	{
		case sleep:
		{
			//shut down peripherals
			//set wake up interupts for:
				//hourly daytime -> state = solar
				//AM/PM schedule -> state = water
			//enter sleep mode

			break;
		}

		case solar:
		{
			//open thread for current tracker
			//run solar tracker program
			
			state = sleep;

			break;
		}

		case program:
		{
			//open thread for bluetooth
			//store incoming instructions in buffer
			//one second spray for each square
			//save full bed data
			//return error checker
			
			state = sleep;

			break;
		}

		case water:
		{
			//load correct instruction set for date and time
			//reference temperature and apply modfifier to watering durations
			//open thread for flow sensor
			//run spray program

			state = sleep;

			break;
		}

		case lowPower:
		{
			//close the valve
			//set LED to red
			//allow solar
			//prevent water until battery > 50%
				//>50% -> perform last spray cycle
			
			state = sleep;

			break;
		}
	}
}

void realTimeClock()
{

}
// M A I N   F U N C T I O N   ---- SOLAR POWER TRACKING
void solarPowerTracker()
{
	int peakInsolationSteps = 0;
	long delayTimer1 = 0;
	long delayTimer2 = 0;
	bool delayComplete1 = false;
	bool delayComplete2 = false;

	domeGoHome();			//set turret to zero (full ccw)
	
	delayTimer1 = millis();

	while (delayComplete1 == false)
	{
		if (delayTimer2 >= delayTimer1 + 2000)		//wait 2s for turret to complete rotation to zero
		{
			delayComplete1 = true;		//reset timers
			delayTimer1 = 0;
			delayTimer2 = 0;

			currentSenseVal1 = analogRead(currentSense);
			currentSenseVal2 = 0;		//set first reading value

			stepperDomeDirCW();		//dir cw

			for (int i = 1; i <= 200; i++)		//take 200 steps and measure current at each spot
			{
				for (int x = 0; x < 2; x++)		
				{
					stepperDomeOneStepHalfPeriod(10);
				}

				delayTimer1 = millis();	

				while (delayTimer2 == false)
				{
					if (delayTimer2 >= delayTimer1 + 20)			//wait for steps to complete
					{
						delayComplete2 = true;
						currentSenseVal2 = analogRead(currentSense);

						if (currentSenseVal1 < currentSenseVal2)		//if new reading is that previous, save position
						{
							currentSenseVal1 = currentSenseVal2;
							peakInsolationSteps = i * 2;
							delayTimer1 = 0;
							delayTimer2 = 0;
						}
					}

					else
					{
						delayTimer2 = millis();
					}
				}

				delayComplete2 = false;
				delayTimer1 = millis();
				delayTimer2 = 0;

				stepperDomeDirCCW();		//dir ccw
				for (int i = 0; i < 400 - peakInsolationSteps; i++)
				{
					stepperDomeOneStepHalfPeriod(10);
				}

				while (delayComplete2 == false)
				{
					if (delayTimer2 >= delayTimer1 + 2000)
					{
						delayComplete2 = true;
					}
					
					else
					{
						delayTimer2 = millis();
					}
				}
			}
		}

		else
		{
			delayTimer2 = millis();
		}
	}
}



// M A I N    F U N  C T I O N  --- STEPPER GO HOME
void stepperGoHome(byte x, byte y, byte z, byte s)											// x STEP, y DIR, z EN, s HALL
{
	
	digitalWrite(y, HIGH);																	// SET stepper CW
	digitalWrite(z, HIGH);																	// ENSURE STEPPER IS NOT IN SLEEP MODE

	while (digitalRead(s) == 1)																// if hallSensor is HIGH the stepper is NOT at HOME
	{
		digitalWrite(x, HIGH);
		delay(10);
		digitalWrite(x, LOW);
		delay(10);
	}
								
	digitalWrite(z, LOW);																	// put stepper back to sleep
	digitalWrite(y, LOW);																	// SET STEOPP BACK TO CCW

}
// S U B   F U N C T I O N S --- dome and valve go home
void domeGoHome()
{
	//digitalWrite(stepperDomeDirPin, HIGH);																				// HIGH IS CLOSEWISE!!!
	stepperGoHome(stepperDomeStpPin, stepperDomeDirPin, stepperDomeSlpPin, hallSensorDome);									// dome stepper go to home posisition
	//digitalWrite(stepperDomeDirPin, LOW);																					// LOW ON DOME DIR PIN MEANS CW MOVEMENT AND HIGHER VALUE for stepCountDome -- ALWAYS INCREMENT FROM HERE
	stepCountDome = 0;
	SerialBT.println("dome go home");																						// LOW IS COUNTERCLOCKWISE
}	
void valveGoHome()
{
	stepperGoHome(stepperValveStpPin, stepperValveDirPin, stepperValveSlpPin, hallSensorValve);
	digitalWrite(stepperValveDirPin, LOW);
	stepCountValve = 0;
	SerialBT.println("valve go home");
}


// M A I N    F U N  C T I O N  --- STEPPER ONE STEP
void stepperOneStepHalfPeriod(byte x, byte y, byte z, int *q, int h)														//x STEP, y DIR, z EN, q SPCNT, h halFRQ ----!!!!!!check POINTERS!?!??!?!?--------
{

	digitalWrite(z, HIGH);
	delay(1);																												// proBablay GeT rId of HTis!!?!?

	digitalWrite(x, HIGH);
	digitalWrite(rgbLedBlue, HIGH);
	//digitalWrite(rgbLedGreen, LOW);
	delay(h);
	digitalWrite(x, LOW);
	digitalWrite(rgbLedBlue, LOW);
	//digitalWrite(rgbLedGreen, HIGH);
	delay(h);

	if (digitalRead(y) == LOW)
	{
		*q--;
	}

	if (digitalRead(y) == HIGH)
	{
		*q++;																												// LOW ON DOME DIR PIN MEANS CW MOVEMENT AND HIGHER VALUE for stepCountDome
	}

}
// S U B   F U N C T I O N S --- dome and valve one step
void stepperDomeOneStepHalfPeriod(int hf)
{
	stepperOneStepHalfPeriod(stepperDomeStpPin, stepperDomeDirPin, stepperDomeSlpPin, &stepCountDome, hf);
}
void stepperValveOneStepHalfPeriod(int hf)
{
	stepperOneStepHalfPeriod(stepperValveStpPin, stepperValveDirPin, stepperValveSlpPin, &stepCountValve, hf);
}

void stepperDomeDirCW()
{
	Serial.println("set dome direction CW---> HIGH IS CLOCKWISE!!!");
	SerialBT.println("set direction CW---> HIGH IS CLOCKWISE!!!");
	digitalWrite(stepperDomeDirPin, HIGH);
}
void stepperDomeDirCCW()
{
	Serial.println("set dome direction CCW ---> LOW IS COUNTERCLOCKWISE");
	SerialBT.println("set direction CCW---> LOW IS COUNTERCLOCKWISE");
	digitalWrite(stepperDomeDirPin, LOW);
}

void toggleStepperValveDir()
{
	bool valveDir;

	valveDir = digitalRead(stepperValveDirPin);
	digitalWrite(stepperValveDirPin, !valveDir);

	if (valveDir == LOW)
	{
		Serial.println("set direction open");
		SerialBT.println("set direction open");
	}
	else
	{
		Serial.println("set direction close");
		SerialBT.println("set direction close");
	}
}

void valveStepperOneStep()
{
	stepperValveOneStepHalfPeriod(10);
	Serial.println(stepCountValve);
	SerialBT.println(stepCountValve);
	//digitalWrite(stepperValveEnPin, LOW);
}

void displaySolarVoltage()
{
	solarPanelVoltageVal = (float)analogRead(solarPanelVoltage);
	delay(1);
	solarPanelVoltageVal = (((solarPanelVoltageVal / 4096) * 3.3) * 4.2448);

	Serial.print("solar panel voltage: ");
	Serial.print(solarPanelVoltageVal);
	Serial.println("V");
	SerialBT.print("solar panel voltage: ");
	SerialBT.print(solarPanelVoltageVal);
	SerialBT.println("V");
}

void displaySolarCurrent()
{
	currentSenseVal1 = analogRead(currentSense);

	Serial.print("current val: ");
	Serial.println(currentSenseVal1);
	SerialBT.print("current val: ");
	SerialBT.println(currentSenseVal1);
}

void doPulseIn()
{
	//Pulse IN shit
	for (int i = 0; i < 5; i++)
	{
	duration += float(pulseIn(pulsePin, HIGH));
	}

	duration /= 5;
	SerialBT.println(duration);

}

// M A I N   F U N C T I O N --- inputCase statement
void inputCase()
{			// read the incoming byte:

		switch (stepperCase)
		{

			case '0':															// send dome stepper to home posistion
				domeGoHome();
				break;
				
			case '1':															// send vavle stepper to home posisiton
				valveGoHome();
				break;

			case 'a':
				//10 steps on dome stepper
				for (int i = 0; i < 10; i++)
				{
					stepperDomeOneStepHalfPeriod(10);
				}

				Serial.println(stepCountDome);
				SerialBT.println(stepCountDome);
				break;

			case 'b':
				// set dome stepper to CW ---> HIGH IS CLOSEWISE!!!
				stepperDomeDirCW();
				break;

			case 'c':
				// set dome stepper to CW ---> LOW IS COUNTER CLOCKWISE!!!
				stepperDomeDirCCW();
				break;

			case 'd':
				//one step on valveStepper
				valveStepperOneStep();
				break;

			case 'e':
				toggleStepperValveDir();
				break;

			case 'f':
				
				// panel shit
				displaySolarCurrent();
				displaySolarVoltage();
				break;

			case 'g':
				solarPowerTracker();
				break;

			case 'h':
				doPulseIn();
				break;

			case 's':
				//esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
				esp_deep_sleep_start();
				break;

			case 't':
				gettimeofday(&now, NULL);

				SerialBT.println(now.tv_sec);
				SerialBT.println(last);

				last = now.tv_sec;
				break;

		}
}

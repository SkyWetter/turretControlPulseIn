// * S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
//  esp32 control firmware
//  october 10, 2018

// *********   P R E P R O C E S S O R S
#include <Stepper.h>
#include <BluetoothSerial.h>

#define pulsePin 23

#define stepperDomeDirPin 19
#define stepperDomeStpPin 18

#define stepperValveDirPin 5
#define stepperValveStpPin 17

#define hallSensorDome 16
#define hallSensorValve 4

#define stepperValveEnPin 2
#define stepperDomeEnPin 15

#define wakeUpPushButton 13

#define currentSense A6
#define solarPanelVoltage A7


// ********    P R O T O T Y P E S
void inputCase();
void solarPowerTracker();

void stepperGoHome(byte x, byte y, byte z, byte zs);
void domeGoHome();
void valveGoHome();

void stepperOneStepHalfPeriod(byte x, byte y, byte z, int *q, int h);
void stepperDomeOneStepHalfPeriod(int hf);
void stepperValveOneStepHalfPeriod(int hf);


// ********   B L U E T O O T H 
BluetoothSerial SerialBT;
byte stepperCase;


// ********   S T E P P E R S  
int stepCountDome = 0;
int stepCountValve = 0;

byte hallSensorDomeVal;
byte hallSensorValveVal;


// ********   P U L S E   C O U N T E R
double duration;


// ********   P O W E R		
float currentSenseVal = 0;
float solarPanelVoltageVal;									// VALUE READ FROM GPIO 3   OR ADC7
float qI, qII, qIII, qIV;									// values for quadrant average 



void setup()
{

	// esp32 general related
	// bluetooth 
	Serial.begin(115200);
	SerialBT.begin("turret");								// RainBow is name for Bluetooth device


	pinMode(pulsePin, INPUT);								// pin to read pulse frequency										// init timers need for pulseCounters

															//MP6500 stepper controller
	pinMode(stepperDomeDirPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper DIRECTION
	pinMode(stepperDomeStpPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper STEP
	pinMode(stepperDomeEnPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control DOME stepper ENABLE

	pinMode(stepperValveDirPin, OUTPUT);					// OUTPUT pin setup for MP6500 to control VALVE stepper DIRECTION
	pinMode(stepperValveStpPin, OUTPUT);					// OUTPUT pin setup for MP6500 to control VALVE stepper STEP
	pinMode(stepperValveEnPin, OUTPUT);						// OUTPUT pin setup for MP6500 to control VALVE stepper ENABLE

	pinMode(hallSensorDome, INPUT);
	pinMode(hallSensorValve, INPUT);

	pinMode(wakeUpPushButton, INPUT);

	pinMode(currentSense, INPUT);
	pinMode(solarPanelVoltage, INPUT);

	digitalWrite(stepperDomeStpPin, LOW);
	digitalWrite(stepperValveStpPin, LOW);

	digitalWrite(stepperDomeDirPin, LOW);
	digitalWrite(stepperValveDirPin, LOW);

	digitalWrite(stepperDomeEnPin, LOW);
	digitalWrite(stepperValveEnPin, LOW);


	// setup serial messages
	Serial.println("*S*KY*W*E*T");
	Serial.println("Rain|)Bow");
	//Serial.println("Adafruit AM2320 Sensor...");

	// setup serial messages
	SerialBT.println("*S*KY*W*E*T");
	SerialBT.println("Rain|)Bow");

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

}

// M A I N   F U N C T I O N   ---- SOLAR POWER TRACKING
void solarPowerTracker()
{

	//init solarPowerTracker function
	currentSenseVal = 0;																	// value read from GPIO 34 or adc6
	domeGoHome();																			// send dome to 0 posisition and set domeDir to CW (increment)

	// enter main process of function -- a for loop of 100 steps is a nested in a for loop of 4 quadrants which gives us a full rotation
	for (int f = 0; f < 4; f++)
	{

		for (int i = 0; i < 100; i++)
		{
			stepperDomeOneStepHalfPeriod(25);
			currentSenseVal += (analogRead(currentSense)/4);								// read value from GPIO 34 or ADC6 to find Max Solar Power
			delay(10);																		// CHANGE THIS LATER TO NON FREEZING DELAY --->> LOOP WITH DELAY SHOULD RUN AT ~ 10Hz
		}

		// average each currentSense val
		switch (f)
		{

			case '0':
				qI = (currentSenseVal / 100);							// average currentSenseVal over a 90degree(100steps) quadrant I
				break;
			case '1':
				qII = (currentSenseVal / 100);							// average currentSenseVal over a 90degree(100steps) quadrant II
				break;
			case '2':
				qIII = (currentSenseVal / 100);							// average currentSenseVal over a 90degree(100steps) quadrant III
				break;
			case '3':
				qIV = (currentSenseVal / 100);							// average currentSenseVal over a 90degree(100steps) quadrant IV
				break;

		}

		currentSenseVal = 0;										// reset currentSenseVal to 0 before next switch statement loop

	}

	// evaluate data and goto quadrant
	if (qI > qII && qI > qIII && qI > qIV)							// goto 0 steps to q1
	{
		domeGoHome();
	}

	if (qII > qI && qII > qIII && qII > qIV)						// goto 100 steps to q2
	{
		domeGoHome();

		for (int i = 0; i < 100; i++)
		{
			stepperDomeOneStepHalfPeriod(10);
		}
	}

	if (qIII > qI && qII > qII && qIII > qIV)						// goto 200 steps to q3
	{
		domeGoHome();

		for (int i = 0; i < 200; i++)
		{
			stepperDomeOneStepHalfPeriod(10);
		}
	}

	if (qIV > qI && qIV > qII && qIV > qIII)						// goto 300 steps to q4
	{
		domeGoHome();

		for (int i = 0; i < 300; i++)
		{
			stepperDomeOneStepHalfPeriod(10);
		}
	}

}


// M A I N    F U N  C T I O N  --- STEPPER GO HOME
void stepperGoHome(byte x, byte y, byte z, byte s)			// x STEP, y DIR, z EN, s HALL
{
	
	digitalWrite(y, HIGH);									// SET stepper CCW ?!?!?!?!??!

	digitalWrite(z, HIGH);									// ENSURE STEPPER IS NOT IN SLEEP MODE

	while (digitalRead(s) == 1)								// if hallSensor is HIGH the stepper is NOT at HOME
	{
		digitalWrite(x, HIGH);
		delay(10);
		digitalWrite(x, LOW);
		delay(10);
	}
								
	digitalWrite(z, LOW);

}
// S U B   F U N C T I O N S --- dome and valve go home
void domeGoHome()
{
	stepperGoHome(stepperDomeStpPin, stepperDomeDirPin, stepperDomeEnPin, hallSensorDome);									// dome stepper go to home posisition
	digitalWrite(stepperDomeDirPin, LOW);																					// LOW ON DOME DIR PIN MEANS CW MOVEMENT AND HIGHER VALUE for stepCountDome -- ALWAYS INCREMENT FROM HERE
	stepCountDome = 0;
}
void valveGoHome()
{
	stepperGoHome(stepperValveStpPin, stepperValveDirPin, stepperValveEnPin, hallSensorValve);
	digitalWrite(stepperValveDirPin, LOW);
	stepCountValve = 0;
}


// M A I N    F U N  C T I O N  --- STEPPER ONE STEP
void stepperOneStepHalfPeriod(byte x, byte y, byte z, int *q, int h) //x STEP, y DIR, z EN, q SPCNT, h halFRQ ----!!!!!!check POINTERS!?!??!?!?--------
{

	digitalWrite(z, HIGH);
	delay(1);																												// proBablay GeT rId of HTis!!?!?

	digitalWrite(x, HIGH);
	delay(h);
	digitalWrite(x, LOW);
	delay(h);

	if (digitalRead(y) == HIGH)
	{
		*q--;
	}

	if (digitalRead(y) == LOW)
	{
		*q++;																												// LOW ON DOME DIR PIN MEANS CW MOVEMENT AND HIGHER VALUE for stepCountDome
	}

}
// S U B   F U N C T I O N S --- dome and valve one step
void stepperDomeOneStepHalfPeriod(int hf)
{
	stepperOneStepHalfPeriod(stepperDomeStpPin, stepperDomeDirPin, stepperDomeEnPin, &stepCountDome, hf);
}
void stepperValveOneStepHalfPeriod(int hf)
{
	stepperOneStepHalfPeriod(stepperValveStpPin, stepperValveDirPin, stepperValveEnPin, &stepCountValve, hf);
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

			// set dome stepper to CCW
			Serial.println("set direction CCW");
			SerialBT.println("set direction CCW");
			digitalWrite(stepperDomeDirPin, HIGH);
			break;

		case 'c':

			// set dome stepper to CW
			Serial.println("set direction CW");
			SerialBT.println("set direction CW");
			digitalWrite(stepperDomeDirPin, LOW);
			break;

		case 'd':

			//one step on valveStepper
			stepperValveOneStepHalfPeriod(10);
			Serial.println(stepCountValve);
			SerialBT.println(stepCountValve);
			//digitalWrite(stepperValveEnPin, LOW);
			break;

		case 'e':

			//set valve stepper to CLOSE.. THIS is HIGH on dir pin
			Serial.println("set direction close");
			SerialBT.println("set direction close");
			digitalWrite(stepperValveDirPin, HIGH);
			break;

		case 'f':
			//set valve stepper to OPEN.. THIS is LOW on dir pin
			Serial.println("set direction open");
			SerialBT.println("set direction open");
			digitalWrite(stepperValveDirPin, LOW);
			break;

		case 'g':

			//Pulse IN shit
			for (int i = 0; i < 5; i++)
			{
				duration += float(pulseIn(pulsePin, HIGH));
			}

			duration /= 5;
			SerialBT.println(duration);
			// panel shit
			currentSenseVal = analogRead(currentSense);
			solarPanelVoltageVal = analogRead(solarPanelVoltage);
			delay(5);
			Serial.print("current val: ");
			Serial.println(currentSenseVal);
			Serial.print("solar panel voltage: ");
			Serial.println(solarPanelVoltageVal);
			SerialBT.print("current val: ");
			SerialBT.println(currentSenseVal);
			SerialBT.print("solar panel voltage: ");
			SerialBT.println(solarPanelVoltageVal);
			break;

		case 'h':

			solarPowerTracker();
			break;

		}
}

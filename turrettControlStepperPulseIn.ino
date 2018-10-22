// * S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
//  esp32 control firmware
//  october 3, 2018


#include "pulseCounter.h"
#include "Adafruit_AM2320.h"
#include <Stepper.h>
#include <BluetoothSerial.h>

#define stepperDomeDirPin 19
#define stepperDomeStpPin 18

#define stepperValveDirPin 5
#define stepperValveStpPin 17

#define hallSensorDome 16
#define hallSensorValve 4

#define stepperValveEnPin 2
#define stepperDomeEnPin 15

#define pulsePin 23


// function prototypes
void delayNonStop(float t);

// ********   B L U E T O O T H 
BluetoothSerial SerialBT;
byte stepperCase;

// ********   T E M P / M O I S T   S E N S 0 R 
Adafruit_AM2320 am2320 = Adafruit_AM2320();

// ********   S T E P P E R S  
int stepCountDome = 0;
int stepCountValve = 0;

boolean domeHigh = 0;
boolean valveHigh = 0;

byte hallSensorDomeVal = 0;
byte hallSensorValveVal = 0;

// ********   P U L S E   C O U N T E R
// global variables
double duration;
double durationF;

// timer shit
hw_timer_t * timer = NULL;									// in order to configure the timer, we will need a pointer to a variable of type hw_timer_t, which we will later use in the Arduino setup function. ---> typedef struct hw_timer_s hw_timer_t;--> hw_timer_t * timerBegin(uint8_t timer, uint16_t divider, bool countUp);
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;		// need to declare a variable of type portMUX_TYPE, which we will use to take care of the synchronization between the main loop and the ISR, when modifying a shared variable. --->typedef struct  --> * - Uninitialized (invalid)*portMUX_FREE_VAL - Mux is free, can be locked by either CPU CORE_ID_PRO / CORE_ID_APP - Mux is locked to the particular core 	 * Any value other than portMUX_FREE_VAL, CORE_ID_PRO, CORE_ID_APP indicates corruption 	 * If mux is unlocked, count should be zero.* If mux is locked, count is non - zero & represents the number of recursive locks on the mux.


// variables for data proccessing functions
volatile byte state = LOW;
volatile byte state2 = LOW;
volatile byte state_tmr = 0;								// used in onTimer() function's case statement
volatile byte value_ready = 0;								// used in onTimer() function's case statement

// variables for non-stoppping delayss
float beforeTime = 0;
float afterTime = 0;


void setup() 
{

	// esp32 general related
	// bluetooth 
	Serial.begin(115200);
	SerialBT.begin("Rain|)Bow");							// RainBow is name for Bluetooth device

	// pulse counter relate
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

	digitalWrite(stepperDomeDirPin, LOW);
	digitalWrite(stepperValveDirPin, LOW);

	digitalWrite(stepperDomeEnPin, LOW);
	digitalWrite(stepperValveEnPin, LOW);

	// adafruit sensor  

	am2320.begin();

	// setup serial messages
	Serial.println("*S*KY*W*E*T");
	Serial.println("Rain|)Bow");
	Serial.println("Adafruit AM2320 Sensor...");

}


void loop() 
{
	
	if (Serial.available() > 0)
	{

		stepperCase = Serial.read();						// read the incoming byte:

		domeHigh = digitalRead(stepperDomeDirPin);
		valveHigh = digitalRead(stepperValveDirPin);

		duration = float(pulseIn(pin, HIGH));
		Serial.println(duration);
		durationF = (1 / (duration / 1000000));
		Serial.println(durationF);
		delay(100);


		switch (stepperCase)
		{

			case 'a':

			digitalWrite(stepperDomeEnPin, HIGH);
			delay(1);     

			for (int i = 0; i <10; i++)
			{    

				digitalWrite(stepperDomeStpPin, HIGH);
				delay(10);
				digitalWrite(stepperDomeStpPin, LOW);
				delay(10);
				Serial.println(stepCountDome);

				if (domeHigh)
				{

					stepCountDome++;

				}

				if (!domeHigh)
				{

					stepCountDome--;

				}
       
			}

			digitalWrite(stepperDomeEnPin, LOW); 
			break;

		case 'b':

			Serial.print("set direction forward");
			Serial.print("set direction forward");
			digitalWrite(stepperDomeDirPin, HIGH);
			break;

		case 'c':

			Serial.print("set direction backward");
			Serial.print("set direction backward");
			digitalWrite(stepperDomeDirPin, LOW);
			break;

		case 'd':

            digitalWrite(stepperValveEnPin, HIGH);
			delay(1);

			for (int i = 0; i <10; i++)
			{

				digitalWrite(stepperValveStpPin, HIGH);
				delay(30);
				digitalWrite(stepperValveStpPin, LOW);
				delay(30);
				Serial.println(stepCountValve);
				if (valveHigh)
				{

					stepCountValve++;

				}

				if (valveHigh == 0)
				{

					stepCountValve--;

				}

			}

		   digitalWrite(stepperValveEnPin, LOW);
		   break;

		case 'e':

			Serial.print("set direction forward");
			Serial.print("set direction forward");
			digitalWrite(stepperValveDirPin, HIGH);
			break;

		case 'f':

			Serial.print("set direction backward");
			Serial.print("set direction backward");
			digitalWrite(stepperValveDirPin, LOW);
			break;

		case 'g':

			digitalWrite(stepperValveEnPin, HIGH);
			delay(1);	

			digitalWrite(stepperValveStpPin, HIGH);
			delay(10);
			digitalWrite(stepperValveStpPin, LOW);
			delay(10);
			Serial.println(stepCountValve);
			break;

		case 'h':
 
			Serial.print("Temp: ");
			Serial.println(am2320.readTemperature());
			Serial.print("Hum: ");
			Serial.println(am2320.readHumidity());
			break;

		case 'i':

			Serial.print("zero step count valve val: ");
			stepCountValve = 0;
			break;
		
		case 'j':

			Serial.print("zero step count dome val: ");
			stepCountDome = 0;
			break;
		}

	}

}



void delayNonStop(float t)
{

	beforeTime = millis();
	afterTime = (beforeTime + t);

	while (beforeTime < afterTime)									//retain status for amount of TIME as defined before switch to next nibble of step sequence array
	{

		beforeTime = millis();
	
	}

}	

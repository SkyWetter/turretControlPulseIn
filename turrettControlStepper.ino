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
byte pulsePin = 23;											// pulse pin input
int16_t flow0Counter = 0;
int16_t Pulses = 0;
//int16_t x;

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
	SerialBT.begin("Rain|)Bow");							// RainBow is name for Bluetooth device

	// pulse counter relate
	pinMode(pulsePin, INPUT);								// pin to read pulse frequency
	initPcnt(pulsePin);										// init pulseCounter function
	initPcntTimers();										// init timers need for pulseCounters

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

		case 'j':

			digitalWrite(stepperValveEnPin, HIGH);
			delay(1);	

			digitalWrite(stepperValveStpPin, HIGH);
			delay(10);
			digitalWrite(stepperValveStpPin, LOW);
			delay(10);
			Serial.println(stepCountValve);

 
 '			Serial.print("Temp: ");
			Serial.println(am2320.readTemperature());
			Serial.print("Hum: ");
			Serial.println(am2320.readHumidity());
			break;

		case 'l':

			Serial.print("zero step count valve val: ");
			stepCountValve = 0;
			break;
		
		case 'm':

			Serial.print("zero step count dome val: ");
			stepCountDome = 0;
			break;
		}

	}

}


// (ISR) |) alarm timer - IRAM_ATTTR ensures this interrupt function will run from RAM and not slower FLASH
void IRAM_ATTR onTimer()
{

	portENTER_CRITICAL_ISR(&timerMux);								 // declare by calling the portENTER_CRITICAL_ISR and portExit_CRITICAL_ISR macros. They also both receive as input the address of the global portMUX_TYPE variable.

	switch (state_tmr)
	{

	case 0:
		pcnt_counter_clear(PCNT_TEST_UNIT);
		pcnt_counter_resume(PCNT_TEST_UNIT);
		value_ready = 0;
		state_tmr = 1;
		break;

	case 1:
		pcnt_counter_pause(PCNT_TEST_UNIT);							 // * @brief Pause PCNT counter of PCNT unit
		pcnt_get_counter_value(PCNT_TEST_UNIT, &flow0Counter);		 // * @brief Get pulse counter value  -->  esp_err_t pcnt_get_counter_value(pcnt_unit_t pcnt_unit, int16_t* count);
		Pulses = flow0Counter;
		flow0Counter = 0;
		value_ready = 1;
		state_tmr = 2;
		break;

	case 2:
		// if you have been thru all states reset state_tmr
		state_tmr = 0;
		break;

	default:
		break;
	}
	portEXIT_CRITICAL_ISR(&timerMux);

}

void initPcntTimers()
{

	// timer init
	timer = timerBegin(0, 80, true);								// init our timer with a call to the timerBegin function, which will return a pointer to a structure of type hw_timer_t, which is the one of the timer global variable we declared in the previous section hw_timer_t<--- the first value should be 0 to 3 reference 1 of 4 timers... second is divide by for prescaler. 3rd defines up or down counter (true == UP) ---> * timerBegin(uint8_t timer, uint16_t divider, bool countUp);  -->  Use 1st timer of 4 (counted from zero). Set 80 divider for prescaler 
	timerAttachInterrupt(timer, &onTimer, true);					// receives as input a pointer to the initialized timer, which we stored in our global variable, the address of the function that will handle the interrupt and a flag indicating if the interrupt to be generated is edge (true) or level (false). ---> void timerAttachInterrupt(hw_timer_t *timer, void(*fn)(void), bool edge);  -->  Attach onTimer function to our timer.
	timerAlarmWrite(timer, 200000, true);							// this function receives as first input the pointer to the timer, seconds; number of timer increments the counter recieves for the interrupt should be generated, and as third a flag indicating if the timer should automatically reload upon generating the interrupt. ---> void timerAlarmWrite(hw_timer_t *timer, uint64_t interruptAt, bool autoreload);  -->  Set alarm to call onTimer function every second (value in microseconds).  -->  Repeat the alarm (third parameter)
	timerAlarmEnable(timer);										//  finish our setup function by enabling the timer with a call to the timerAlarmEnable function, passing as input our timer variable. ---> void timerAlarmEnable(hw_timer_t *timer);

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

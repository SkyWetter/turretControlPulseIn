// * S * K * Y  |)  W * E * T *
//  -=-=-=-=-=-=-=-=-=-=-=-=-
//  esp32 control firmware
//  october 10, 2018


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





// ********   B L U E T O O T H 
BluetoothSerial SerialBT;
byte stepperCase;


// ********   S T E P P E R S  
int stepCountDome = 0;
int stepCountValve = 0;

byte hallSensorDomeVal = 0;
byte hallSensorValveVal = 0;

// ********   P U L S E   C O U N T E R
// global variables
double duration;
double durationF;



void setup()
{

	// esp32 general related
	// bluetooth 
	Serial.begin(115200);
	SerialBT.begin("turret");				// RainBow is name for Bluetooth device


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

	digitalWrite(stepperDomeStpPin, LOW);
	digitalWrite(stepperValveStpPin, LOW);

	digitalWrite(stepperDomeDirPin, LOW);
	digitalWrite(stepperValveDirPin, LOW);

	digitalWrite(stepperDomeEnPin, LOW);
	digitalWrite(stepperValveEnPin, LOW);

	// setup serial messages
	Serial.println("*S*KY*W*E*T");
	Serial.println("Rain|)Bow");
	Serial.println("Adafruit AM2320 Sensor...");

	// setup serial messages
	SerialBT.println("*S*KY*W*E*T");
	SerialBT.println("Rain|)Bow");

}


void loop()
{
	


	

	if (SerialBT.available() > 0)
	{

		stepperCase = SerialBT.read();						// read the incoming byte:


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
				//SerialBT.println(stepCountDome);

			}
			
			digitalWrite(stepperDomeEnPin, LOW);
			break;

		case 'b':

			Serial.print("set direction forward");
			SerialBT.print("set direction forward");
			digitalWrite(stepperDomeDirPin, HIGH);
			break;

		case 'c':

			Serial.print("set direction backward");
			SerialBT.print("set direction backward");
			digitalWrite(stepperDomeDirPin, LOW);
			break;

		case 'd':

			digitalWrite(stepperValveEnPin, HIGH);
			delay(1);

			digitalWrite(stepperValveStpPin, HIGH);
			delay(10);
			digitalWrite(stepperValveStpPin, LOW);
			delay(10);

			//digitalWrite(stepperValveEnPin, LOW);
			break;
			
		case 'e':

			Serial.print("set direction forward");
			SerialBT.print("set direction forward");
			digitalWrite(stepperValveDirPin, HIGH);
			break;

		case 'f':

			Serial.print("set direction backward");
			SerialBT.print("set direction backward");
			digitalWrite(stepperValveDirPin, LOW);
			break;

		case 'g':

			//Pulse IN shit
			duration = float(pulseIn(pulsePin, HIGH));
			//Serial.println(duration);
			durationF = (1 / (duration / 1000000));
			//Serial.println(durationF);
			SerialBT.println(durationF);

		}

	}

}




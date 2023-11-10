// Pulse generation system according to the configuration specified by the Gantt chart
// version 1.1 от 20.07.2020 


// Valve connection options. When controlling logical 0 - swap places
#define StateOn HIGH		// Turning on the valves with logical 1 (supplying 5 Volts to the pin)
#define StateOff LOW		// Turning off the valves with logical 0 (supplying 0 Volts to the pin)


void ValveStatus(); // Valve status display function

#include "Impulse.h"	// Pulse control library


// Determine the required valve pulse sequences according Gantt chart
// For pulses in units of ms, it is advisable to disable output to the console (otherwise it may cause delays)
// (Number of PIN, 	delay before start in ms,	pulse duration in ms,	pause between pulses in ms, 	quantity of pulses) 	
Impulse ValveImpulse[]={
				// Example test ptogram (testing the vehicle fuel system on a stand)
	           // Valve 1 - air cylinder. Pulses are supplied to 2 pins, start 10080ms after run, 1000ms pulse length, 1000ms pause between pulses, 100 pulses
 				Impulse(2,10080,1000,1000,100),

				// Benchmark pulse sequence 2
				// Pulses are supplied to 3rd pin, start 4000 ms after run, 20ms pulse length, 300ms pause between pulses, 150 pulses

 				Impulse(3,4000,20,300,150),

				// Blowing process
				// Pulses are supplied to 4rd pin, start 1300 ms after run, 6ms pulse length, 0ms pause between pulses, 1 pulse
				// Pulses are supplied to 4rd pin, start 65000 ms after run, 6ms pulse length, 0ms pause between pulses, 1 pulse
				Impulse(4,1300,6,0,1),
				Impulse(4,65000,6,0,1),
				
				//Pressure release
				// Pulses are supplied to 5th pin, start 10 ms after run, 3000ms pulse length, 0ms pause between pulses, 1 pulse
				// Pulses are supplied to 5th pin, start 65000 ms after run, 3000ms pulse length, 0ms pause between pulses, 1 pulse
				Impulse(5,10,3000,0,1),
				Impulse(5,65000,3000,0,1),
				
				//Filling 
				// Pulses are supplied to 6th pin, start 10 ms after run, 7000ms pulse length, 0ms pause between pulses, 1 pulse
				// Pulses are supplied to 6th pin, start 61000 ms after run, 15000ms pulse length, 0ms pause between pulses, 1 pulse
				//Impulse(6,3,10,0,1),
				Impulse(6,10,7000,0,1),
				Impulse(6,61000,15000,0,1),
				};
const byte ImpulseTypes=sizeof(ValveImpulse)/sizeof(Impulse);  //Number of pulse sequences

unsigned long maxCyclesDurations=200000; // Maximum duration of all cycles, in ms. If exceeded - stop


unsigned long cycleCount = 0;	
unsigned long cycleMillis = 0;	
			
const byte SensorsNumber=3;	// Number of sensors

// Designation of the pins for connecting the sensors (the range of 0..5 Volts corresponds to readings of 0..1024)
byte SENSORS[]={A0, A1, A2}; 			//Connecting sensors - pins A0, A1, A2
double SensorRatio[]= {10,100,100}; 	//Coefficients for converting sensor values (0..5 Volts) into real values
										//A value of 100 corresponds to SensorValue=100 when 5 volts are applied, 66 when 3.3V is applied, etc.
double SensorValue[]= {0,0,0}; 			//Current sensor readings

bool SensorIntoImpulse=false;			// mode for influencing the sensor value on the pulse duration, disabled by default
byte SensorIntoImpulse_Input=0;			// Which analog input will control
byte SensorIntoImpulse_Number=0;		// Which of the impulses will be controlled
int ImpulseTimeByDefault=50000;   		// Default pulse duration (unless sensor dependent)
int ImpulsePauseByDefault=50000;  		// Default pause between pulses (unless sensor dependent)
double SensorIntoImpulseX=15000; 		// Coefficient of influence of the sensor value on the pulse duration
										// the sensor value is taken  into account according SensorRatio value

// Designation of the servise pins 
const byte WORKING=13;	// Operation indication - pin 13 (built-in LED)
const byte RUN=12;		// "Run" button connection



// Time delays
int SensorsPause=1000;				// Частота опроса датчиков, мс
int RunButtonPause=100;				// Power button polling frequency, ms (protection against contact “rattling”)
int RunButtonWait=1000;				// Waiting duration for counting the number of clicks (counted from the first click), the desired action is not activated instantly, but only after the end of the period

// Варианты отобраения значений
bool ValvesConsole=false;		// Displaying valve positions in the console
bool ValvesDisplay=false;		// Displaying valve positions on display
bool SensorsConsole=false;		// Displaying sensors readings  in the console
bool SensorsDisplay=false;		// Displaying sensors readings  on display


// Timers used
unsigned long MainTimer=0;			// Cyclogram operation timer
unsigned long RunButtonTimer=0;		// "Run" button press timer
unsigned long RunButtonWaitTimer=0;	// Wait timer for counting when the "Run" button is pressed
unsigned long SensorsTimer=0;		// Sensor polling timer

bool RunButtonState1=true;		// "Run" button position
bool RunButtonState2=true;		// "Run" button previous position
bool ModeRunning=false;			// Mode
bool RunButtonWaiting=false;	// waiting to count the number of "Run" button clicks after the first click
byte RunButtonClicks=0;			// number of "Run" button clicks

				
void ValveStatus()				// Displaying valve positions in the console
{
	if (ValvesConsole)			// If console output is enabled
		{
			Serial.print("T=");
			Serial.print(millis()-MainTimer);
			Serial.print("; Valves:[");
			for (byte i=0; i<ImpulseTypes; i++)
			{
				Serial.print(" : ");		
				Serial.print(ValveImpulse[i].State);	
			}
			Serial.println("]");	
		}		
		if (ValvesDisplay)		// If display output is enabled
		{	
			//
			//	
			// 		Place for code to display valve position on screen
			//
			//
			//
		}
	
}


void SensorsRead()	// Sensor polling
{
	for (byte SensorsCount=0; SensorsCount<SensorsNumber; SensorsCount++)
	{
		// Converting voltage values on the pin (0..5 Volts) into real readings
		SensorValue[SensorsCount]=analogRead(SENSORS[SensorsCount])*SensorRatio[SensorsCount]/1024;
		
		if (SensorsConsole)		// If output of sensor values to the console is enabled
		{
			Serial.print("[Sensor");
			Serial.print(SensorsCount+1);
			Serial.print("=");
			Serial.print(SensorValue[SensorsCount]);
			Serial.print("]");	
		}		
		if (SensorsDisplay)		// If displaying sensor values on display is enabled
		{
			//
			//
			//
			// Place for code to display sensor values on screen
			//
			//
			//
			
		}
	}
	if (SensorsConsole) Serial.println("");
	if (SensorIntoImpulse)		// If pulse dependence on analog input is enabled
	{
		// Scaling the pulse duration:
		ValveImpulse[SensorIntoImpulse_Number].Time=SensorValue[SensorIntoImpulse_Input]*SensorIntoImpulseX;	
		
		// Scaling pause between the pulses:
		ValveImpulse[SensorIntoImpulse_Number].Pause=SensorValue[SensorIntoImpulse_Input]*SensorIntoImpulseX;	
		
		if (SensorsConsole)
		{
			Serial.print("Impulse[");
			Serial.print(SensorIntoImpulse_Number);
			Serial.print("] duration=");			
			Serial.println(ValveImpulse[SensorIntoImpulse_Number].Time);
		}
	}
}

void setup() {
	Serial.begin(9600);		// Serial port debugging - speed 9600
	// Configuring valve and indicator pins as outputs
	for (byte i=0; i<ImpulseTypes; i++)
	{
		pinMode(ValveImpulse[i].ValvePin, OUTPUT);    	 // Each valve control pin as output
	}
	// Configuring pins of sensors and buttons as inputs
	for (byte SensorsCount=0; SensorsCount<SensorsNumber; SensorsCount++)
	{
		pinMode(SENSORS[SensorsCount], INPUT);    	 
	}
	pinMode(WORKING, OUTPUT);    	 // Working indicator as output
	pinMode(RUN, INPUT);   	 		 // "Run" button as input 
	digitalWrite(RUN,HIGH);          // Turn on the internal pull-up resistor for the button. Pressing the button must close the contact to ground
	digitalWrite(WORKING,LOW);
	Serial.println("Initialization - OK");	// Initializatoin finish message
	SensorsRead();		// Querying sensors 
	ValveStatus();		// Show current status 
}

bool Impulse1switchImpulse4=true; 
void RunButtonFunctions(byte clicks)	// "Run" button actions
{
	switch (clicks)
	{
	case 1:		// One click
		Serial.println("Clicks - 1");
		ModeRunning=!ModeRunning;	// Turn on/off main operating mode
			if (ModeRunning) 
			{
				Serial.println("RUN"); 
				cycleCount = 0;
				cycleMillis = millis();
				MainTimer=millis(); 	//Starting the main sequence timer
				digitalWrite(WORKING,HIGH);
				// Launch all pulse sequences
				for (byte i=0; i<ImpulseTypes; i++)
					{
						ValveImpulse[i].Begin();    	 // Launch each pulse sequences from query
					} 
				
			}
			else 
			{
				digitalWrite(WORKING,LOW);
				Serial.println("STOP after ");
				Serial.println(millis( )- cycleMillis);
				for (byte i=0; i<ImpulseTypes; i++)
					{
						ValveImpulse[i].Stop();    	 // Stop all pulse sequences
					} 
				MainTimer=0;
			}
	break;
	case 2:		// Action for two clicks
		// turning on and off the proportional dependence of the pulse duration of the first valve on the values ​​at the analog input 
		if (!SensorIntoImpulse)
		{
			
			// save the original values
			ImpulseTimeByDefault=ValveImpulse[SensorIntoImpulse_Number].Time;
			ImpulsePauseByDefault=ValveImpulse[SensorIntoImpulse_Number].Pause;
			Serial.println("Sensor value change impulse - ON");
			SensorIntoImpulse=true;		// Enable the pulse dependence on the analog input
		}
		else
		{
			// Return initial values
			ValveImpulse[SensorIntoImpulse_Number].Time=ImpulseTimeByDefault;
			ValveImpulse[SensorIntoImpulse_Number].Pause=ImpulsePauseByDefault;
			Serial.println("Sensor value change impulse - OFF");
			SensorIntoImpulse=false; 		// Disable the pulse dependence on the analog input
		}

	break;
	case 3:		// Action for three "Run" button clicks
	
		Serial.println("Clicks - 3");
				//
				// reserved for action for three "Run" button clicks
				//
	break;
	case 4:		// Action for four "Run" button clicks
		
		Serial.println("Clicks - 4");
				//
				// reserved for action for four "Run" button clicks
				//
	break;
	}
}
void loop() {
	// poll sensors every SensorsPause ms
	if ((millis()-SensorsTimer)>SensorsPause)
	{
		SensorsTimer=millis();
		SensorsRead();
		
	}
	
	// Example of valve control depending on sensors
	// ------------------------------------------------
	
	if (SensorValue[0]>100)	// If the readings of 1 sensor are more than 100
	{
		ValveImpulse[3].Stop();		// Stop pulse sequence #3
	}
	// Example of valve control depending on previous sensors
	if ((ValveImpulse[1].Finished)&&(Impulse1switchImpulse4))	// If  pulse sequence #1 is completed
	{
		Impulse1switchImpulse4=false;
		ValveImpulse[0].Begin();		// Run  pulse sequence #2 

	}
	
	// ---------------------------------------------------
	// poll the power button every RunButtonPause ms 
	if ((millis()-RunButtonTimer)>RunButtonPause)
	{
		RunButtonState1=RunButtonState2;
		RunButtonState2=digitalRead(RUN);
		RunButtonTimer=millis();
		if ((RunButtonState1)&&(!RunButtonState2)) 	//If the button changes state to pressed
		{
			if (!RunButtonWaiting)
			{
				RunButtonWaiting=true;			// start counting the clicks
				RunButtonWaitTimer=millis();	// start click waiting timer
				RunButtonClicks++;				// increment click counter
			}
			else
			{
				RunButtonClicks++;				// increment click counter
			}
		}
	}
	if ((millis()-RunButtonWaitTimer)>RunButtonWait)
	{
		RunButtonFunctions(RunButtonClicks);
		RunButtonClicks=0;
		RunButtonWaiting=false;
		RunButtonWaitTimer=0;
	}
	if (ModeRunning) // Main operational mode
	{
		cycleCount++;
		for (byte i=0; i<ImpulseTypes; i++)
		{
			ValveImpulse[i].Update();    	 // Processing all sequences
		}
		if ((millis( )- cycleMillis)>maxCyclesDurations) // Check if the maximum test duration timer has expired
		{
			ModeRunning=false;
			digitalWrite(WORKING,LOW);
			Serial.println("STOP by timeout after ");
			Serial.println(millis( )- cycleMillis);
			for (byte i=0; i<ImpulseTypes; i++)
				{
					ValveImpulse[i].Stop();    	 // Stop all pulse sequences
				} 
			MainTimer=0;
		}
	}
	
	
}
// Pulse generation system according to the configuration specified by the Gantt chart
// version 1.2 


// Valve connection options. When controlling logical 0 - swap places
#define StateOn HIGH		// Turning on the valves with logical 1 (supplying 5 Volts to the pin)
#define StateOff LOW		// Turning off the valves with logical 0 (supplying 0 Volts to the pin)


void ValveStatus(); // Valve status display function

#include "Impulse.h"	// Pulse control library
#include "SensorScaler.h"	// Sensor scaler control library


// Determine the required valve pulse sequences according Gantt chart
// For pulses in units of ms, it is advisable to disable output to the console (otherwise it may cause delays)
// (Number of PIN, 	delay before start in ms,	pulse duration in ms,	pause between pulses in ms, 	quantity of pulses) 	
Impulse ValveImpulse[]={
				// Example test ptogram (testing the vehicle fuel system on a stand)
				// Blowing process
				// Pulses are supplied to 4rd pin, start 1300 ms after run, 100ms pulse length, 0ms pause between pulses, 1 pulse
				// Pulses are supplied to 4rd pin, start 65000 ms after run, 100ms pulse length, 0ms pause between pulses, 1 pulse
				Impulse(4,	1300,	100),
				Impulse(4,	65000,	100),	           
			   
			   // Valve 1 - air cylinder. Pulses are supplied to 2 pins, start 6000ms after run, 1000ms pulse length, 500ms pause between pulses, 5 pulses
 				Impulse(2,	6000,	1000,	500,	5),

				// Benchmark pulse sequence 2


				
				//Pressure release
				// Pulses are supplied to 5th pin, start 10 ms after run, 3000ms pulse length, 0ms pause between pulses, 1 pulse
				// Pulses are supplied to 5th pin, start 65000 ms after run, 3000ms pulse length, 0ms pause between pulses, 1 pulse
				Impulse(5,	10,		3000,	0,		1),
				Impulse(5,	65000,	3000,	0,		1),
				
				//Filling 
				// Pulses are supplied to 6th pin, start 10 ms after run, 7000ms pulse length, 0ms pause between pulses, 1 pulse
				// Pulses are supplied to 6th pin, start 61000 ms after run, 15000ms pulse length, 0ms pause between pulses, 1 pulse
				//Impulse(6,3,10,0,1),
				
				Impulse(6,	10,		7000,	0,		1),
				Impulse(6,	61000,	15000,	0,		1),
				
				//Example of sequence triggered by other sequence
				// Pulses are supplied to 3rd pin, start 4000 ms after finishing pulse sequence #2, 20ms pulse length, 300ms pause between pulses, 15 pulses
 				Impulse(3,	4000,	100,		300,	15,	2),
				
				//Examples of sequence triggered by sensor
				// Pulses are supplied to 6th pin, 7000ms pulse length, 0ms pause between pulses, 1 pulse. Triggered by sensor #0 value (by default - exceeding 3V 			
				Impulse(5,	10,		7000,	0,		1,	-1,	0), 
				// Pulses are supplied to 6th pin, 7000ms pulse length, 0ms pause between pulses, 1 pulse. Triggered by sensor #1 value more than 2.5V (5V*1024/512) 	
				Impulse(6,	10,		7000,	0,		1,	-1,	1,	512),
				};
				
SensorScaler Scaler[]={	
	
				SensorScaler(3,	3, -1,	0, 0, 0, 3.33,	3.33)
				// scale pulse #3 by sensor #3 - no influence to delay, Pulse duration =100ms for sensor=0, Pulse pause = 300ms for sensor =0, no pulse delay scaling, scale pulse duration in three times when sensor value changed from 0 to 5V, scale pulse pause in three times when sensor value chnged from 0 to 5V
				
				// For default Impulse duration 100ms 
				// 1.5V -> 0+1.5/5*3.33*100=100ms
				// 3.0V -> 0+3.0/5*3.33*100=200ms
				// 4.5V -> 0+4.5/5*3.33*100=300ms	
				// 5.0V -> 0+5.0/5*3.33*100=330ms
				
				// For default Impulse pause 300ms 
				// 1.5V -> 0+1.5/5*3.33*300=300ms
				// 3.0V -> 0+3.0/5*3.33*300=600ms
				// 4.5V -> 0+4.5/5*3.33*300=900ms	
				// 5.0V -> 0+5.0/5*3.33*300=990ms
				};
const byte ImpulseTypes=sizeof(ValveImpulse)/sizeof(Impulse);  //Number of pulse sequences

unsigned long maxCyclesDurations=200000; // Maximum duration of all cycles, in ms. If exceeded - stop
byte SensorTriggeringLimit=820;    // Default Limit of triggering pulse sequence by sensor value

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
    for (byte i=0; i<ImpulseTypes; i++)    // check if any of impulse need to be triggered by sensor
    {
      if (ValveImpulse[i].TriggerSensor>=0 && 
        ValveImpulse[i].TriggerSensor<SensorsNumber && 
        ValveImpulse[i].TriggerSensor==SensorsCount && 
        analogRead(SENSORS[SensorsCount])>=ValveImpulse[i].TriggerLimit) 
      {
         ValveImpulse[i].Begin();   // start corresponding sequence    
      }
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
						if (ValveImpulse[i].TriggerPulse<0 && ValveImpulse[i].TriggerSensor<0)  //if sequence is not triggered by other sequence or sensor 
						  ValveImpulse[i].Begin();    	 
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
      if (ValveImpulse[i].TriggerPulse>=0 && ValveImpulse[i].TriggerPulse<ImpulseTypes)   // Does current pulse sequence triggered by previous sequence?
      {
        if (ValveImpulse[ValveImpulse[i].TriggerPulse].Finished)    // if triggering sequence finished
        {
             ValveImpulse[ValveImpulse[i].TriggerPulse].Begin();   // start corresponding sequence
        }
      }
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

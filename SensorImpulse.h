class Impulse
{
	public:
		byte ValvePin;
		unsigned long Start;		// Turn-on delay after start, ms
		unsigned long Time;			// Pulse duration, ms
		unsigned long Pause;		// Delay between pulses, ms
		unsigned int Number;					// Pulse number
		byte TriggerPulse;			// Number of previous pulse sequence to start current pulse sequence
		byte TriggerSensor;			// Number of sensor which trigger start of pulse sequence
		byte TriggerLimit;    		//Limit of triggering pulse sequence by sensor value
		int Count;					// Pulse counter
		bool Need=false;			// Are valve impulses required?
		bool Impulsing=false;		// Pulse mode 
		bool State=false;			// Current valve status
		bool On=false;				// Pulse mode (ON)
		bool Off=false;				// Pulse mode (OFF)
		bool Finished=false;		// The entire pulse sequence is complete
		int Valve2ImpulseCount=0;	// Valve pulse counter
		unsigned long CurrTimer;	// Pulse timer
		
	Impulse() 						// Default values
	{
		ValvePin=3;
		Start=0;					// Turn-on delay after start, ms
		Time=0;						// Pulse duration, ms
		Pause=0;					// Delay between pulses, ms
		Number=0;					// Pulse number
		Count=0;					// Pulse counter
		TriggerPulse=-1;
		TriggerSensor=-1;
		TriggerLimit=615;
	}
	Impulse(byte _ValvePin, unsigned long  _Start, unsigned long  _Time,unsigned long  _Pause=0, int _Number=1, byte _TriggerPulse=-1, byte _TriggerSensor=-1, byte _TriggerLimit=615)
	{
		ValvePin=_ValvePin;			// control pin
		Start=_Start*1000;			// delay after start, �s
		Time=_Time*1000;			// Pulse duration, �s
		Pause=_Pause*1000;			// Delay between pulses, �s
		Number=_Number;				// Pulse number
		TriggerPulse=_TriggerPulse;	// Number of previous pulse sequence to start current pulse sequence
		TriggerSensor=_TriggerSensor;		// Number of sensor which trigger start of pulse sequence
    TriggerLimit=_TriggerLimit;   // Sensor value to triger sequence: 1024=5V 615=3V
		Count=0;					// Pulse counter
	};
	void Begin()					//Start counting the pulse sequence
	{
		Count=0;
		Need=true;
		Finished=false;
		CurrTimer=micros()/1;
	};
	void Stop()						// Stop pulsing sequence
	{
		Impulsing=false;
		On=false;
		Off=false;
		State=false;
		Need=false;
		Finished=false;
		digitalWrite(ValvePin,StateOff);	// pin state changing
	};
	void Update()
	{
		if (Need)	// Is it necessary to pulse ?
		{
			if (!Impulsing)
			if (((micros()/1-CurrTimer))>Start)
			{
				Impulsing=true; 		// We switch to the pulse output mode
				CurrTimer=micros()/1;	// We start the pulse timer 
				On=true;				// pulse timer mode
				State=true;				// Turn on valve
				digitalWrite(ValvePin,StateOn);		//  pin state changing
				ValveStatus();			// Display the current position of the valves
			}
			if (On) 	//In the pulse mode of the timer, wait for the end of the pulse
			if 	((((micros()/1-CurrTimer))>Time)&&(Time>0))	//If the pulse time has expired
			{
				On=false;
				Off=true;		// next pulse waiting mode
				State=false;		// Turn off valve
				digitalWrite(ValvePin,StateOff);
				ValveStatus();			// Display the current position of the valves
				Count++;				// Increment pulse counter
				CurrTimer=micros()/1;	// Reset timer						
			}
			if (Off) 	// In the off pulse mode of the timer, we wait for the start of the next pulse
			if 	(((micros()/1-CurrTimer))>Pause)	//If the pulse time has expired
			{
				On=true;
				Off=false;				// next impulse
				State=true;				// turn on valve 
				digitalWrite(ValvePin,StateOn);		// // pin state changing
				ValveStatus();			// Display the current position of the valves	
				CurrTimer=micros()/1;	// Reset timer							
			}
			if (Count>=(Number))		// if pulse sequnece ended
			{
				// �������� ��� ������
				On=false;
				Off=false;
				Need=false;
				State=false;
				Finished=true;  
				digitalWrite(ValvePin,StateOff); 
				ValveStatus();			
			} 
			
		}
	};
};

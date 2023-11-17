#include "Impulse.h"	// Pulse control library
class SensorScaler
{
	public:
		bool Scaling;					// Is it requred to scale 
		byte SensorNumber;				// Which sensor will control pulse
		byte TargetPulse;				// Which pulse will be controlled
		long ZeroPulseDelay	// Which pulse delay corresponds to sensor value =0; Default value =-1 - do not change delay
		long ZeroPulseTime		// Which pulse duration corresponds to sensor value =0; Default value =-1 - do not change duration
		long ZeroPulsePause	// Which delay between pulses corresponds to sensor value=0; Default value =-1 - do not change delay
		double StartScaling;			// Scaling factor for delay value. value=2 doubled delay for sensor value 1024; Default value 0 - no scaling
		double TimeScaling;				// Scaling factor for pulse duration. value=2 doubled pulse duration for sensor value 1024; Default value 0 - no scaling )
		double PauseScaling;			// Scaling factor for pulse pause. value=2 doubled pulse duration for sensor value 1024; Default value 0 - no scaling )
		
		
		
	SensorScaler() 						// Default values
	{
		Scaling=true;					// Scaling required by default
		SensorNumber=-1;				// Which sensor will control
		TargetPulseNumber=-1;			// Which pulse will be controlled
		ZeroPulseDelay=-1;				// Which pulse delay corresponds to sensor value =0; Default value =-1 - do not change delay
		ZeroPulseTime=-1;				// Which pulse duration corresponds to sensor value =0; Default value =-1 - do not change duration
		ZeroPulsePause=-1;				// Which delay between pulses corresponds to sensor value=0; Default value =-1 - do not change delay
		DelayScaling=0;					// Scaling factor for delay value. value=2 doubled delay for sensor value 1024; Default value 0 - no scaling
		TimeScaling=0;					// Scaling factor for pulse duration. value=2 doubled pulse duration for sensor value 1024; Default value 0 - no scaling 1024)
		PauseScaling=0;					// Scaling factor for pulse pause. value=2 doubled pulse pause for sensor value 1024; Default value 0 - no scaling 1024)
	}
	SensorScaler(byte _SensorNumber, byte _TargetPulseNumber, double _StartScaling, double _TimeScaling, double _PauseScaling  long _ZeroPulseDelay=-1, long  _ZeroPulseTime=-1,  long  _ZeroPulsePause=-1)
	{
		Scaling=true;
		SensorNumber=_SensorNumber;				// Which sensor will control
		TargetPulseNumber=_TargetPulseNumber;			// Which pulse will be controlled
		ZeroPulseDelay=_ZeroPulseDelay;				// Which pulse delay corresponds to sensor value =0; Default value =-1 - do not change delay
		ZeroPulseTime=_ZeroPulseTime;				// Which pulse duration corresponds to sensor value =0; Default value =-1 - do not change duration
		ZeroPulsePause=_ZeroPulsePause;				// Which delay between pulses corresponds to sensor value=0; Default value =-1 - do not change delay
		DelayScaling=_StartScaling;			// Scaling factor for delay value. value=2 doubled delay for sensor value 1024; Default value 0 - no scaling
		TimeScaling=_TimeScaling;				// Scaling factor for pulse duration. value=2 doubled pulse duration for sensor value 1024; Default value 0 - no scaling 1024)
		PauseScaling=_PauseScaling;	
	};
	ScaleImpulse(byte Sensor, Impulse Target)
	{
		if (Scaling)
			int SensorValue=analogRead(Sensor[SensorNumber]);
			double StartFactor=1;
			double TimeFactor=1;
			double PauseFactor=1;
			if (Sensor==SensorNumber)
			{
				if ((StartScaling>=0)
					StartFactor=SensorValue/1024;
				else
					StartFactor=-(1-SensorValue/1024);
				if ((TimeScaling>=0)
					TimeFactor=SensorValue/1024;
				else
					TimeFactor=-(1-SensorValue/1024);	
				if ((PauseScaling>=0)
					PauseFactor=SensorValue/1024;
				else
					PauseFactor=-(1-SensorValue/1024);	
				
				if (StartScaling!=0)
					if (ZeroPulseDelay>=0) Target.Start=ZeroPulseDelay+Target.Start*(StartScaling*StartFactor); else Target.Start*=StartScaling*SensorValue/1024);
				if (TimeScaling!=0)
					if (ZeroPulseTime>=0) Target.Time=ZeroPulseTime+Target.Time*(TimeScaling*TimeFactor); else Target.Time*=TimeScaling*TimeFactor);
				if (PauseScaling!=0)
					if (ZeroPulsePause>=0) Target.Pause=ZeroPulsePause+Target.Pause*(PauseScaling*PauseFactor); else Target.Pause*=PauseScaling*PauseFactor);
			}
	}
	
};

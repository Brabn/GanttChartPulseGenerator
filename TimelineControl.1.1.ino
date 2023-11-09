// Управление клапанами по сигналам с датчиков
// версия 1.1 от 20.07.2020 
// (Добавлены альтернативные действия для нескольких нажатий кнопки 
// и пропорциональная зависимость интервала импульса от зачений аналогового входа

// Варианты подключения клапанов. При управлении логическим 0 - поменять местами
#define StateOn HIGH		// Включение клапанов логическим 1 (подача 5 Вольт на пин)
#define StateOff LOW		// Выключение клапанов логическим 0 (подача 0 Вольт на пин)


void ValveStatus(); // Функция отображения клапанов

#include "Impulse.h"	// Функционалуправления импулсами в отдельном файле (поместить его в ту-же папку)
// Определяем требуемые последовательности импульсов клапанов
// Для импульсов в единицы мс желательно отключать вывод в консоль (иначе это может вызывать задержки)
Impulse ValveImpulse[]={
	            //Клапан1 воздушний цилиндир. Импульсы подаются на 2 пин, начало через 2650 мс после пуска,2000 длина импульса,0 пауза между импулисами,
 				// количество 1шт
 				//Impulse(2,10000,10,0,0),
 				Impulse(2,10080,1000,1000,100),
				//Impulse(2,46500,15,0,1),
				// Еталон 2
 				//Impulse(3,4000,39,500,70),//390мс дорівнюе 1мл
 				Impulse(3,4000,20,300,150),
				//Impulse(3,4000,780,0,1),
				//Продувка 3
				Impulse(4,1300,6,0,1),
				Impulse(4,65000,6,0,1),
				//Сброс 4
				Impulse(5,10,3000,0,1),
				Impulse(5,65000,3000,0,1),
				//Заповнення 5
				//Impulse(6,3,10,0,1),
				Impulse(6,10,7000,0,1),
				Impulse(6,61000,15000,0,1),
				
				};
const byte ImpulseTypes=sizeof(ValveImpulse)/sizeof(Impulse);  // Количество последовательностей импульсов

unsigned long maxCyclesDurations=200000; // Максимальная длительность все циклов, в мс. При превышении - останов


unsigned long cycleCount = 0;	
unsigned long cycleMillis = 0;	
			
const byte SensorsNumber=3;	// Количество датчиков

// Обозначаем пины подключения датчиков (диапазон 0..5 Вольт соответсвует показаниям 0..1024)
byte SENSORS[]={A0, A1, A2}; 	//Подключение сенсоров - пины A0, A1, A2
double SensorRatio[]= {10,100,100}; //Коффициенты перевода значений датчиков (0..5 Вольт) в реальные значения
									// Значение 100 соответсвует SensorValue=100 при подаче 5 вольт, 66 - при 3,3В и т.д.
double SensorValue[]= {0,0,0}; //Текущие показания датчиков

bool SensorIntoImpulse=false;	// режим влияния значения датчика на длительность импульса, по умолчанию отключен
byte SensorIntoImpulse_Input=0;	// Какой из аналоговых входов будет управлять
byte SensorIntoImpulse_Number=0;	// Каким из импульсов будет управляться
int ImpulseTimeByDefault=50000;    // Длительность импульса по умолчанию (если не будет зависимости от датчика)
int ImpulsePauseByDefault=50000;    // Пауза между импульсами по умолчанию (если не будет зависимости от датчика)
double SensorIntoImpulseX=15000; 	// Коэффициент влияния значения датчика на длительность импульса 
								// значение датчика берется с учетом SensorRatio 

// Обозначаем служебыне пины
const byte WORKING=13;	// Индикация работы - пин 13 (встроенный светодиод)
const byte RUN=12;		// Подключение кнопки пуска



// Временные задержки
int SensorsPause=1000;		// Частота опроса датчиков, мс
int RunButtonPause=100;		// Частота опроса кнопки включения, мс (защита от "дребезжания" контакта)
int RunButtonWait=1000;		// Длительность ожидания для подсчета количества нажатий (отсчитывается с перовго нажатия), нужное действие включается не мгновенно, а только после конца периода

// Варианты отобраения значений
bool ValvesConsole=false;		// Вывод положения клапанов в консоль
bool ValvesDisplay=false;		// Вывод положения клапанов на дисплей
bool SensorsConsole=false;		// Вывод показаний датчиков в консоль
bool SensorsDisplay=false;		// Вывод показаний датчиков на дисплей


// Используемые таймеры
unsigned long MainTimer=0;	// Таймер работы циклограммы
unsigned long RunButtonTimer=0;	// Таймер нажатия кнопки включения
unsigned long RunButtonWaitTimer=0;	// Таймер ожидания ддля подсчетов нажатия кнопки включения

unsigned long SensorsTimer=0;	// Таймер опроса датчиков

bool RunButtonState1=true;		// Значение кнопки включения
bool RunButtonState2=true;		// Значение кнопки включения
bool ModeRunning=false;			// Режим работы
bool RunButtonWaiting=false;	// ожидания для подсчета количества нажатий после перовго нажатия
byte RunButtonClicks=0;			// Количество нажатий

				
void ValveStatus()	// Отобраение положения клапанов в консоли
{
	if (ValvesConsole)		// Если включен вывод в косноль
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
		if (ValvesDisplay)		// Если включен вывод на дисплей
		{
			// Место для кода вывода положения клапанов на экран
		}
	
}


void SensorsRead()	// Опрос датчиков
{
	for (byte SensorsCount=0; SensorsCount<SensorsNumber; SensorsCount++)
	{
		// Перевод значений напряжения на пине (0..5 Вольт) в реальные показания
		SensorValue[SensorsCount]=analogRead(SENSORS[SensorsCount])*SensorRatio[SensorsCount]/1024;
		
		if (SensorsConsole)		// Если включен вывод значений датчиков в консоль
		{
			Serial.print("[Sensor");
			Serial.print(SensorsCount+1);
			Serial.print("=");
			Serial.print(SensorValue[SensorsCount]);
			Serial.print("]");	
		}		
		if (SensorsDisplay)		// Если включен вывод значений датчиков на дисплей
		{
			// Место для кода вывода значений датчиков на дисплей
		}
	}
	if (SensorsConsole) Serial.println("");
	if (SensorIntoImpulse)		// Если включена зависимость импульса от аналогового входа
	{
		// Меняем длительность импульса:
		ValveImpulse[SensorIntoImpulse_Number].Time=SensorValue[SensorIntoImpulse_Input]*SensorIntoImpulseX;	
		
		// Меняем паузу между импульсами:	
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
	Serial.begin(9600);		// Отладка по серийному порту - скорость 9600
	// Конфигурируем пины клапанов и индикаторов как выходы
	for (byte i=0; i<ImpulseTypes; i++)
	{
		pinMode(ValveImpulse[i].ValvePin, OUTPUT);    	 // Контакт управления 1 клапаном как выход
	}
	// Конфигурируем пины датчиков и кнопок как входы
	for (byte SensorsCount=0; SensorsCount<SensorsNumber; SensorsCount++)
	{
		pinMode(SENSORS[SensorsCount], INPUT);    	 // Контакт управления 1 клапаном как выход
	}
	pinMode(WORKING, OUTPUT);    	 // Индикация работы как выход
	

	pinMode(RUN, INPUT);   	 		 // Контакт подключения кнопки как вход
	digitalWrite(RUN,HIGH);          //Включаем внутренний pull-up резистор для кнопки. Нажатие кнопки долно замыкать контакт на "землю"
	digitalWrite(WORKING,LOW);
	Serial.println("Initialization - OK");	// Сообщение что все инициализировалось нормально
	SensorsRead();		// Опрашиваем датчики первый раз
	ValveStatus();		// Отображаем текущее полоение клапанов
}

bool Impulse1switchImpulse4=true; 
void RunButtonFunctions(byte clicks)	//действия в зависимости от количества нажатий
{
	switch (clicks)
	{
	case 1:		// Для одного нажатия
		Serial.println("Clicks - 1");
		ModeRunning=!ModeRunning;	// Включаем/выключаем рабочий режим 
			if (ModeRunning) 
			{
				Serial.println("RUN"); 
				cycleCount = 0;
				cycleMillis = millis();
				MainTimer=millis(); 	// Запуск основного таймера циклограммы
				digitalWrite(WORKING,HIGH);
				// Запускаем все последовательности импульсов
				for (byte i=0; i<ImpulseTypes; i++)
					{
						ValveImpulse[i].Begin();    	 // Запускаем последовательности импульсов
					} 
				
			}
			else 
			{
				digitalWrite(WORKING,LOW);
				Serial.println("STOP after ");
				//Serial.println(cycleCount);
				Serial.println(millis( )- cycleMillis);
				for (byte i=0; i<ImpulseTypes; i++)
					{
						ValveImpulse[i].Stop();    	 // Останавливаем все последовательности импульсов
					} 
				MainTimer=0;
			}
	break;
	case 2:		// Действия для двух нажатий
		// вклюение и выключение прпорциональной зависимости длительности импульсов первого клапана от значений на аналоговом входе 
		if (!SensorIntoImpulse)
		{
			
			// Сохранем исходные значения
			ImpulseTimeByDefault=ValveImpulse[SensorIntoImpulse_Number].Time;
			ImpulsePauseByDefault=ValveImpulse[SensorIntoImpulse_Number].Pause;
			Serial.println("Sensor value change impulse - ON");
			SensorIntoImpulse=true;		// Включаем зависимость импульса от аналоговго входа
		}
		else
		{
			// Возвращаем исходные значения
			ValveImpulse[SensorIntoImpulse_Number].Time=ImpulseTimeByDefault;
			ValveImpulse[SensorIntoImpulse_Number].Pause=ImpulsePauseByDefault;
			Serial.println("Sensor value change impulse - OFF");
			SensorIntoImpulse=false; 		// Выключаем зависимость импульса от аналоговго входа
		}
		
		//Serial.println("Clicks - 2");
	break;
	case 3:		// Действия для трех нажатий
	
		Serial.println("Clicks - 3");
	break;
	case 4:		// Для четырех четырех нажатий
		
		Serial.println("Clicks - 4");
	break;
	}
}
void loop() {
	// Опрашиваем сенсоры каждые SensorsPause мс
	if ((millis()-SensorsTimer)>SensorsPause)
	{
		SensorsTimer=millis();
		SensorsRead();
		
	}
	
	// Пример управление клапанами в зависимости от датчиков
	if (SensorValue[0]>100)	// Если показания 1 датчика больше 100 
	{
		//ValveImpulse[3].Stop();		// Остановить последовательность №3 
	}
	// Пример управление клапанами в зависимости от предыдущих датчиков
	if ((ValveImpulse[1].Finished)&&(Impulse1switchImpulse4))	// Если 1 последовательность завершена
	{
		Impulse1switchImpulse4=false;
		//ValveImpulse[0].Begin();		// запустить последовательность №0

	}
	
	//Опрашиваем кнопку включения каждые RunButtonPause мс 
	if ((millis()-RunButtonTimer)>RunButtonPause)
	{
		RunButtonState1=RunButtonState2;
		RunButtonState2=digitalRead(RUN);
		RunButtonTimer=millis();
		if ((RunButtonState1)&&(!RunButtonState2)) 	// Если кнопка поменяла состояние на нажатое   
		{
			if (!RunButtonWaiting)
			{
				RunButtonWaiting=true;			// начинаем подсчет нажатий
				RunButtonWaitTimer=millis();	// запускаем таймер ожидания
				RunButtonClicks++;				// добавляем нажатие
			}
			else
			{
				RunButtonClicks++;				// добавляем нажатие
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
	if (ModeRunning) // Рабочий режим
	{
		cycleCount++;
		for (byte i=0; i<ImpulseTypes; i++)
		{
			ValveImpulse[i].Update();    	 // Обрабатываем все последовтельности
		}
		if ((millis( )- cycleMillis)>maxCyclesDurations) // Провереям, не истек ли таймер
		{
			ModeRunning=false;
			digitalWrite(WORKING,LOW);
			Serial.println("STOP by timeout after ");
			//Serial.println(cycleCount);
			Serial.println(millis( )- cycleMillis);
			for (byte i=0; i<ImpulseTypes; i++)
				{
					ValveImpulse[i].Stop();    	 // Останавливаем все последовательности импульсов
				} 
			MainTimer=0;
		}
	}
	
	
}

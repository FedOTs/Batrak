#include <Servo.h>

#include <PS2X_lib.h>

// Задаем пины для серв

//Ковш
#define SERV_BUCKET1       2
#define SERV_BUCKET2       3

//Стрела
#define SERV_ARROW1       4
#define SERV_ARROW2       5

// Вперед - назад
#define SERV_MOTOR1       6
#define SERV_MOTOR2       7

// ГАЗ
#define SERV_ACCEL       8
// Дроссельная заслонка
#define SERV_THROTTLE_VALVE       9

// Это углы сервы для управления рычагами ковша и стрелы
#define SERVO_CENTER  90
#define SERVO_MAX  130 
#define SERVO_MIN  50 

// Это углы сервы для иных устройств газ, дроссель, вперед, назад
#define SERVO_MAX_90  180 
#define SERVO_MIN_90  0 

//Шаговый мотор
#define STEP_PUL_EN 22   // Дает питание на плату с ШИМ
#define STEP_DIR 14   // Направление шаговика 
#define STEP_ENA 16   // Включение шаговика

//Выводы джойстика
#define PS2_DAT        A11
#define PS2_CMD        A12
#define PS2_SEL        A13
#define PS2_CLK        A14

//Фары, мигалки, звук
#define PIN_START_ENGINE   48  // Зажигание
#define PIN_BACK_LIGHT   49  // Фара заднего хода
#define PIN_SOUND        50  // Звуковой сигнал
#define PIN_LIGHT        51  // Фары
#define PIN_BLINK        52  // Габариты


//время в миллисекундах
#define TIME_STANDSTILL_MAX 30000 //через которое робот перейдёт в режим бездействия
#define TIME_STANDSTILLLONG_MAX 30000 //промежуток времени, через которое робот напоминает о бездействии
#define TIME_PAUSE_MAX  500  //на ожидание подтверждения (режим калибровки) 

//Режимы работы джойстика (раскомментировать нужное)
//- pressures = аналоговое считывание нажатия кнопок
//- rumble    = вибромоторы
#define pressures   true
//#define pressures   false
//#define rumble      true
#define rumble      false

Servo serv_bucket1;  // Ковш
Servo serv_bucket2;

Servo serv_arrow1; // Стрела
Servo serv_arrow2;

Servo serv_motor1; //Вперед назад
Servo serv_motor2;

Servo serv_accel;  // Газ
Servo serv_throttle_valve; // Дроссельная заслонка

//Создание класса для джойстика
PS2X ps2x;
int error = 0;
byte type = 0;
byte vibrate = 0;

unsigned long time_standstill, time_pause, time_standstill_long;
unsigned int cntServMotor1,cntServMotor2,cntServAccel,cntServThrottleValve; // Счетчики для серв
int stWheelDir, stWheelMove;  
int nJoyX, nJoyY;  // Счетчики для аналогов джойстика
bool fLight, fBlink, fSound, fStart; // Флаги для хранения состояния фар и габаритов

void setup() { 
  pinMode (STEP_PUL_EN, OUTPUT);
  pinMode (STEP_DIR, OUTPUT);
  pinMode (STEP_ENA, OUTPUT);
  pinMode (PIN_START_ENGINE, OUTPUT);
  pinMode (PIN_BACK_LIGHT, OUTPUT);
  pinMode (PIN_SOUND, OUTPUT);
  pinMode (PIN_LIGHT, OUTPUT);
  pinMode (PIN_BLINK, OUTPUT);
  digitalWrite(STEP_PUL_EN, HIGH);
  digitalWrite(STEP_DIR, HIGH);
  digitalWrite(STEP_ENA, LOW);
  digitalWrite(PIN_START_ENGINE, HIGH);
  digitalWrite(PIN_SOUND, HIGH);
  digitalWrite(PIN_BACK_LIGHT, HIGH);
  //digitalWrite(PIN_BLINK, HIGH);
  digitalWrite(PIN_LIGHT, HIGH);
  digitalWrite(PIN_BLINK, LOW);
  Serial.begin(57600);
  delay(300);
  //установка выводов и настроек: GamePad(clock, command, attention, data, Pressures?, Rumble?) проверка ошибок
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
 
  PrintError(error);

  type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
  case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
   }

  // Прирепляем пины к экземплярам класса Серво.
  serv_bucket1.attach(SERV_BUCKET1);
  serv_bucket2.attach(SERV_BUCKET2);
  serv_arrow1.attach(SERV_ARROW1);
  serv_arrow2.attach(SERV_ARROW2);
  serv_motor1.attach(SERV_MOTOR1);
  serv_motor2.attach(SERV_MOTOR2);
  serv_accel.attach(SERV_ACCEL);
  serv_throttle_valve.attach(SERV_THROTTLE_VALVE);

  // Центруем сервы (только стрела, ковш и вперед-назад)
  ServCenter();
  // Центруем отдельно сервы газа и дросселя
  cntServAccel = SERVO_CENTER;  
  cntServThrottleValve = SERVO_CENTER;

  // Выключенные фары и габариты
  fLight = false;
  fBlink = false;
  fSound = false;
  fStart = false;
  
  time_standstill = millis();
}

void loop()
{
  //Опрос джойстика
  ps2x.read_gamepad(false, vibrate); //считывание данных с джойстика и установка скорости вибрации


  if(ps2x.Button(PSB_L2)) {

 // Треугольник нажат (ковш вверх)
  if (ps2x.Button(PSB_TRIANGLE))
  {
    Serial.println("TRIANGLE");
    time_standstill = millis();
    serv_bucket1.write(SERVO_MAX);
    serv_bucket2.write(SERVO_MIN);
  }


  //Х нажат (ковш вниз)
  if (ps2x.Button(PSB_CROSS))
  {
    Serial.println("CROSS");
    time_standstill = millis();
    serv_bucket1.write(SERVO_MIN);
    serv_bucket2.write(SERVO_MAX);
  }

   //Круг нажат (стрела верх)
  if (ps2x.Button(PSB_CIRCLE))
  {
    Serial.println("PSB_CIRCLE");
    time_standstill = millis();
    serv_arrow1.write(SERVO_MAX);
    serv_arrow2.write(SERVO_MIN);
  }

     //Квадрат нажат (стрела вниз)
  if (ps2x.Button(PSB_SQUARE))
  {
    Serial.println("PSB_SQUARE");
    serv_arrow1.write(SERVO_MIN);
    serv_arrow2.write(SERVO_MAX);  
  }

  // Если нажата L2 только тогда слушаем аналоговый левый джойстик
  

    nJoyX = ps2x.Analog(PSS_LX); // read x-joystick
    nJoyY = ps2x.Analog(PSS_LY); // read y-joystick
    
    if (nJoyX < 125) 
      {
         StepRun(1, true);
         time_standstill = millis();
      }
    if (nJoyX > 130)
      {
        StepRun(-1, true);
        time_standstill = millis();
      }

   if ((nJoyX > 125) & (nJoyX < 130)) {
        StepRun(0, false);
       // time_standstill = millis();
    }
   if (nJoyY < 125) {
      time_standstill = millis();
     /** if (cntServMotor1 < SERVO_MAX_90-1) 
      {       
         serv_motor1.write(cntServMotor1);
         cntServMotor1++;
      }
      if (cntServMotor2 > SERVO_MIN_90+1) 
      {
         serv_motor2.write(cntServMotor2);
         cntServMotor2--;
      }*/

      serv_motor1.write(SERVO_MAX_90);
      serv_motor2.write(SERVO_MIN_90);
    } 

    if (nJoyY > 130) {
      time_standstill = millis();/*
      if (cntServMotor2 < SERVO_MAX_90-1) 
      {       
         serv_motor2.write(cntServMotor2);
         cntServMotor2++;
      }
      if (cntServMotor1 > SERVO_MIN_90+1) 
      {
         serv_motor1.write(cntServMotor1);
         cntServMotor1--;
      }
      */
      digitalWrite(PIN_BACK_LIGHT, LOW);
      serv_motor2.write(SERVO_MAX_90);
      serv_motor1.write(SERVO_MIN_90);
    } 

    if ((nJoyY) > 125 & (nJoyY) < 130) {
      ServCenter();     
      digitalWrite(PIN_BACK_LIGHT, HIGH);
      // time_standstill = millis(); 
    }
 
  
 // Газ увеличиваем
  if (ps2x.Button(PSB_PAD_UP))
    {
    Serial.println("PSB_PAD_UP");
    Serial.println(cntServAccel);
    time_standstill = millis();
    if (cntServAccel < SERVO_MAX_90-1) 
    {       
       serv_accel.write(cntServAccel);
       cntServAccel++;
    }
  }
 // Газ уменьшаем
  if (ps2x.Button(PSB_PAD_DOWN))
    {
    Serial.println("PSB_PAD_DOWN");
    Serial.println(cntServAccel);
    time_standstill = millis();
    if (cntServAccel > SERVO_MIN_90+1) 
    {
       serv_accel.write(cntServAccel);
       cntServAccel--;
    }
  }

  // Дроссель увеливаем
  if (ps2x.Button(PSB_PAD_LEFT))
    {
    Serial.println("PSB_PAD_LEFT");
    Serial.println(cntServThrottleValve);
    time_standstill = millis();
    if (cntServThrottleValve < SERVO_MAX_90-1) 
    {       
       serv_throttle_valve.write(cntServThrottleValve);
       cntServThrottleValve++;
    }
  }
 // Дроссель уменьшаем
  if (ps2x.Button(PSB_PAD_RIGHT))
    {
    Serial.println("PSB_PAD_RIGHT");
    Serial.println(cntServThrottleValve);
    time_standstill = millis();
    if (cntServThrottleValve > SERVO_MIN_90+1) 
    {
       serv_throttle_valve.write(cntServThrottleValve);
       cntServThrottleValve--;
    }
  }

   // Фары вкл - выкл
  if (ps2x.ButtonPressed(PSB_L1))
    {
    Serial.println("PSB_L1");
    time_standstill = millis();
    if (fLight == true) 
    {
       digitalWrite(PIN_LIGHT, LOW);
       fLight = false;
    } else 
    {
       digitalWrite(PIN_LIGHT, HIGH);
       fLight = true;
    }
  }

 /* // Габариты вкл - выкл
  if (ps2x.ButtonPressed(PSB_R1))
    {
    Serial.println("PSB_R1");
    time_standstill = millis();
    if (fBlink == true) 
    {
       digitalWrite(PIN_BLINK, LOW);
       fBlink = false;
    } else 
    {
       digitalWrite(PIN_BLINK, HIGH);
       fBlink = true;
    }
  }
*/
   // Звук
  if (ps2x.Button(PSB_R2))
  {  
    Serial.println("PSB_R2");
      time_standstill = millis();
    if (fSound == false) {
      digitalWrite(PIN_SOUND, LOW);
      fSound = true;
    }
  }

 // Заводим
  if(ps2x.Button(PSB_START)) 
  {
     Serial.println("PSB_START");
     time_standstill = millis();
     if (fStart == false) {
       digitalWrite(PIN_START_ENGINE, LOW);
       fStart = true;
     }
  }
  }
  else {
    //time_standstill = millis();
      digitalWrite(PIN_SOUND, LOW);
      StepRun(0, false);
      ServCenterMotor();
    }
   // Крестовина отпущена и не запущен режим калибровки

   Serial.print(millis());
   Serial.print("   ");
   Serial.println(time_standstill);

  if (fStart == true & ps2x.Button(PSB_START) == false) {
    if (millis() - time_standstill >= TIME_PAUSE_MAX) {
        fStart = false;
        digitalWrite(PIN_START_ENGINE, HIGH);
      }      
    }  

  if (fSound == true & ps2x.Button(PSB_R2) == false) {
    if (millis() - time_standstill >= TIME_PAUSE_MAX) {
        fSound = false;
        digitalWrite(PIN_SOUND, HIGH);
      }      
    }  

   if ((ps2x.Button(PSB_L2) == false)) {
    if ((nJoyX > 125) & (nJoyX < 130) & (nJoyY > 125) & (nJoyY < 130)) {
          StepRun(0, false);
          ServCenterMotor();
          digitalWrite(PIN_BACK_LIGHT, HIGH);
    }
     
      if ((ps2x.Button(PSB_PAD_UP) == false) & (ps2x.Button(PSB_PAD_DOWN) == false) &
          (ps2x.Button(PSB_PAD_LEFT) == false) & (ps2x.Button(PSB_PAD_RIGHT) == false)){
        //не нажата ни одна кнопка действия
        if ((ps2x.Button(PSB_TRIANGLE) == false) & (ps2x.Button(PSB_CROSS) == false) &
            (ps2x.Button(PSB_CIRCLE) == false) & (ps2x.Button(PSB_START) == false) &
            (ps2x.Button(PSB_R2) == false)  & (ps2x.Button(PSB_L1) == false) & (ps2x.Button(PSB_R1) == false))
        {
          ServCenter();
          digitalWrite(PIN_SOUND, HIGH);
          digitalWrite(PIN_BACK_LIGHT, HIGH);
          digitalWrite(PIN_START_ENGINE, HIGH);
          if (millis() - time_standstill >= TIME_STANDSTILL_MAX)  //бездействие
          {
            digitalWrite(PIN_SOUND, HIGH);
            digitalWrite(PIN_BACK_LIGHT, HIGH);
            digitalWrite(PIN_START_ENGINE, HIGH);
            digitalWrite(PIN_BACK_LIGHT, HIGH);
          
            if (millis() - time_standstill_long >= TIME_STANDSTILLLONG_MAX) //напоминание о бездействии
            {
              time_standstill_long = millis();
            }
          }
        }
      }
     }
      delay(50);
}

void PrintError(int error) {
  
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
  if (pressures)
    Serial.println("true ");
  else
    Serial.println("false");
  Serial.print("rumble = ");
  if (rumble)
    Serial.println("true)");
  else
    Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }  
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
}

void ServCenter()
{
  serv_bucket1.write(SERVO_CENTER);
  serv_bucket2.write(SERVO_CENTER);
  serv_arrow1.write(SERVO_CENTER);
  serv_arrow2.write(SERVO_CENTER);
  serv_motor1.write(SERVO_CENTER);
  serv_motor2.write(SERVO_CENTER);
}

void ServCenterMotor()
{
  serv_motor1.write(SERVO_CENTER);
  serv_motor2.write(SERVO_CENTER);
  cntServMotor1 = SERVO_CENTER;
  cntServMotor2 = SERVO_CENTER;
}


void StepRun(int dir, bool en) {
  if (en == true) {
    if (dir > 0) {
        digitalWrite(STEP_DIR, HIGH);
        digitalWrite(STEP_ENA, HIGH);
      }
    else {
        digitalWrite(STEP_DIR, LOW);
        digitalWrite(STEP_ENA, HIGH);
      }
  }
  else {
    digitalWrite(STEP_DIR, LOW);
    digitalWrite(STEP_ENA, LOW);
  }
}

// define a servo pulse function
void servoPulse(int pin, int angle)
{
  // convert angle to 500-2480 pulse width
  int pulseWidth = (angle * 11) + 500; 
  digitalWrite(pin, HIGH); // set the level of servo pin as high
  delayMicroseconds(pulseWidth); // delay microsecond of pulse width
  digitalWrite(pin, LOW); // set the level of servo pin as low
  delay(20 - pulseWidth / 1000);
}

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

#define SERVO_CENTER  90
#define SERVO_MAX  180 
#define SERVO_MIN  0 

//Шаговый мотор
#define STEP_PUL_EN 22
#define STEP_DIR 14
#define STEP_ENA 16

//Выводы джойстика
#define PS2_DAT        A11
#define PS2_CMD        A12
#define PS2_SEL        A13
#define PS2_CLK        A14

//Фары, мигалки, звук
#define PIN_LIGHT        24
#define PIN_BLINK        25
#define PIN_SOUND        26

//время в миллисекундах
#define TIME_STANDSTILL_MAX 30000 //через которое робот перейдёт в режим бездействия
#define TIME_STANDSTILLLONG_MAX 30000 //промежуток времени, через которое робот напоминает о бездействии
#define TIME_PAUSE_MAX  5000  //на ожидание подтверждения (режим калибровки) 

//Режимы работы джойстика (раскомментировать нужное)
//- pressures = аналоговое считывание нажатия кнопок
//- rumble    = вибромоторы
#define pressures   true
//#define pressures   false
//#define rumble      true
#define rumble      false

Servo serv_bucket1;
Servo serv_bucket2;

Servo serv_arrow1;
Servo serv_arrow2;

Servo serv_motor1;
Servo serv_motor2;

Servo serv_accel;
Servo serv_throttle_valve;

//Создание класса для джойстика
PS2X ps2x;
int error = 0;
byte type = 0;
byte vibrate = 0;

unsigned long time_standstill, time_pause, time_standstill_long;
unsigned int cntServ1,cntServ2, cntServ3,cntServ4, cntServ5,cntServ6, cntServ7,cntServ8; 
int stWheelDir, stWheelMove;

void setup() { 
  pinMode (STEP_PUL_EN, OUTPUT);
  pinMode (STEP_DIR, OUTPUT);
  pinMode (STEP_ENA, OUTPUT);
  digitalWrite(STEP_PUL_EN, HIGH);
  digitalWrite(STEP_DIR, HIGH);
  digitalWrite(STEP_ENA, LOW);
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
  
  serv_bucket1.attach(SERV_BUCKET1);
  serv_bucket2.attach(SERV_BUCKET2);
  serv_arrow1.attach(SERV_ARROW1);
  serv_arrow2.attach(SERV_ARROW2);
  serv_motor1.attach(SERV_MOTOR1);
  serv_motor2.attach(SERV_MOTOR2);
  serv_accel.attach(SERV_ACCEL);
  serv_throttle_valve.attach(SERV_THROTTLE_VALVE);
  
  ServCenter();
  cntServ7 = SERVO_CENTER;
  cntServ8 = SERVO_CENTER;
  time_standstill = millis();
}

void loop()
{
  //Опрос джойстика
  ps2x.read_gamepad(false, vibrate); //считывание данных с джойстика и установка скорости вибрации

 // Треугольник нажат (ковш вверх)
  if (ps2x.Button(PSB_TRIANGLE))
  {
    Serial.println("TRIANGLE");
    time_standstill = millis();
    serv_bucket1.write(SERVO_MAX);
    serv_bucket2.write(SERVO_MIN);
    /*if (cntServ1 < SERVO_MAX-1) 
    {       
       serv_bucket1.write(cntServ1);
       cntServ1++;
    }
    if (cntServ2 > SERVO_MIN+1) 
    {
       serv_bucket2.write(cntServ2);
       cntServ2--;
    }*/
  }


  //Х нажат (ковш вниз)
  if (ps2x.Button(PSB_CROSS))
  {
    Serial.println("CROSS");
    time_standstill = millis();
    serv_bucket1.write(SERVO_MIN);
    serv_bucket2.write(SERVO_MAX);
    /*if (cntServ1 > SERVO_MIN+1) 
    {
       serv_bucket1.write(cntServ1);
       cntServ1--;
    }
    if (cntServ2 < SERVO_MAX-1) 
    {
       serv_bucket2.write(cntServ2);
       cntServ2++;
    }*/
  }

   //Круг нажат (захват)
  if (ps2x.Button(PSB_CIRCLE))
  {
    Serial.println("PSB_CIRCLE");
    time_standstill = millis();
    serv_arrow1.write(SERVO_MAX);
    serv_arrow2.write(SERVO_MIN);
  }


   //Квадрат нажат (захват)
  if (ps2x.Button(PSB_SQUARE))
  {
    Serial.println("PSB_SQUARE");
    serv_arrow1.write(SERVO_MIN);
    serv_arrow2.write(SERVO_MAX);  
  }

      if (ps2x.Button(PSB_PAD_RIGHT) || ps2x.Button(PSB_PAD_LEFT))
      {
        if (ps2x.Button(PSB_PAD_RIGHT)) 
        {
            StepRun(1, true);
            time_standstill = millis();
        }
        else if (ps2x.Button(PSB_PAD_LEFT)) 
        {     
            StepRun(-1, true);
            time_standstill = millis();
        }
      }

      if (ps2x.Button(PSB_PAD_UP) || ps2x.Button(PSB_PAD_DOWN))
      {
        if (ps2x.Button(PSB_PAD_UP)) 
        {
            time_standstill = millis();
            if (cntServ1 < SERVO_MAX-1) 
            {       
               serv_bucket1.write(cntServ1);
               cntServ1++;
            }
            if (cntServ2 > SERVO_MIN+1) 
            {
       serv_bucket2.write(cntServ2);
       cntServ2--;
    }
        }
        else if (ps2x.Button(PSB_PAD_DOWN)) 
        {     
            StepRun(-1, true);
            time_standstill = millis();
        }
      }

  // ВВЕРХ нажато (движение вперёд)
  if (ps2x.Button(PSB_PAD_UP)){
    Serial.println("PSB_PAD_UP");
    serv_motor1.write(SERVO_MAX);
    serv_motor2.write(SERVO_MIN);
  }

 //ВНИЗ нажато (движение назад)
  if (ps2x.Button(PSB_PAD_DOWN))
    {
    Serial.println("PSB_PAD_DOWN");
    serv_motor1.write(SERVO_MIN);
    serv_motor2.write(SERVO_MAX);
  }

  if (ps2x.Button(PSB_R1))
    {
    Serial.println("PSB_R2");
    Serial.println(cntServ7);
    time_standstill = millis();
    if (cntServ7 < SERVO_MAX-1) 
    {       
       serv_accel.write(cntServ7);
       cntServ7++;
    }
  }

  if (ps2x.Button(PSB_L1))
    {
    Serial.println("PSB_L1");
    Serial.println(cntServ7);
    time_standstill = millis();
    if (cntServ7 > SERVO_MIN+1) 
    {
       serv_accel.write(cntServ7);
       cntServ7--;
    }
  }

  
  if (ps2x.Button(PSB_R2))
    {
    Serial.println("PSB_R2");
    Serial.println(cntServ8);
    time_standstill = millis();
    if (cntServ8 < SERVO_MAX-1) 
    {       
       serv_throttle_valve.write(cntServ8);
       cntServ8++;
    }
  }

  if (ps2x.Button(PSB_L2))
    {
    Serial.println("PSB_L2");
    Serial.println(cntServ8);
    time_standstill = millis();
    if (cntServ8 > SERVO_MIN+1) 
    {
       serv_throttle_valve.write(cntServ8);
       cntServ8--;
    }
  }
 
   // Крестовина отпущена и не запущен режим калибровки
      if ((ps2x.Button(PSB_PAD_UP) == false) & (ps2x.Button(PSB_PAD_DOWN) == false) &
          (ps2x.Button(PSB_PAD_LEFT) == false) & (ps2x.Button(PSB_PAD_RIGHT) == false)){

        StepRun(0, false);

        //не нажата ни одна кнопка действия
        if ((ps2x.Button(PSB_TRIANGLE) == false) & (ps2x.Button(PSB_CROSS) == false) &
            (ps2x.Button(PSB_CIRCLE) == false) & (ps2x.Button(PSB_SQUARE) == false))
        {
          ServCenter();
          if (millis() - time_standstill >= TIME_STANDSTILL_MAX)  //бездействие
          {

            if (millis() - time_standstill_long >= TIME_STANDSTILLLONG_MAX) //напоминание о бездействии
            {
              time_standstill_long = millis();
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
  cntServ1 = SERVO_CENTER;
  cntServ2 = SERVO_CENTER;
  cntServ3 = SERVO_CENTER;
  cntServ4 = SERVO_CENTER;
  cntServ5 = SERVO_CENTER;
  cntServ6 = SERVO_CENTER;
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

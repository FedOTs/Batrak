#include <Servo.h>

#include <PS2X_lib.h>

#define SERV1       2
#define SERV2       3

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

Servo myservo1;
Servo myservo2;

//Создание класса для джойстика
PS2X ps2x;
int error = 0;
byte type = 0;
byte vibrate = 0;

unsigned long time_standstill, time_pause, time_standstill_long;
unsigned int cntServ1,cntServ2; 
int stWheelDir, stWheelMove;

void setup() { 
  pinMode (STEP_PUL_EN, OUTPUT);
  pinMode (STEP_DIR, OUTPUT);
  pinMode (STEP_ENA, OUTPUT);
  digitalWrite(STEP_PUL_EN, HIGH);
  digitalWrite(STEP_DIR, HIGH);
  digitalWrite(STEP_ENA, LOW);
  
  //установка выводов и настроек: GamePad(clock, command, attention, data, Pressures?, Rumble?) проверка ошибок
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  Serial.begin(9600);
  PrintError(error);
  myservo1.attach(SERV1);
  myservo2.attach(SERV2);
  ServCenter();
  time_standstill = millis();
}

void loop()
{
  //Опрос джойстика
  ps2x.read_gamepad(false, vibrate); //считывание данных с джойстика и установка скорости вибрации

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

  
  // Треугольник нажат (ковш вверх)
  if (ps2x.Button(PSB_TRIANGLE))
  {
    Serial.println("TRIANGLE");
    Serial.println(cntServ1);
    Serial.println(cntServ2);
    time_standstill = millis();
    if (cntServ1 > SERVO_MIN && cntServ1 < SERVO_MAX) 
    {
       myservo1.write(cntServ1);
       cntServ1++;
    }
    if (cntServ2 > SERVO_MIN && cntServ2 < SERVO_MAX) 
    {
       myservo2.write(cntServ2);
       cntServ2--;
    }
  }


  //Х нажат (ковш вниз)
  if (ps2x.Button(PSB_CROSS))
  {
    Serial.println("CROSS");
    Serial.println(cntServ1);
    Serial.println(cntServ2);
    time_standstill = millis();
    if (cntServ1+1 > SERVO_MIN && cntServ1-1 < SERVO_MAX) 
    {
       myservo1.write(cntServ1);
       cntServ1--;
    }
    if (cntServ2+1 > SERVO_MIN && cntServ2-1 < SERVO_MAX) 
    {
       myservo2.write(cntServ2);
       cntServ2++;
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

          if (millis() - time_standstill >= TIME_STANDSTILL_MAX)  //бездействие
          {

            if (millis() - time_standstill_long >= TIME_STANDSTILLLONG_MAX) //напоминание о бездействии
            {
              time_standstill_long = millis();
            }
          }
        }
      }
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
  myservo1.write(SERVO_CENTER);
  myservo2.write(SERVO_CENTER);
  cntServ1 = SERVO_CENTER;
  cntServ2 = SERVO_CENTER;
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

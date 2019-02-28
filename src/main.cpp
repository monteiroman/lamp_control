#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <Time.h>

#define EST_PRES 0
#define EST_SET 1
#define EST_EXP 2
#define EST_STOP 3

#define DELAY_INI 3000
#define START_TIME 120000
#define MAX_SET_TIME 600000
#define MIN_SET_TIME 60000
#define STEP_TIME 10000

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int show;
uint16_t state;

class myTime {
    long set_time, start, count;
  public:
    myTime ();
    String getTimeSet ();
    String getCount ();
    String printTime (long);
    void setTime (long);
    void sumSetTime ();
    void subsSetTime ();
    void startCount ();
    void startCount (long);
    int countdown ();
} time;

myTime::myTime (){
  set_time = 0;
  start = 0;
  count = 0;
}

void myTime::setTime (long m_set_time){
  set_time = m_set_time;
}

String myTime::printTime (long t){
  int min = (int)(t/60000);
  long aux = t - 60000 * min;
  int sec = aux/1000;
  String s_min = String(min);
  String s_sec = String(sec);
  if(min < 10)
    s_min = "0" + s_min;
  if(sec < 10)
    s_sec = "0" + s_sec;
  return (s_min + ":" + s_sec);
}

String myTime::getTimeSet (){
  return printTime(set_time);
}

String myTime::getCount (){
  return printTime(count);
}

void myTime::sumSetTime (){
  if(set_time < MAX_SET_TIME)
    set_time+=STEP_TIME;
}

void myTime::subsSetTime (){
  if(set_time > MIN_SET_TIME)
    set_time-=STEP_TIME;
}

void myTime::startCount (long m_start){
  start = m_start;
}

int myTime::countdown (){
  long actual = millis();
  if(start){
    if((actual-start) < set_time){
      count = set_time-(actual-start);
      return 0;
    }else{
      return -1;
    }
  }
  return -2;
}

void setup(){
  int error;

  Serial.begin(115200);
  Serial.println("LCD...");

  while (! Serial);

  Serial.println("Dose: check for LCD");

  // See http://playground.arduino.cc/Main/I2cScanner
  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  Serial.print("Error: ");
  Serial.print(error);

  if (error == 0) {
    Serial.println(": LCD found.");
  } else {
    Serial.println(": LCD not found.");
  } // if

  lcd.begin(20, 4); // initialize the lcd
  show = 0;
  state = EST_PRES;
} // setup()


void PantallaInicio (){
  lcd.clear();
  lcd.setBacklight(255);
  lcd.setCursor(3, 0);
  lcd.print("**Insolador**");
  lcd.setCursor(4, 1);
  lcd.print("Version 0.1");
}

void PantallaSeteo (String time){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tiempo deseado");
  lcd.setCursor(15, 0);
  lcd.print(time);
  lcd.setCursor(0, 2);
  lcd.print("< INICIAR");
  lcd.setCursor(13, 2);
  lcd.print("+ 10s >");
  lcd.setCursor(13, 3);
  lcd.print("- 10s >");
}

void PantallaExp(String time){
  lcd.setCursor(5, 0);
  lcd.print("EXPONIENDO");
  lcd.setCursor(0, 2);
  lcd.print("< PARAR");
  lcd.setCursor(4, 3);
  lcd.print("t.restante");
  lcd.setCursor(15, 3);
  lcd.print(time);
}

void loop(){
  time.setTime(START_TIME);

  switch (state) {
    case EST_PRES:{
      PantallaInicio();
      delay(DELAY_INI);
      state = EST_SET;
      break;
    }
    case EST_SET:{
      String tm = time.getTimeSet();
      PantallaSeteo(tm);
      delay(4000);
      time.startCount(millis());
      lcd.clear();
      state = EST_EXP;
      break;
    }
    case EST_EXP:{
      if(time.countdown()==0){
        String tm = time.getCount();
        PantallaExp(tm);
      }else{
        if(time.countdown()==-1)
          state = EST_SET;
      }
      break;
    }
  }
}

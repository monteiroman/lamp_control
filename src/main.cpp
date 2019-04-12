#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <Time.h>
#include <Bounce2.h> // Used for "debouncing" the pushbutton

#define EST_PRES 0
#define EST_SET 1
#define EST_EXP 2
#define EST_STOP 3

#define DELAY_INI 3000
#define START_TIME 60000
#define MAX_SET_TIME 600000
#define MIN_SET_TIME 10000
#define STEP_TIME 10000
#define DEBOUNCE_TIME 10

LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int show;
uint16_t state;

const int startStopBtn = 4;
Bounce startStopBtnDeb = Bounce();
const int sumBtn = 6;
Bounce sumBtnDeb = Bounce();
const int subsBtn = 8;
Bounce subsBtnDeb = Bounce();
const int auxBtn = 10;
Bounce auxBtnDeb = Bounce();
const int relay1 = 11;
const int relay2 = 12;
int8_t screenFlag;


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
  this->set_time = 0;
  this->start = 0;
  this->count = 0;
}

void myTime::setTime (long m_set_time){
  this->set_time = m_set_time;
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
  return printTime(this->set_time);
}

String myTime::getCount (){
  return printTime(this->count);
}

void myTime::sumSetTime (){
  if(this->set_time < MAX_SET_TIME){
    this->set_time+=STEP_TIME;
  }
}

void myTime::subsSetTime (){
  if(this->set_time > MIN_SET_TIME){
    this->set_time-=STEP_TIME;
  }
}

void myTime::startCount (long m_start){
  this->start = m_start;
}

int myTime::countdown (){
  long actual = millis();
  if(this->start){
    if((actual-this->start) < this->set_time){
      count = this->set_time-(actual-this->start);
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

  pinMode(startStopBtn, INPUT);
  pinMode(sumBtn, INPUT);
  pinMode(subsBtn, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  // Setup pushbutton Bouncer object
  startStopBtnDeb.attach(startStopBtn);
  startStopBtnDeb.interval(DEBOUNCE_TIME);
  sumBtnDeb.attach(sumBtn);
  sumBtnDeb.interval(DEBOUNCE_TIME);
  subsBtnDeb.attach(subsBtn);
  subsBtnDeb.interval(DEBOUNCE_TIME);
  auxBtnDeb.attach(auxBtn);
  auxBtnDeb.interval(DEBOUNCE_TIME);

  screenFlag = 1;

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  time.setTime(START_TIME);
} // setup()


void PantallaInicio (){
  lcd.clear();
  lcd.setBacklight(255);
  lcd.setCursor(4, 1);
  lcd.print("Sweet Candy");
}

void PantallaSeteo (String time){
  lcd.setCursor(0, 0);
  lcd.print("< INICIAR");
  lcd.setCursor(14, 0);
  lcd.print("+10s >");
  lcd.setCursor(0, 2);
  lcd.print("Tiempo");
  lcd.setCursor(7, 2);
  lcd.print(time);
  lcd.setCursor(14, 3);
  lcd.print("-10s >");
}

void PantallaExp(String time){
  lcd.setCursor(0, 0);
  lcd.print("< PARAR");
  lcd.setCursor(5, 2);
  lcd.print("EXPONIENDO");
  lcd.setCursor(8, 3);
  lcd.print(time);
}

void loop(){
  startStopBtnDeb.update();
  sumBtnDeb.update();
  subsBtnDeb.update();
  auxBtnDeb.update();

  if(auxBtnDeb.rose()){
    digitalWrite(relay2, !digitalRead(relay2));
  }

  switch (state) {
    case EST_PRES:{
      PantallaInicio();
      delay(DELAY_INI);
      state = EST_SET;
      lcd.clear();
      break;
    }
    case EST_SET:{
      String tm = time.getTimeSet();

      if(screenFlag==1){
        PantallaSeteo(tm);
        screenFlag = 0;
      }
      if(sumBtnDeb.rose()){
        time.sumSetTime();
        screenFlag = 1;
      }
      if(subsBtnDeb.rose()){
        time.subsSetTime();
        screenFlag = 1;
      }
      if(startStopBtnDeb.rose()){
        time.startCount(millis());
        lcd.clear();
        state = EST_EXP;
        screenFlag = 1;
        digitalWrite(relay1, LOW);
        // digitalWrite(relay2, LOW);
      }
      break;
    }
    case EST_EXP:{
      if(time.countdown()==0){
        String tm = time.getCount();
        PantallaExp(tm);
      }
      if(time.countdown()==-1 || startStopBtnDeb.rose()){
          state = EST_SET;
          digitalWrite(relay1, HIGH);
          // digitalWrite(relay2, HIGH);
          time.setTime(START_TIME);
          lcd.clear();
        }
      break;
    }
  }
}

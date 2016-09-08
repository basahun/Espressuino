#include <LiquidCrystal.h>
#include <PID_v1.h>
#include <max6675.h>
#define RelayPin 11
#include <EEPROM.h>
#include <Gaggia.h>

int brew_time = 27;
int brew_temperature = 91;
int steam_temperature = 135;
int flush_temperature = 91;

bool pump_started;
bool HeatPhase;
bool Pause;
bool Coffee_Mode, Steam_Mode, Flush_Mode, Temp1_Mode, Temp2_Mode;
bool Menu_Button, Action_Button;

int state;

int thermoDO = 8;
int thermoCS = 9;
int thermoCLK = 10;
int PumpPIN = 12;


int WindowSize = 500;

bool pre_brew_finished;
int pre_brew_cycles = 6;
int pre_brew_cycle_duration = 200;
int pre_brew_duration = 6;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
Gaggia Gaggia(PumpPIN, RelayPin, WindowSize);

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

double Setpoint, Output, Input;
PID myPID(&Input, &Output, &Setpoint, 40, 2, 20, DIRECT);

unsigned long TemperatureTime;
unsigned long PhaseTime;
unsigned long PumpStartTime;
unsigned long progresstimer;
int PauseLength, HeatLength;
byte InputF;
byte Mode;

void setup() {
/*  if ( EEPROM.read(0) > 80 && EEPROM.read(0) < 110 ) {
    brew_temperature = EEPROM.read(0);
  }
  if ( EEPROM.read(1) > 110 && EEPROM.read(1) < 150 ) {
    steam_temperature = EEPROM.read(1);
  }
*/
  myPID.SetOutputLimits(0, WindowSize);
  myPID.SetMode(AUTOMATIC);
  //  mySerial.begin(9600);
  pinMode(PumpPIN, OUTPUT); digitalWrite(PumpPIN, LOW);
  pinMode(RelayPin, OUTPUT);
  lcd.begin(16, 2);
  pump_started = false;
  Mode = 1;

  lcd.setCursor(0, 0);
  lcd.print("CoffeeMaker 1.6 ");
  lcd.setCursor(0, 1);
  lcd.print("Kersz egy kavet?");
  delay(1000);
  lcd.clear();

  pre_brew_finished = false;
  state = 0;
  Serial.begin(9600);
}


void loop() {
  
  Action_Button = analogRead(0) > 800;
  Menu_Button = analogRead(1) > 800;

  Coffee_Mode = false;
  Steam_Mode = false;
  Flush_Mode = false;
  Temp1_Mode = false;


  switch ( Mode ) {
    case 1:
      Coffee_Mode = true;
      Setpoint = brew_temperature;
      break;
    case 2:
      Flush_Mode = true;
      Setpoint = flush_temperature;
      break;
    case 3:
      Steam_Mode = true;
      Setpoint = steam_temperature;
      break;
    case 4:
      Temp1_Mode = true;
      Setpoint = brew_temperature;
      break;
    case 5:
      Temp2_Mode = true;
      Setpoint = steam_temperature;
      break;



  }

  if (TemperatureTime < millis() - 250 ) {
    Input = thermocouple.readCelsius();
    InputF = Input;
  Serial.print(Input);
  Serial.print("\t");
  Serial.print(50*Output/WindowSize);
  Serial.print("\t");
  Serial.println(Setpoint);
    TemperatureTime = millis();
  }

  myPID.Compute();

  delay(10);
  Gaggia.Control(Output, brew_time);

  if ( state == 2 && ! pump_started && Coffee_Mode ) {
    pump_started = true;
    PumpStartTime = millis();
    digitalWrite(PumpPIN, HIGH);
    state = 3;

  }

  if ( Action_Button && Coffee_Mode && state == 0 ) {
    pump_started = true;
    PumpStartTime = millis();
    lcd.setCursor(4,1);
    lcd.print("[ Eloaztatas ]");
    for( int x = 1; x<= pre_brew_cycles; x++ ) {
      digitalWrite(PumpPIN, HIGH);
      delay(pre_brew_cycle_duration);

      digitalWrite(PumpPIN, LOW);
      delay(pre_brew_cycle_duration);
    }
  state = 1;
  }

  if ( pump_started && Coffee_Mode && state == 1 ) {
    if( millis() > PumpStartTime + 1000 * pre_brew_duration ) {
      lcd.setCursor(4,1);
      lcd.print("               ");
      pre_brew_finished = true;
      state = 2;
      pump_started = false;
    }
  }

  if ( pump_started && Coffee_Mode && state == 3  ) {
    lcd.setCursor(4, 1);
    lcd.print("[");
    for (int x = 1; x < (round(millis() - PumpStartTime) / ((brew_time - pre_brew_duration )* 100)) + 1; x++) {
      lcd.print((char)255);
    }
    lcd.setCursor(15, 1);
    lcd.print("]");

  }
  if ( millis() > PumpStartTime + (brew_time - pre_brew_duration) * 1000 && pump_started && Coffee_Mode && state == 3 ) {
    pump_started = false;
    pre_brew_finished = false;
    digitalWrite(PumpPIN, LOW);
    
    delay(3000);
    state = 0;
    lcd.clear();


  }

  if ( Menu_Button && ! pump_started ) {
    Mode = Mode + 1;
    if ( Mode > 6 ) {
      Mode = 1;
    }
    delay(300);
  }

  if ( Action_Button && Coffee_Mode && pump_started && millis() - PumpStartTime > 500 && state == 3) {
    pump_started = false;
    pre_brew_finished = false;
    digitalWrite(PumpPIN, LOW);

    lcd.setCursor(6, 1);
    lcd.print("  STOP  ");

    delay(1000);
    lcd.clear();
    lcd.setCursor(6, 1);
    lcd.print("        ");
    state = 0;
  }

  if ( Action_Button && Temp1_Mode ) {
    brew_temperature = brew_temperature + 1;
    if ( brew_temperature > 104 ) {
      brew_temperature = 85;
    }
    delay(250);
  }

  if ( Action_Button && Temp2_Mode ) {
    steam_temperature = steam_temperature + 1;
    if ( steam_temperature > 140 ) {
      steam_temperature = 120;
    }
    delay(250);
  }

  if ( Action_Button && Flush_Mode ) {
    digitalWrite(PumpPIN, HIGH);
    delay(2000);
    digitalWrite(PumpPIN, LOW);
    delay(1000);
    digitalWrite(PumpPIN, HIGH);
    delay(2000);
    digitalWrite(PumpPIN, LOW);
    delay(1000);
    digitalWrite(PumpPIN, HIGH);
    delay(2000);
    digitalWrite(PumpPIN, LOW);
    Mode = 2;
  }
  lcd.setCursor(0, 0);
  lcd.print("Homero:");
  lcd.print(InputF);
  lcd.print("/");
  lcd.print((int)Setpoint);
  lcd.print(" ");
  lcd.print((char)223);
  lcd.print("C ");
  lcd.setCursor(0, 1);
  switch ( Mode ) {
    case 1: lcd.print("Kave");
      break;
    case 2: lcd.print("Oblites");
      break;
    case 3: lcd.print("Goz    ");
      break;
    case 4: lcd.print("Kave Hom: "); lcd.print(brew_temperature); lcd.print(" ");
      break;
    case 5: lcd.print("Goz Hom: "); lcd.print(steam_temperature); lcd.print(" ");
      break;
    case 6: lcd.print("                ");
      EEPROM.write(0, brew_temperature);
      EEPROM.write(1, steam_temperature);
      Mode = 1;
      break;
  }
  lcd.setCursor(15, 1);
}

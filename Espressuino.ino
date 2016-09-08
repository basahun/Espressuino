#include <LiquidCrystal.h>
#include <PID_v1.h>
#include <max6675.h>
#define RelayPin 11
#include <EEPROM.h>

int brew_time = 27;
int brew_temperature = 91;
int steam_temperature = 135;
int flush_temperature = 91;

bool pump_started;
bool HeatPhase;
bool Pause;
bool Coffee_Mode,Steam_Mode,Flush_Mode,Temp1_Mode,Temp2_Mode;
bool Menu_Button,Action_Button;

int thermoDO = 8;
int thermoCS = 9;
int thermoCLK = 10;
int PumpPIN = 12;



MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

double Setpoint, Output, Input;
PID myPID(&Input, &Output, &Setpoint,200,20,2, DIRECT);
int WindowSize = 1500;
unsigned long TemperatureTime;
unsigned long PhaseTime;
unsigned long PumpStartTime;
unsigned long progresstimer;
int PauseLength,HeatLength;
byte InputF;
byte Mode;

void setup() {
  if( EEPROM.read(0) > 80 && EEPROM.read(0) < 110 ) { brew_temperature = EEPROM.read(0); }
  if( EEPROM.read(1) > 110 && EEPROM.read(1) < 150 ) { steam_temperature = EEPROM.read(1); }
  
  myPID.SetOutputLimits(0, 1000);
  myPID.SetMode(AUTOMATIC);
  Serial.begin(9600);
//  mySerial.begin(9600);
  pinMode(PumpPIN, OUTPUT); digitalWrite(PumpPIN, LOW);
  pinMode(RelayPin, OUTPUT);
  lcd.begin(16, 2);
  pump_started = false;
  Mode = 1;  
  HeatPhase = false;

  lcd.setCursor(0,0);
  lcd.print("CoffeeMaker 1.3 ");
  lcd.setCursor(0,1);
  lcd.print("Kersz egy kavet?");
  delay(1000);
  lcd.clear();
  
}

void loop() {

  Action_Button = analogRead(0) > 800;
  Menu_Button = analogRead(1) > 800;
  
  Coffee_Mode = false;
  Steam_Mode = false;
  Flush_Mode = false;
  Temp1_Mode = false;

   
  switch( Mode ) {
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
    
  if(TemperatureTime < millis() - 500 ) {
    Input = thermocouple.readCelsius();
    InputF = Input;
    Serial.print("PID ");
    Serial.print(Setpoint);   
    Serial.print(" ");
    Serial.print(Input);   
    Serial.print(" ");
    Serial.print(Output);   
    Serial.print(" ");
    Serial.print("2");   
    Serial.print(" ");
    Serial.print("5");   
    Serial.print(" ");
    Serial.print("1");   
    Serial.print(" ");
    Serial.println("Automatic");
//    Serial.println(analogRead(0));
//    Serial.println(analogRead(1));
        
//    mySerial.print("PID ");
//    mySerial.print(Setpoint);   
//    mySerial.print(" ");
//    mySerial.print(Input);   
//    mySerial.print(" ");
//    mySerial.print(Output);   
//    mySerial.print(" ");
//    mySerial.print("2");   
//    mySerial.print(" ");
//    mySerial.print("5");   
//    mySerial.print(" ");
//    mySerial.print("1");   
//    mySerial.print(" ");
//    mySerial.println("Automatic");
    

    TemperatureTime = millis();
  }
     
  myPID.Compute();

  if( Output > 100 && ! HeatPhase && ! Pause ) {
      PhaseTime = millis();
      digitalWrite(RelayPin,HIGH);
      HeatLength = Output;
      PauseLength = WindowSize - Output;
      HeatPhase = true;
  }

  if( HeatPhase && millis() > PhaseTime + HeatLength && ! Pause ) {
      PhaseTime = millis();
      digitalWrite(RelayPin,LOW);
      Pause = true;
      HeatPhase = false;
      
  }

  if( millis() > PhaseTime + PauseLength && Pause ) {
      Pause = false;
  }

  if( Action_Button && ! pump_started && Coffee_Mode ) {
    pump_started = true;    
    PumpStartTime = millis();
    digitalWrite(PumpPIN,HIGH);
    
  }

  if( pump_started && Coffee_Mode  ) {
        lcd.setCursor(4,1);
        lcd.print("[");
    for(int x=1;x<(round(millis()-PumpStartTime)/(brew_time*100))+1;x++) {
      lcd.print((char)255);
    }
      lcd.setCursor(15,1);
      lcd.print("]");

  }
  if( millis() > PumpStartTime+brew_time*1000 && pump_started && Coffee_Mode ) {
    pump_started = false;
    digitalWrite(PumpPIN,LOW);
    Serial.println("NO PUMP");
    delay(3000);
    lcd.clear();
  
    
  }

  if( Menu_Button && ! pump_started ) {
    Mode = Mode + 1;
    if( Mode > 6 ) { Mode = 1; }
    delay(300);
  }
  
  if( Action_Button && Coffee_Mode && pump_started && millis()-PumpStartTime > 500) {
    pump_started = false;
    digitalWrite(PumpPIN,LOW);
      
    lcd.setCursor(6,1);
    lcd.print("  STOP  ");
    
    delay(1000);
    lcd.clear();
    lcd.setCursor(6,1);
    lcd.print("        ");

  }

  if( Action_Button && Temp1_Mode ) {
    brew_temperature = brew_temperature + 1;
    if( brew_temperature > 104 ) { brew_temperature = 85; }
    delay(250);
  }

  if( Action_Button && Temp2_Mode ) {
    steam_temperature = steam_temperature + 1;
    if( steam_temperature > 140 ) { steam_temperature = 120; }
    delay(250);
  }

  if( Action_Button && Flush_Mode ) {
    digitalWrite(PumpPIN,HIGH);
    delay(2000);
    digitalWrite(PumpPIN,LOW);
    delay(1000);
    digitalWrite(PumpPIN,HIGH);
    delay(2000);
    digitalWrite(PumpPIN,LOW);
    delay(1000);
    digitalWrite(PumpPIN,HIGH);
    delay(2000);
    digitalWrite(PumpPIN,LOW);
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
    case 4: lcd.print("Kave Hom: ");lcd.print(brew_temperature);lcd.print(" ");
    break;
    case 5: lcd.print("Goz Hom: ");lcd.print(steam_temperature);lcd.print(" ");
    break;
    case 6: lcd.print("                ");
      EEPROM.write(0,brew_temperature);
      EEPROM.write(1,steam_temperature);
      Mode=1;
    break;
  }
  lcd.setCursor(15,1);
  if(Output > 0 ) { Serial.println(Output);}
}

#include <LiquidCrystal.h>

#define BUZZER 2       // 
#define POW 3          // power % pin
#define RELAY_PIN 4    // power polarity pin
#define FAN 5          // FAN pin
#define VBAT A6        // +12 battary 
#define VCC A7         // VCC 
#define lcdLen 8       // 8 for 16*1, 16 for 16*2 lcdt
#define BUTTON_COUNT 2 //
#define BYPASS 12 // relay

char ver[] = "v1.30";
char data[] ="12/09/22";
 
//CALIBRATING

float t0Corr = -2.8;  //8737
float t1Corr = -2.0;  // outwere temperature correction

float Tset = 28;      //  Temperature at start
boolean onlyHeating = 1;
boolean onlyCooling = 0;

// PID controller coefficients:
float kp = 10;     // in reality is feedback differential koefficient 
                   // should be more ki in 100 times for heating and in 30 times for cooling 
                    
float ki = 0.050;   // prorortional koefficient with summarity integration 
                   // for heating should be less that for cooling

float kd = 0;      // 


int deltaRegulatedTemp = 15; // border for Tset
float v0Corr = 0.067,v1Corr = 0.067;  // voltage measurement coefficient

// ALARMS SETTINGS
const int alarmCount = 3;
int alarm0[alarmCount] = {0,0,0}; //
int deltaT0[alarmCount] = {2, 6, 10}; //
int sumAlarm0[alarmCount] = {0,0,0};  //
int al0CyclesCount[alarmCount] = {32, 16, 4};// 

int alarm1[alarmCount] = {0,0,0}; //
int deltaT1[alarmCount] = {15, 25, 30}; //
int sumAlarm1[alarmCount] = {0,0,0};
int al1CyclesCount[alarmCount] = {128, 64, 32};//

int buttonPins[BUTTON_COUNT] = {A5, A4};
int button;   
boolean buttonUpLevel = HIGH;
String buttonLcdMsg[BUTTON_COUNT] = {"-----   ", "+++++   "};
int buttonChValue[2] = {-1, 1}; // temperatyre adjust step

int printPause = 1000; // have 6
int timeCycle = 2 * printPause; 
// adjusting values

int powerHisteresis = 1; // relay paramert
float byPassOnLevel = 0.95, byPassOffLevel = 0.85;

int fanMaxPower = 75, powMax = 250;    //  of 254 max
int poweForFanOn = 120;   // value output power for FAN on
int poweForFanOff = 70;   // value output power for FAN off
float powWaitCoef = 0.0;  // coeficient output power decriace when VCC(V1) < 6V,
float fanWaitCoef = 0.25; // coeficient FAN speed decriase when VCC < 6V;
float uLowCoef = 0.7;

// PID variables
float powE = 0, powE_1 = 0;
float En = 0, En_1 = 0, En_2 = 0;

//  object/ air/ unused/ temperature
float temp0 = 0, temp1 = 0;// ADC temprature var
int raw0, raw1, raw2;
float v0, v1;
int powerOutput = 0;
 
// serial device LCD pins
 const int rs = 11, en = 10, d4 = 9, d5 = 8, d6 = 7, d7 = 6;
// laboratory device LCD pins
//const int rs = 6, en = 7, d4 = 8, d5 = 9, d6 = 10, d7 = 11;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup() {
   
    pinMode( A0, INPUT );
    pinMode( A1, INPUT );
    pinMode(buttonPins[0],INPUT_PULLUP); 
    pinMode(buttonPins[1],INPUT_PULLUP); 
    //pinMode( A5, INPUT );
    
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(POW, OUTPUT);
    pinMode(FAN, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(BYPASS, OUTPUT);
    
    Serial.begin(9600);

    lcd.begin(lcdLen,2);
   
    lcd.print(ver);
    lcd.setCursor(0,1);
    lcd.print(data);
    delay(3000);

    lcd.clear();
    lcd.print(t0Corr,1);
    lcd.setCursor(0,1);
    lcd.print(t1Corr,1);
    delay(2000);
}
    
void loop() {
    analogWrite(POW, 0);
    delay(75);
    // Temperature measuring
    raw0 = analogRead(A0);
    temp0 = ( raw0*0.489)-273+t0Corr;
    raw1 = analogRead(A1);
    temp1 = ( raw1*0.489)-273+t1Corr;
   
    // Voltage parametrs
       
    bool V1On = v1 > 6;
    bool V0High = v0 > 9;
    
    delay(25);
    analogWrite(POW, abs(powerOutput));
        
    Serial.print("T0="); 
    Serial.print(temp0);    
    Serial.print("/ T1="); 
    Serial.print(temp1);
    Serial.print("/ Ts=");
    Serial.print(Tset,0);
    Serial.print("C");

    Serial.print("  A0:"); 
     for (int i = 0; i < alarmCount;i++){
         Serial.print(sumAlarm0[i]);
         Serial.print("/");  
     }
     Serial.print("  A1:");
     for (int i = 0; i < alarmCount;i++){
         Serial.print(sumAlarm1[i]);
         Serial.print("/");  
     }    
   //    
    Serial.print(" V0=");
    Serial.print(v0);
    Serial.print(" V1=");
    Serial.print(v1); 

    lcd.setCursor(0,0);
    lcd.print(temp0,0);
    lcd.print("C ");
    lcd.print(temp1,0);
    //lcd.print("/");
    //lcd.print(Tset,0);
    lcd.print("C  ");  

    lcd.setCursor(0,1);
    lcd.print(" B:");
    lcd.print(v0,1);
    lcd.print("V  ");
    delay(2*printPause);
    
    lcd.setCursor(0,1);
    lcd.print(" Z:");
    lcd.print(v1,1);
    lcd.print("V  ");
    delay(2*printPause);
    
    lcd.setCursor(0,1);
    lcd.print(" Ts=");
    lcd.print(Tset,1);
    lcd.print("C  ");
    delay(2*printPause);
    
    // save coefficints for recutent evaluation
    En_2 = En_1;
    En_1 = En;
    En = Tset - temp0;
    powE_1 = powE;
    float Ui = ki*En;
    float Up = kp*(En - En_1);
    float Ud = kd*(En-2*En_1+En_2);
    powE = powE_1 + Ui + Up + Ud;
 
    
    // at result powE = powMax or -powMax
    if (abs(powE) > powMax){
        powE = powMax - 2*powMax*(powE < 0);
    }
     if ((powE < 0)&&onlyHeating){
        powE = 0;
    }  
     if ((temp0 >= Tset)&&onlyHeating){
        powE = 0;
    }  
    powerOutput = int(powE*(1 - powWaitCoef*!V1On)*(1 - uLowCoef*!V0High));     

   // prepariang heating or cooling realay, use histeresis
    if(powE > powerHisteresis) {
      digitalWrite(RELAY_PIN, LOW);
    }
    if (powE < -powerHisteresis){
      digitalWrite(RELAY_PIN, HIGH);
    }
     
    int fanPower; 
  
    // FAN on/of
    
   // if ((powE > poweForFanOn)&&(powE < 0)){
   //    fanPower = int(fanMaxPower*(1 - fanWaitCoef*(v1 < 6)));
   //  }
    fanPower = fanMaxPower;
    if ((powE < poweForFanOff)&&(powE >= 0)){
          fanPower = 0; 
    }
    
    lcd.setCursor(0,1);
    lcd.print(" P:");
    lcd.print(powerOutput/2.5,0);
    lcd.print("%     ");
  
    Serial.print("/ P:");
    Serial.print(powerOutput/2.5,0);
    Serial.print("%");
    Serial.print("/ FAN:");
    Serial.print(fanPower/2.5,0);
    Serial.print("%");
    Serial.print("  In/Dif/Lin: ");
    Serial.print(Ui,2);
    Serial.print("/");
    Serial.print(Ud,1);
    Serial.print("/");
    Serial.print(Up,1);
      
     
    // OUTPUT POWER ON!!!
         
    analogWrite(POW, abs(powerOutput));      
    digitalWrite(FAN, fanPower);
    
    delay(250);
    
    // change bypass status 
    if (abs(powE) < powMax*byPassOffLevel){
       digitalWrite(BYPASS, LOW);
       Serial.print(" ByPass OFF ");
    } 
    else if (abs(powE) < powMax*byPassOnLevel){
        Serial.print(" ByPass FIXED");
    }  else {
        digitalWrite(BYPASS, HIGH*V1On);
        Serial.print(" ByPAss: ");
        Serial.print(HIGH*V1On); 
     }
     
    delay(timeCycle);  
    
    // voltage measuring
    v0 = ((analogRead(VBAT)*v0Corr));
    v1 = ((analogRead(VCC)*v1Corr));
  
    for(int i = 0; i < 100 ; button = i % BUTTON_COUNT){
      if (digitalRead(buttonPins[button])==!buttonUpLevel) {
        
        Tset = Tset + buttonChValue[button];
       
        Serial.println("Button pressed: "); 
        Serial.print(buttonLcdMsg[button]);
        
        lcd.setCursor(0,0);
        lcd.print(buttonLcdMsg[button]);
        lcd.setCursor(0,1);
        lcd.print(" Ts=");
        lcd.print(Tset,1);        
        
        digitalWrite(BUZZER, HIGH);
        delay(2*printPause);
        digitalWrite(BUZZER, LOW);

        lcd.clear();
        for (int i = 0; i < alarmCount;i++){
            lcd.print(sumAlarm0[i]);
            lcd.print("/");  
        }
        
        lcd.setCursor(0,1);
        for (int i = 0; i < alarmCount;i++){
            lcd.print(sumAlarm1[i]);
            lcd.print("/");  
        }
        delay(2*printPause);
        break;
      }
      i++;
   }
  /// ALARMS on Tset, Tset 
   
   if (abs(Tset - 18) > deltaRegulatedTemp){
        lcd.clear();
        lcd.print("Tset =");
        lcd.setCursor(0,1);
        lcd.print(Tset);
        tone(BUZZER, 1700);
        delay(2000);
        noTone(BUZZER);
      }
      
  /// ALARMS(0) on on temp0 
  
   for (int i = 0; i < alarmCount;i++){
        if (abs(temp0 - Tset) > deltaT0[i]){
              alarm0[i]++;             
            } 
            else {
              alarm0[i] = 0;
        }
        if (alarm0[i]>al0CyclesCount[i]){
          
            sumAlarm0[i]++;
            lcd.clear();
            lcd.print(temp0);
            tone(BUZZER, (i+1)*100);
            delay(150);
            noTone(BUZZER);    
        } 
    }
     
    delay(1*printPause);
/// ALARMS(1) on on temp0 and Tset

   for (int i = 0; i < alarmCount;i++){
        if (temp1 - Tset > deltaT1[i]){
              alarm1[i]++;             
            } 
            else {
              alarm1[i] = 0;
        }
        if (alarm1[i]>al1CyclesCount[i]){
            sumAlarm1[i]++;
            lcd.clear();
            lcd.print("    ");
            lcd.print(temp1);
            tone(BUZZER, (2*i+6)*100);
            
            delay(150);
            noTone(BUZZER);    
        } 
    }
                 
  // time out for power          
   delay(printPause);
   Serial.println("");
}

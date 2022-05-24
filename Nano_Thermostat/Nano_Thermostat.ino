#include <LiquidCrystal.h>

#define BUZZER 2       // 
#define POW 3          // power % pin
#define RELAY_PIN 4    // power polarity pin
#define FAN 5          // FAN pin
#define VBAT A6        // +12 battary 
#define VCC A7         
// VCC 
#define lcdLen 8       // 8 for 16*1, 16 for 16*2 lcdt

#define BUTTON_COUNT 2 //
int buttonPins[BUTTON_COUNT] = {A5, A4};
boolean buttonUpLevel = HIGH;
String buttonLcdMsg[BUTTON_COUNT] = {"-----", "+++++"};
int buttonChValue[2] = {-1, 1}; // temperatyre adjust step



int printPause = 500; // have 6
int timeCycle = 2 * printPause; 
// adjusting values
float tCorr = 0;  // temperature correction
float v0Corr = 0.067,v1Corr = 0.067;  // voltage measurement coefficient
int powerHisteresis = 4; // relay paramert

float temp_reg = 18;      //  Temperature at start
int fanMaxPower = 125, powMax = 250;    //  of 254 max
int poweForFanOn = 120;   // value output power for FAN on
int poweForFanOff = 70;   // value output power for FAN off
float powWaitCoef = 0.5;  // coeficient output power decriace when VCC(V1) < 6V,
float fanWaitCoef = 0.25; // coeficient FAN speed decriase when VCC < 6V;
float uLowCoef = 0.7;

// PID controller coefficients:
float kp = 50;     // proportional
float ki = 5;       // integral 
float kd = 0;       // differencial

// PID variables
float powe = 0, powe_1 = 0;
float En = 0, En_1 = 0, En_2 = 0;

//  object/ air/ unused/ temperature
float temp0 = 0, temp1 = 0;// ADC temprature var
int raw0, raw1, raw2;
float v0, v1;

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
    
    Serial.begin(9600);
    lcd.begin(lcdLen,2);
}
    
void loop() {
    
    // Temperature measuring
    raw0 = analogRead(A0);
    temp0 = ( raw0*0.489)-273+tCorr;
    raw1 = analogRead(A1);
    temp1 = ( raw1*0.489)-273+tCorr;
   
    // Voltage measuring
    v0 = ((analogRead(VBAT)*v0Corr));
    v1 = ((analogRead(VCC)*v1Corr));

    Serial.println("");   
    Serial.print("T0="); 
    Serial.print(temp0);    
    Serial.print("/ T1="); 
    Serial.print(temp1);
    Serial.print("/ Ts=");
    Serial.print(temp_reg,0);
    Serial.print("C");
        
    Serial.print("/ V0=");
    Serial.print(v0);
    Serial.print("/ V1=");
    Serial.print(v1); 
   
    lcd.setCursor(0,0);
    lcd.print(temp0,0);
    lcd.print("C ");
    lcd.print(temp1,0);
    //lcd.print("/");
    //lcd.print(temp_reg,0);
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
    lcd.print(temp_reg,1);
    lcd.print("C  ");
    delay(2*printPause);
    
    // save coefficints for recutent evaluation
    En_2 = En_1;
    En_1 = En;
    En = temp_reg - temp0;
    powe_1 = powe;
    float Ui = ki*En;
    float Up = kp*(En - En_1);
    float Ud = kd*(En-2*En_1+En_2);
    powe = powe_1 + Ui + Up + Ud;

 
    // heating or cooling use histeresis
    if(powe > powerHisteresis) {
      digitalWrite(RELAY_PIN, LOW);
    }
    if (powe < -powerHisteresis){
      digitalWrite(RELAY_PIN, HIGH);
    }
    
    // cut border of power value
    if (powe > powMax)
        powe = powMax;
    if (powe < -powMax)
        powe = -powMax;
        
    int powerOutput = int(powe*(1 - powWaitCoef*(v1 < 6))*(1 - uLowCoef*(v0<9)));     
    //int powerOutput = powe;
 
    int fanPower; 
    // FAN on/of
    if (abs(powe) > poweForFanOn){
       fanPower = int(fanMaxPower*(1 - fanWaitCoef*(v1 < 6)));
     }
    if (abs(powe) < poweForFanOff)                 
       fanPower = 0; 
    
    lcd.setCursor(0,1);
    lcd.print(" P:");
    lcd.print(powerOutput/2.5,0);
    lcd.print("%     ");
    //if (fanPower > 0)
    //    lcd.print("FAN");
    

    Serial.print("/ Power:");
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
    Serial.println(Up,1);
      
     
    // OUTPUT POWER ON!!!     
    analogWrite(POW, abs(powerOutput));      
    digitalWrite(FAN, fanPower);
    delay(timeCycle);  

    int button;   
    for(int i = 0; i < 100 ; button = i % BUTTON_COUNT){
      if (digitalRead(buttonPins[button])==!buttonUpLevel) {
     
        Serial.println("Button pressed: "); 
        Serial.print(buttonLcdMsg[button]);
        
        lcd.setCursor(0,0);
        lcd.print(analogRead(buttonPins[button]));
        lcd.print("Button: ");
        
        lcd.setCursor(0,1);
        lcd.print(buttonLcdMsg[button]);
        
        temp_reg = temp_reg + buttonChValue[button];
        digitalWrite(BUZZER, HIGH);
        delay(printPause);
        digitalWrite(BUZZER, LOW); 
        break;
      }
      i++;
   }
   
  // time out for power          
  // analogWrite(POW, 0);
  // tone(BUZZER,1000);
  // delay(500);
  // noTone(BUZZER);
}

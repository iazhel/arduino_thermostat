#include <LiquidCrystal.h>

#define BUZZER 2       // 
#define POW 3          // power % pin
#define RELAY_PIN 4    // power polarity pin
#define FAN 5          // FAN pin
#define BUTTON_COUNT 2 //

int buttonPins[BUTTON_COUNT] = {12, A3};
String buttonLcdMsg[BUTTON_COUNT] = {"-----", "+++++"};
int buttonChValue[2] = {-1, 1}; // temperatyre adjust step

// adjusting values
float tCorr = 0;  // temperature correction
float vCorr = 0.069;  // voltage measurement coefficient
int powerHisteresis = 4; // relay polarity reversal

float temp_reg = 18;      //  T at start
int fanMaxPower = 125, powMax = 250;    //  of 254 max 
float powWaitCoef = 0.5;  //  coef power decriace in the parking,
float fanWaitCoef = 0.5;  //  when VCC off (V1 < 8V)
int fanPower = 0;

// PID controller coefficients:
float kp = 50;     // proportional
float ki = 5;       // integral 
float kd = 0;       // differencial

// PID variables
float powe = 0, powe_1 = 0;
float En = 0, En_1 = 0, En_2 = 0;

//  object/ air/ unused/ temperature
float temp0 = 0, temp1 = 0, temp2 = 0; 
// ADC temprature var
int raw0, raw1, raw2;
float v0, v1;

// serial device LCD pins
// const int rs = 11, en = 10, d4 = 9, d5 = 8, d6 = 7, d7 = 6;
// laboratory device LCD pins
const int rs = 6, en = 7, d4 = 8, d5 = 9, d6 = 10, d7 = 11;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
   
    pinMode( A0, INPUT );
    pinMode( A1, INPUT );
    pinMode( A2, INPUT );
    pinMode(buttonPins[0],INPUT_PULLUP); 
    pinMode(buttonPins[1],INPUT_PULLUP); 
    //pinMode( A5, INPUT );

    
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(POW, OUTPUT);
    pinMode(FAN, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    
    Serial.begin(9600);
    lcd.begin(16,2);
}
    
void loop() {
       
    raw0 = analogRead(A0);
    temp0 = ( raw0*0.489)-273+tCorr;
   
    raw1 = analogRead(A1);
    temp1 = ( raw1*0.489)-273+tCorr;
       
    raw2 = analogRead(A2);
    temp2 = ( raw2*0.489)-273+tCorr;
    
    Serial.println("");   
    Serial.print("T0="); 
    Serial.print(temp0);    
    Serial.print("/ T1="); 
    Serial.print(temp1);
    Serial.print("/ Tset=");
    Serial.print(temp_reg,0);
    Serial.print("C");
    
    lcd.setCursor(0,0);
    lcd.print(temp0,1);
    lcd.print("/");
    lcd.print(temp1,0);
    lcd.print("/Y=");
    lcd.print(temp_reg,0);
    lcd.print("C  ");  
    
    // save coefficints for recutent evaluation
    En_2 = En_1;
    En_1 = En;
    En = temp_reg - temp0;
    powe_1 = powe;
    
    float Ui = ki*En;
    float Up = kp*(En - En_1);
    float Ud = kd*(En-2*En_1+En_2);
    powe = powe_1 + Ui + Up + Ud;

 
    // heat or cold. use histeresis
    if(powe > powerHisteresis) {
      digitalWrite(RELAY_PIN, LOW);
    }
    if (powe < -1* powerHisteresis){
      digitalWrite(RELAY_PIN, HIGH);
    }
    
    // cut border of power value
    if (powe > 250)
        powe = 250;
    if (powe < -250)
        powe = -250;
        
     int powerOutput = int(powe*(1 - powWaitCoef*(v1 < 8)));     
     //int powerOutput = powe;
     // on power     
    analogWrite(POW, abs(powerOutput));

    // FAN on/of
    if (abs(powe) > 200){
       fanPower = int(fanMaxPower*(1 - fanWaitCoef*(v1 < 8)));
     }
    if (abs(powe) < 100)                 
       fanPower = 0; 
       
    digitalWrite(FAN, fanPower);
         
    lcd.setCursor(0,1);
    lcd.print(powerOutput/2.5,0);
    lcd.print("% ");

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
            
    delay(500);
          
    // Voltage measuring
    v0 = ((analogRead(A5)*vCorr));
    v1 = ((analogRead(A6)*vCorr));
        
    Serial.print("/ V0=");
    Serial.print(v0);
    Serial.print("/ V1=");
    Serial.print(v1); 
    
    lcd.print(v0,1);
    lcd.print("V ");
    lcd.print(v1,1);
    lcd.print("V ");
           
    delay(2500); 
          
    // time out for power          
    analogWrite(POW, 0);
    delay(50);

    //    
   
    int button;   
    for(int i = 0; i < 100 ; button = i % BUTTON_COUNT){
      if (!digitalRead(buttonPins[button])) {
        
        Serial.println("Button pressed: "); 
        Serial.print(buttonLcdMsg[button]);
        
        lcd.setCursor(0,1);
        lcd.print(buttonLcdMsg[button]);
        
        temp_reg = temp_reg + buttonChValue[button];
        digitalWrite(BUZZER, HIGH);
        delay(250);
        digitalWrite(BUZZER, LOW); 
        break;
      }
      i++;
   }
  // tone(BUZZER,1000);
  // delay(500);
  // noTone(BUZZER);
}

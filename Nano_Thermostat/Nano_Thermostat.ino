#include <LiquidCrystal.h>

#define BUTTON_COUNT 2

#define POW 3       // power % pin
#define RELAY_PIN 4 // power polarity pin
#define FAN 5       // FAN pin


float temp_reg = 18; // TEMPERATURE WHAT WE NEED AT START
float tCorr = 0;   // Temperature adjusting value
float vCorr = 0; // Voltage adjucting value
int power_histeresis = 4;


int buttonPins[BUTTON_COUNT] = {A3, A4};
String buttonNames[BUTTON_COUNT] = {"---", "+++"};
int buttonValue[2] = {-1, 1}; // temperatyre adjust step

                    // PID controller coefficients:
float kp = 50;     // proportional
float ki = 5;       // integral 
float kd = 0;       // differencial

float temp0 = 0;    // var of temperaure what we need
float temp1 = 0;    // var of outwhwere temperature
float temp2 = 0;    // var of FAN radiator temperature

int raw0 = 0;   // real output temperature
int raw1 = 0;   // sensor1 temperature /FAN or cooler/
int raw2 = 0;   // sensor2 temperature /outwere air/

float powe = 0; //
float powe_1 = 0;
float En = 0;
float En_1 = 0;
float En_2 = 0;
// LiquidCrystal object_name(rs,rw,en,d4,d5,d6,d7)
//* LiquidCrystal lcd(RS, EN, DB4, DB5, DB6, DB7);
//LiquidCrystal lcd(6,7,8,9,10,11);
LiquidCrystal lcd(11,10,9,8,7,6);
void setup() {
   
    pinMode( A0, INPUT );
    pinMode( A1, INPUT );
    pinMode( A2, INPUT );
    pinMode(buttonPins[0],INPUT_PULLUP); //A3
    pinMode(buttonPins[1],INPUT_PULLUP); //A4
    pinMode( A5, INPUT );
//    pinMode( A6, INPUT );
    
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(POW, OUTPUT);
    pinMode(FAN, OUTPUT);

    Serial.begin(9600);
    lcd.begin(8,2);
}
    
void loop() {
    Serial.println("");   
    Serial.print("T0=");    
    raw0 = analogRead(A0);
    temp0 = ( raw0*0.489)-273+tCorr;
    Serial.print(temp0);    
    
    Serial.print("/       T1=");    
    raw1 = analogRead(A1);
    temp1 = ( raw1*0.489)-273+tCorr;
       
   // raw2 = analogRead(A2);
   // temp2 = ( raw2*0.489)-273+tCorr;

    Serial.print(temp1);
    Serial.print("/   Tset=");
    Serial.println(temp_reg,0);
    
    lcd.setCursor(0,0);
    lcd.print("");
    lcd.print(temp0,1);
    lcd.print("/");
    lcd.print(temp1,0);
    lcd.print("/");
    lcd.print(temp_reg,0);
    lcd.print("C");
    
    // save coefficints for recutent evaluation
    En_2 = En_1;
    En_1 = En;
    En = temp_reg - temp0;
    powe_1 = powe;
    
    float Ui = ki*En;
    float Up = kp*(En - En_1);
    float Ud = kd*(En-2*En_1+En_2);
    powe = powe_1 + Ui + Up + Ud;

    // cut border of power value
    if (powe > 250)
        powe = 250;
    if (powe < -250)
        powe = -250;
 
    // heat or cold. use histeresis
    if(powe > power_histeresis) {
      digitalWrite(RELAY_PIN, LOW);
    
      lcd.setCursor(0,1);
      lcd.print(" +:");
    }
    if (powe < -1* power_histeresis){
      digitalWrite(RELAY_PIN, HIGH);
      lcd.setCursor(0,1);
      lcd.print(" -");
    }

    lcd.print(powe/2.5,0);
    lcd.print("% ");

     // on power
    delay(50);      
    analogWrite(POW, abs(powe));
    
    Serial.print("    Power:");
    Serial.print(powe/2.5,0);
    Serial.print("%");
    Serial.print("  In/Dif/Lin: ");
    Serial.print(Ui,2);
    Serial.print("/");
    Serial.print(Ud,1);
    Serial.print("/");
    Serial.println(Up,1);
            
    // FAN on/of   
    if (abs(powe) > 200)
        digitalWrite(FAN, 128);
    if (abs(powe) < 100)                 
        digitalWrite(FAN, 0);
         
    delay(500);
          
    // Voltage measuring
    float v0 = ((analogRead(A5)*5*(11.7-vCorr))/(1023*2.10)) + 0.9;
    int ads1 = analogRead(A6);
    
    Serial.print("    V0=");
    Serial.println(v0);
    lcd.print(v0,1);
    lcd.print("V  ");

    
    Serial.print("A5: ");
    Serial.print(analogRead(A5));
    Serial.print("  A6: ");
    Serial.print(analogRead(A6));
    Serial.print("  A7: ");
    Serial.print(analogRead(A7));
    
    delay(2500); 
          
    // time out for power          
    analogWrite(POW, 0);
    delay(50);

    //    
    int button;   
    for(int i = 0; i < 100 ; button = i % BUTTON_COUNT){
      if (!digitalRead(buttonPins[button])) {
    
        Serial.print(buttonNames[button]);
        lcd.setCursor(12,1);
        lcd.print(buttonNames[button]);
        
        temp_reg = temp_reg + buttonValue[button];
        Serial.println(temp_reg);
        //delay(1000); 
        break;
      }
      i++;
  }
   
}

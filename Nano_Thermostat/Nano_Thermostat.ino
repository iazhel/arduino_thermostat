#include <LiquidCrystal.h>

#define BUZZER 2    // 
#define POW 3       // power % pin
#define RELAY_PIN 4 // power polarity pin
#define FAN 5       // FAN pin
#define BUTTON_COUNT 2

// adjusting values
float temp_reg = 18; // 
float tCorr = 0, vCorr = 0.069;  // 
int powerHisteresis = 4;

int buttonPins[BUTTON_COUNT] = {12, A3};
String buttonLcdMsg[BUTTON_COUNT] = {"---", "+++"};
int buttonChValue[2] = {-1, 1}; // temperatyre adjust step

// PID controller coefficients:
float kp = 50;     // proportional
float ki = 5;       // integral 
float kd = 0;       // differencial
// PID calculaded vars
float powe = 0, powe_1 = 0;
float En = 0, En_1 = 0, En_2 = 0;

//  object/ air/ unused/ temperature
float temp0 = 0, temp1 = 0, temp2 = 0; 
// ADC temprature var
int raw0, raw1, raw2;


// serial device LCD pin
//const int rs = 11, en = 10, d4 = 9, d5 = 8, d6 = 7, d7 = 6;
// laboratiory device LCD pin
const int rs = 6, en = 7, d4 = 8, d5 = 9, d6 = 10, d7 = 11;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
   
    pinMode( A0, INPUT );
    pinMode( A1, INPUT );
    pinMode( A2, INPUT );
    pinMode(buttonPins[0],INPUT_PULLUP); //A3
    pinMode(buttonPins[1],INPUT_PULLUP); //A4
    pinMode( A5, INPUT );

    
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(POW, OUTPUT);
    pinMode(FAN, OUTPUT);

    Serial.begin(9600);
    lcd.begin(16,2);
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
       
    raw2 = analogRead(A2);
    temp2 = ( raw2*0.489)-273+tCorr;

    Serial.print(temp1);
    Serial.print("/   Tset=");
    Serial.println(temp_reg,0);
    
    lcd.setCursor(0,0);
    lcd.print(temp0,1);
    lcd.print("/");
    lcd.print(temp1,0);
    lcd.print("/Y=");
    lcd.print(temp_reg,0);
    lcd.print("С");  
    
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
    if(powe > powerHisteresis) {
      digitalWrite(RELAY_PIN, LOW);
    
      lcd.setCursor(0,1);
     
    }
    if (powe < -1* powerHisteresis){
      digitalWrite(RELAY_PIN, HIGH);
      lcd.setCursor(0,1);
      
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
    float v0 = ((analogRead(A5)*0.069));
    float v1 = ((analogRead(A6)*0.069));
        
    Serial.print("    V0=");
    Serial.println(v0);
    lcd.print(v0,1);
    lcd.print("V ");
    lcd.print(v1,1);
    lcd.print("V ");
    
    Serial.print("    V1=");
    Serial.println(v1);    
    delay(2500); 
          
    // time out for power          
    analogWrite(POW, 0);
    delay(50);

    //    
   
    int button;   
    for(int i = 0; i < 100 ; button = i % BUTTON_COUNT){
      if (!digitalRead(buttonPins[button])) {
    
        Serial.print(buttonLcdMsg[button]);
        lcd.setCursor(0,1);
        lcd.print(buttonLcdMsg[button]);
        
        temp_reg = temp_reg + buttonChValue[button];
        Serial.println(temp_reg);
        //delay(1000); 
        break;
      }
      i++;
      
  }
   
}

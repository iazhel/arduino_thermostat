#include <LiquidCrystal.h>

#define BUTTON_COUNT 2
#define POW 3       // % of power 
#define RELAY_PIN 4 // + or - polarity
#define FAN 5       // % of FAN


float temp_reg = 25; // TEMPERATURE WHAT WE NEED AT START

int buttonPins[BUTTON_COUNT] = {A3, A4};
String buttonNames[BUTTON_COUNT] = {"Button_0", "Button_1"};

                    // PID controller coefficients:
float kp = 100;     // proportional
float ki = 5;       // integral 
float kd = 0;       // differencial

float temp0 = 0;    // var of temperaure what we need
float temp1 = 0;    // var of outwhwere temperature
float temp2 = 0;    // var of FAN radiator temperature
int raw0 = 0;   // analog input temperature sensors
int raw1 = 0;   // vars 
int raw2 = 0;

float powe = 0;
float powe_1 = 0;
float En = 0;
float En_1 = 0;
float En_2 = 0;

LiquidCrystal lcd(6,7,8,9,10,11);

void setup() {
    
    pinMode( A0, INPUT );
    pinMode( A1, INPUT );
    pinMode( A2, INPUT );
    pinMode(buttonPins[0],INPUT_PULLUP);
    pinMode(buttonPins[1],INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(POW, OUTPUT);
    pinMode(FAN, OUTPUT);

    Serial.begin(9600);

    lcd.begin(16,2);
    lcd.println("Power:  ");  
}
    
void loop() {
    Serial.println("T Sensor      Outwhere      FAN ");    
    Serial.print("T:");    
    raw0 = analogRead(A0);
    temp0 = ( raw0*0.489)-273;
    Serial.print(temp0);    
    Serial.print("/    ");    
    
    raw1 = analogRead(A1);
    temp1 = ( raw1*0.489)-273;
    Serial.print(temp1);
    Serial.print("/    ");    
    
    raw2 = analogRead(A2);
    temp2 = ( raw2*0.489)-273;
    Serial.println(temp2);
    
    lcd.setCursor(0,1);
    lcd.print(temp0,1);
    lcd.print("/");
    lcd.print(temp1,1);
    lcd.print("/");
    lcd.print(temp2,1);
    lcd.print(" C");
    
    // save coefficints for recutent evaluation
    En_2 = En_1;
    En_1 = En;
    En = temp_reg - temp0;
    powe_1 = powe;
    
    float Ui = ki*En;
    float Up = kp*(En - En_1);
    float Ud = kd*(En-2*En_1+En_2);
    powe = powe_1 + Ui + Up + Ud;
    
    Serial.print("P:");
    Serial.print(powe/2.5,0);
    Serial.print("%");
    
    Serial.print("  In/Dif/Lin: ");
    Serial.print(Ui,2);
    Serial.print("/");
    Serial.print(Ud,1);
    Serial.print("/");
    Serial.println(Up,1);
    
    if (powe > 250)
        powe = 250;
    if (powe < -250)
        powe = -250;
        
    lcd.setCursor(7,0);
    lcd.print("    ");    
    lcd.setCursor(7,0);
    lcd.print(powe/2.5,0);
    lcd.print("%");
    
    if(powe > 0) 
      digitalWrite(RELAY_PIN, LOW);
    else  
      digitalWrite(RELAY_PIN, HIGH);
    
    if (abs(powe) > 200)
        digitalWrite(FAN, 128);
    if (abs(powe) < 100)                 
        digitalWrite(FAN, 0);
        
    analogWrite(POW, abs(powe));
    delay(3000); 
              
    analogWrite(POW, 0);
    delay(100);
    
    for(int button = 0; ; button = (button +1)% BUTTON_COUNT){
    if (!digitalRead(buttonPins[button])) {
      Serial.print("button");
      delay(100); 
      break;
    }
  }

     
     
    
}

#include <LiquidCrystal.h>

#define lcdLen 8
#define BUZZER 2
boolean buttonUpLevel = HIGH;
boolean buttonIsUp  = false;
boolean ledEnabled  = false;

#define BUTTON_COUNT 2
int buttonPins[BUTTON_COUNT] = {A5, A4};
String buttonLcdMsg[BUTTON_COUNT] = {"--", "++"};


int printPause = 300, requestCount = 50;

// serial device LCD pin
const int rs = 11, en = 10, d4 = 9, d5 = 8, d6 = 7, d7 = 6;
// laboratiory device LCD pin
// const int rs = 6, en = 7, d4 = 8, d5 = 9, d6 = 10, d7 = 11;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
//  pinMode(BUZZER, OUTPUT);
  pinMode(buttonPins[0], INPUT_PULLUP);
  pinMode(buttonPins[1], INPUT_PULLUP);
  lcd.begin(lcdLen, 2);
}

int buttonN;

void loop() {
  lcd.setCursor(0,0); 
  lcd.print(printPause);
  lcd.print("ms ");
  delay(printPause);
  lcd.setCursor(0,0); // 0-я позиция 1-й строчки
  //lcd.print("___ButtonRequest");
  delay(printPause);
  
  int button;   
    for(int i = 0; i < 100 ; button = i % BUTTON_COUNT){
      lcd.clear();
      lcd.print(i);
      lcd.setCursor(0,1);
      lcd.print(" request");
      delay(200);
      
      if (digitalRead(buttonPins[button])==!buttonUpLevel) {
     
        //Serial.println("Button pressed: "); 
        //Serial.print(buttonLcdMsg[button]);
        
        lcd.setCursor(0,0);
        //lcd.print(analogRead(buttonPins[button]));
        lcd.print("Button ");
        
        lcd.setCursor(0,1);
        lcd.print(buttonLcdMsg[button]);
        lcd.print(" pressed");
        
//        temp_reg = temp_reg + buttonChValue[button];
        digitalWrite(BUZZER, HIGH);
        delay(4*printPause);
        digitalWrite(BUZZER, LOW); 
        break;
      }
      i++;
   }
}

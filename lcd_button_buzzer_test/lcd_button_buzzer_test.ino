#include <LiquidCrystal.h>
#define BUTTON_COUNT 2


boolean buttonIsUp  = HIGH;
boolean buttonWasUp = !buttonIsUp;


boolean ledEnabled  = false;

int buttonPins[BUTTON_COUNT] = {A5, A4};
String buttonLcdMsg[BUTTON_COUNT] = {"--", "++"};

int buzzerPin = 2;
int waitingTime = 3000, requestCount = 50;

//serial device LCD pin
const int rs = 11, en = 10, d4 = 9, d5 = 8, d6 = 7, d7 = 6;
// laboratiory device LCD pin
//const int rs = 6, en = 7, d4 = 8, d5 = 9, d6 = 10, d7 = 11;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPins[0], INPUT_PULLUP);
  pinMode(buttonPins[1], INPUT_PULLUP);
  lcd.begin(8, 2);
}

int buttonN;

void loop() {
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print(waitingTime);
  lcd.print("ms");
  lcd.setCursor(0,1);
  lcd.print("waiting...");
  delay(waitingTime);
  lcd.clear();
  
  for(int i = 0; i < requestCount ; buttonN = i % BUTTON_COUNT){
     lcd.setCursor(0,0);
     // print timer
     if (i < 10) {
      lcd.print(0);
     }
     lcd.print(i);

//Button 0 
   lcd.print(" ");
//   lcd.print(analogRead(buttonPins[0]));     
   lcd.print(digitalRead(buttonPins[0]));
   lcd.print("#B0  ");

//Button 1     
     lcd.setCursor(3,1);
//     lcd.print(analogRead(buttonPins[1]));     
     lcd.print(digitalRead(buttonPins[1]));
     lcd.print("#B1  ");
     delay(1000);
       
    // если кнопка не вверху(нажата)
      if (digitalRead(buttonPins[buttonN])==!buttonIsUp) {
        lcd.setCursor(0,0);
        lcd.print(buttonLcdMsg[buttonN]);
        digitalWrite(buzzerPin, HIGH);
        delay(500);
        digitalWrite(buzzerPin, LOW);
        lcd.setCursor(0,0);
        lcd.print("  ");
        //break;
    }
  i++;
 }
}

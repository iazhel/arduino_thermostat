#include <LiquidCrystal.h>


boolean buttonWasUp = true;
boolean buttonIsUp  = false;
boolean ledEnabled  = false;

#define BUTTON_COUNT 2
int buttonPins[BUTTON_COUNT] = {12, A3};
String buttonLcdMsg[BUTTON_COUNT] = {"--", "++"};

int buzzerPin = 2;
int waitingTime = 3000, requestCount = 50;

// serial device LCD pin
//const int rs = 11, en = 10, d4 = 9, d5 = 8, d6 = 7, d7 = 6;
// laboratiory device LCD pin
const int rs = 6, en = 7, d4 = 8, d5 = 9, d6 = 10, d7 = 11;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPins[0], INPUT_PULLUP);
  pinMode(buttonPins[1], INPUT_PULLUP);
  lcd.begin(8, 2);
}

int buttonN;

void loop() {
  lcd.setCursor(0,0); 
  lcd.print("__Wait ");
  lcd.print(waitingTime);
  lcd.print("ms      ");
  delay(waitingTime);
  lcd.setCursor(0,0); // 0-я позиция 1-й строчки
  lcd.print("___ButtonRequest");
  
  for(int i = 0; i < requestCount ; buttonN = i % BUTTON_COUNT){
     lcd.setCursor(0,0);
     if (i < 10) {
      lcd.print(0);
     }
     lcd.print(i);
     lcd.setCursor(0,2);
     lcd.print(digitalRead(buttonPins[0]));
     lcd.print(digitalRead(buttonPins[1]));
     lcd.print(" on ");
     lcd.print(buttonPins[0]);
     lcd.print("/");
     lcd.print(buttonPins[1]);
     delay(100);
       
    // проверяем, отпущена ли кнопка сейчас
    buttonIsUp = digitalRead(buttonPins[buttonN]);
 
    // исключаем ложный сигнал при «дребезге» контактов
    // и считываем сигнал с кнопки повторно
    delay(10);
    buttonIsUp = digitalRead(buttonPins[buttonN]);

    // если кнопка не была до этого момента нажата
      if (!buttonIsUp) {
      lcd.setCursor(3,1);
      lcd.print(buttonLcdMsg[buttonN]);
      digitalWrite(buzzerPin, HIGH);
      delay(250);
      digitalWrite(buzzerPin, LOW);
      lcd.setCursor(3,1);
      lcd.print("  ");
      break;
    }
 
  i++;
 }
}

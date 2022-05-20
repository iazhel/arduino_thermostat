//Blink
//Выводим строку hello, world! и мигающий курсор в виде знакоместа.


#include <LiquidCrystal.h>

// используйте свои значения
const int rs = 11, en = 10, d4 = 9, d5 = 8, d6 = 7, d7 = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // 1601 кодируется как 8x2
  lcd.begin(8, 2);
  // выводим сообщение
  
  //lcd.setCursor(0,1); // 0-я позиция 1-й строчки
  lcd.print("Hello,__");
  lcd.setCursor(0,2); // 0-я позиция 2-й строчки
  lcd.print("world!_");

}

void loop() {
  // Выключаем мигающий курсор
  lcd.noBlink();
  delay(3000);
  // Включаем мигающий курсор
  lcd.blink();
  delay(3000);
}

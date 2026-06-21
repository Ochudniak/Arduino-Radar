// Radar, w ktorym zakladamy zasieg 200cm


#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>

Servo mojeSerwo;

// Znak nieskonczonosci
byte infinity[8] = { 
  B00000,
  B00000,
  B01110,
  B10001,
  B10001,
  B01110,
  B00000,
  B00000
};

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// HC-SR04
const int trigPin = 9;
const int echoPin = 8;

// Zmienne na wartosci z joysticka
int x;
int y;
int sw;

// Zmienna opcji menu
int menuOpt = 1;

// Zmienna stanu LED
bool isLed = true;

// Zmienna odleglosci pobranej przez radar
int distance;

void millisDelay(unsigned long a) { // Funkcja z użyciem millis zastepujaca delay()
  unsigned long start = millis();

  while (millis() - start < a) {
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(12, INPUT_PULLUP); // SW Joystick

  // HC-SR04
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(11, OUTPUT); //Czerwony LED

  lcd.begin(16, 2);

  lcd.createChar(0, infinity); // Zapisanie znaku nieskonczonosci w CGRAM pod indeksem 0

  // Test serwo
  mojeSerwo.attach(10);
  mojeSerwo.write(0);
  millisDelay(1000);
  mojeSerwo.write(180);
}

void printAngleDistance(int a, int b) { // Funkcja wypisujaca kat i odleglosc na LCD

  lcd.setCursor(0, 0);
  lcd.print("Distance = ");
  if(a == 0) {
    lcd.write(byte(0));
    lcd.write(byte(0));
  } else {
    lcd.print(a);
  }
  lcd.print("  ");
  lcd.setCursor(0, 1);
  lcd.print("Angle = ");
  lcd.print(b);
  lcd.print(" ");

  Serial.print("Angle: ");
  Serial.print(b);
  Serial.print(" Distance: ");
  Serial.println(a);
}

void printLCD(String tekst1, String tekst2) { // Funkcja upraszczajaca wypisywanie zdan na ekranie LCD
  lcd.setCursor(0, 0);
  lcd.print(tekst1);
  lcd.setCursor(0, 1);
  lcd.print(tekst2);
}

int radarInput() { //Funkcja pobierajaca dane z HC-SR04
  long czas = 0;
  int liczbaPomiarow = 0;

  for(int i = 0 ; i<3 ; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    czas = pulseIn(echoPin, HIGH, 15000);

    if(czas != 0) { // Do sredniej z 3 pomiarow uzywamy tylko odczyty, ktore cos zwrocily
      czas += czas;
      liczbaPomiarow += 1; 
    }
  }

  if(liczbaPomiarow == 0) {
    return 0;
  }

  return czas / liczbaPomiarow / 58; //odległość po 3 pomiarach
}

void skan(int pos) { // Opcja menu info, ktora robi skan otoczenia o kacie 180 stopni
  Serial.println("SCANNING ...");

  // 0 -> 180
  int minDistance = 200;
  int angle = 0;

  joystickRead();

  if(pos == 180) {
    // 0 -> 180
    for(int i = 0 ; i<=9 ; i++) {
      distance = radarInput();
      if(distance<200 && distance>0) { // Zapisujemy najblizszy obiekt jesli taki jest
        if(distance < minDistance) {
          minDistance = distance;
          angle = i*20;
        }
      } 
      printAngleDistance(distance, i*20);
      mojeSerwo.write(180-(i*20));
      if(isLed == true) {
        analogWrite(11, i*14);
      }
      millisDelay(400);
    }
  } 
  else {
    // 180 -> 0
    for(int i = 9 ; i>=0 ; i--) {
      distance = radarInput();
      if(distance<200 && distance>0) { // Zapisujemy najblizszy obiekt jesli taki jest
        minDistance = distance;
        angle = i*20;
      } 
      printAngleDistance(distance, i*20);
      mojeSerwo.write(180-(i*20));
      if(isLed == true) {
        analogWrite(11, i*14);
      }
      millisDelay(400);
    }
    lcd.clear();
  }
  
  if(minDistance > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Angle:");
    lcd.print(angle);
    lcd.print(" Dis:");
    lcd.print(minDistance);
    lcd.setCursor(0, 1);
    lcd.print("Click to save");

    Serial.println("FOUND DATA, DO U WANT TO SAVE?...");

    while(true) { 
      joystickRead();

      if(sw == LOW) {
        Serial.println("SAVING...");
        lcd.clear();
        EEPROM.update(1, angle);
        EEPROM.update(2, minDistance);
        printLCD("SAVING", "...");
        millisDelay(500);
        Serial.println("BACK TO THE MENU");
        Serial.println("_____________");
        break;
      }
      if(y > 720) { // Powrot do menu
        Serial.println("BACK TO THE MENU");
        Serial.println("_____________");
        break;
      }
    }
  }
  Serial.println("BACK TO MENU");
  Serial.println("_____________");

  digitalWrite(11, LOW); // Po skanie wylacza LED
  lcd.clear();
}

void led() { // Opcja menu LED, wylaczajaca lub wlaczajaca LED podczas skanowania
  Serial.println("LED");

  if(isLed == true) {
    Serial.println("LED IS ON");
    printLCD("LED -> ON ", "CLICK TO CHANGE");
  }
  else {
    Serial.println("LED IS OFF");
    printLCD("LED -> OFF", "CLICK TO CHANGE");
  }

  while(true) {
    joystickRead();
    millisDelay(300);
    if(sw == LOW) { // Wcisniecie guzika zmienia stan ledow podczas skanu
      isLed = !isLed;
      if(isLed == true) {
        Serial.println("LED TURNED ON");
        printLCD("LED -> ON ", "CLICK TO CHANGE");
      } 
      else {
        Serial.println("LED TURNED OFF");
        digitalWrite(11, LOW); // Wyłacza LED
        printLCD("LED -> OFF", "CLICK TO CHANGE");
      }
    }
    if(y > 700) {
      Serial.println("BACK TO THE MENU");
      Serial.println("_____________");
      lcd.clear();
      break;
    }
  }
}

void last() { // Opcja menu last, ktora zapisuje w EEPROM ostatnia zapisana wartosc ze skanu
  Serial.println("CHECKING LAST SAVED DATA...");
  
  int a = EEPROM.read(1);
  int b = EEPROM.read(2);

  if(a==255 || b==255) {
    Serial.println("NO DATA SAVED TO EEPROM");
    printLCD("NO DATA", "SAVED");
    millisDelay(3500);
    Serial.println("BACK TO THE MENU");
    Serial.println("_____________");
    lcd.clear();
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("Angle:");
  lcd.print(a);
  lcd.print(" Dis:");
  lcd.print(b);
  lcd.setCursor(0, 1);
  lcd.print("Click to pos");

  while(true) {
    joystickRead();
    millisDelay(300);
    if(sw == LOW) { // Ustawienie serwa na ostatnio zapisana pozycje
      mojeSerwo.write(180-a);
      Serial.println("SETTING SERVO TO LAST SAVE DATA");
    }
    if(y > 700) { // Powrot do menu
      Serial.println("BACK TO THE MENU");
      Serial.println("_____________");
      lcd.clear();
      break;
    }
  }
}

void info() { // Opcja menu info, z githubem tworcy
  Serial.println("INFO");
  printLCD("GitHub ", "Ochudniak");
  
  while(true) { // Cofniecie do menu
    joystickRead();
    if(y > 700) {
      Serial.println("BACK TO THE MENU");
      Serial.println("_____________");
      lcd.clear();
      break;
    }
  }
}

void delEeprom() { // Opcja menu del last, usuwajaca ostatnia zapisana wartosc do EEPROM
  lcd.clear();
  printLCD("DELETING", "DATA");
  millisDelay(2000);
  Serial.println("DELETING LAST SAVED DATA");
  EEPROM.update(1, 255);
  EEPROM.update(2, 255);
  lcd.clear();
  Serial.println("BACK TO THE MENU");
  Serial.println("_____________");
}

void joystickRead() { // Funkcja pobierajaca dane z joysticka
  x = analogRead(A0);
  y = analogRead(A1);
  sw = digitalRead(12);
}

void menu(int a) { // Funkcja od pokazywania opcji menu
  switch (a) {
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("SKAN");
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print("LED ");
      break;
    case 3:
      lcd.setCursor(0, 1);
      lcd.print("LAST");
      break;
    case 4:
      lcd.setCursor(0, 1);
      lcd.print("DEL LAST");
      break;
    case 5:
      lcd.setCursor(0, 1);
      lcd.print("INFO");
  }
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("MENU");
  joystickRead();
  menu(menuOpt);
  millisDelay(300);
  

  if(x < 300) { // Poruszanie sie po menu w gore
    if(menuOpt > 1) {
      menuOpt -= 1;
      lcd.clear();
      menu(menuOpt);
    }
  }
  if(x > 720) { // Poruszanie sie po menu w dol
    if(menuOpt < 5) {
      menuOpt += 1;
      lcd.clear();
      menu(menuOpt);
    }
  }

  if(sw == LOW) { // Wybranie opcji w menu
    switch(menuOpt) {
      case 1:
        skan(mojeSerwo.read());
        break;
      case 2:
        led();
        break;
      case 3:
        last();
        break;
      case 4:
        delEeprom();
        break;
      case 5:
        info();
        break;
    }
  }
}

# Arduino Radar z menu, LCD i zapisem do EEPROM

Projekt prostego radaru opartego na Arduino. Układ wykorzystuje czujnik ultradźwiękowy HC-SR04 zamontowany na serwomechanizmie, wykonuje skan w zakresie 180°, pokazuje aktualny kąt i odległość na wyświetlaczu LCD oraz pozwala zapisać najbliżej wykryty obiekt do pamięci EEPROM.

Sterowanie odbywa się za pomocą joysticka. Projekt ma też menu, obsługę diody LED podczas skanowania i możliwość powrotu do ostatnio zapisanej pozycji.

---

## Spis treści

- [Opis działania](#opis-działania)
- [Funkcje projektu](#funkcje-projektu)
- [Wymagane elementy](#wymagane-elementy)
- [Biblioteki](#biblioteki)
- [Podłączenie pinów](#podłączenie-pinów)
- [Menu urządzenia](#menu-urządzenia)
- [Skanowanie](#skanowanie)
- [Zapis do EEPROM](#zapis-do-eeprom)
- [Komunikacja przez Serial Monitor](#komunikacja-przez-serial-monitor)
- [Uruchomienie projektu](#uruchomienie-projektu)
- [Struktura kodu](#struktura-kodu)
- [Możliwe usprawnienia](#możliwe-usprawnienia)
- [Autor](#autor)

---

## Opis działania

Radar obraca czujnik ultradźwiękowy za pomocą serwomechanizmu i mierzy odległość od przeszkód. Pomiar wykonywany jest dla kolejnych kątów w zakresie od `0°` do `180°`, co `20°`.

Dane są wyświetlane na ekranie LCD oraz wysyłane do Serial Monitora. Jeżeli podczas skanowania zostanie znaleziony obiekt w zakresie do około `200 cm`, program zapamiętuje najbliższą wykrytą odległość oraz kąt, pod którym obiekt został wykryty.

Po zakończeniu skanu użytkownik może zapisać znalezioną pozycję do pamięci EEPROM. Dzięki temu dane nie znikają po odłączeniu zasilania.

---

## Funkcje projektu

- skanowanie przestrzeni w zakresie `180°`,
- pomiar odległości za pomocą czujnika HC-SR04,
- sterowanie serwomechanizmem,
- menu obsługiwane joystickiem,
- wyświetlanie danych na LCD 16x2,
- wysyłanie danych do Serial Monitora,
- zapisywanie ostatniego wykrytego obiektu do EEPROM,
- odczyt zapisanej pozycji,
- ustawienie serwa na ostatnio zapisaną pozycję,
- kasowanie zapisanych danych,
- opcjonalne włączanie i wyłączanie diody LED podczas skanowania.

---

## Wymagane elementy

Do zbudowania projektu potrzebne są:

| Element | Ilość | Opis |
|---|---:|---|
| Arduino Uno / Nano lub kompatybilna płytka | 1 | Główny mikrokontroler |
| HC-SR04 | 1 | Czujnik ultradźwiękowy do pomiaru odległości |
| Serwomechanizm | 1 | Obraca czujnik w zakresie 180° |
| LCD 16x2 | 1 | Wyświetlanie menu, kąta i odległości |
| Joystick analogowy z przyciskiem | 1 | Obsługa menu |
| Dioda LED czerwona | 1 | Sygnalizacja podczas skanowania |
| Rezystor do LED | 1 | Zalecany, np. 220 Ω |
| Przewody połączeniowe | - | Do połączenia elementów |
| Płytka stykowa | 1 | Do prototypowania układu |

---

## Biblioteki

Kod korzysta z bibliotek dostępnych w środowisku Arduino IDE:

```cpp
#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>
```

Znaczenie bibliotek:

| Biblioteka | Zastosowanie |
|---|---|
| `LiquidCrystal` | Obsługa wyświetlacza LCD 16x2 |
| `Servo` | Sterowanie serwomechanizmem |
| `EEPROM` | Zapis i odczyt danych z pamięci nieulotnej Arduino |

---

## Podłączenie pinów

### Wyświetlacz LCD

W kodzie LCD jest podłączony w trybie 4-bitowym:

```cpp
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
```

| Pin LCD | Pin Arduino |
|---|---:|
| RS | 2 |
| E | 3 |
| D4 | 4 |
| D5 | 5 |
| D6 | 6 |
| D7 | 7 |

### Czujnik HC-SR04

| Pin HC-SR04 | Pin Arduino |
|---|---:|
| TRIG | 9 |
| ECHO | 8 |
| VCC | 5V |
| GND | GND |

### Serwomechanizm

| Przewód serwa | Pin Arduino |
|---|---:|
| Sygnał | 10 |
| VCC | 5V |
| GND | GND |

### Joystick

| Pin joysticka | Pin Arduino |
|---|---:|
| X | A0 |
| Y | A1 |
| SW | 12 |
| VCC | 5V |
| GND | GND |

Przycisk joysticka działa z użyciem `INPUT_PULLUP`, dlatego po wciśnięciu zwraca stan `LOW`.

### Dioda LED

| Element | Pin Arduino |
|---|---:|
| LED czerwona | 11 |

Pin `11` obsługuje PWM, dlatego podczas skanowania jasność LED może zmieniać się zależnie od aktualnego kroku skanu.

---

## Menu urządzenia

Menu wyświetlane jest na LCD. Nawigacja odbywa się joystickiem w osi `X`, a wybór opcji przez kliknięcie joysticka.

| Opcja | Opis |
|---|---|
| `SKAN` | Uruchamia skanowanie otoczenia |
| `LED` | Włącza lub wyłącza LED podczas skanowania |
| `LAST` | Pokazuje ostatnio zapisany pomiar i pozwala ustawić serwo w tej pozycji |
| `DEL LAST` | Usuwa ostatnio zapisane dane z EEPROM |
| `INFO` | Wyświetla informację o autorze/profilu GitHub |

Sterowanie:

| Ruch / akcja | Działanie |
|---|---|
| Joystick X w lewo | Przejście do poprzedniej opcji |
| Joystick X w prawo | Przejście do następnej opcji |
| Kliknięcie joysticka | Wybranie aktualnej opcji |
| Joystick Y w górę | Powrót z podmenu do menu głównego |

---

## Skanowanie

Funkcja skanowania znajduje się w:

```cpp
void skan(int pos)
```

Skan działa w dwóch kierunkach, zależnie od aktualnej pozycji serwa:

- jeżeli serwo jest na `180°`, skan przechodzi od `0°` do `180°`,
- w przeciwnym przypadku skan przechodzi od `180°` do `0°`.

Pomiar wykonywany jest dla 10 punktów:

```cpp
0°, 20°, 40°, 60°, 80°, 100°, 120°, 140°, 160°, 180°
```

Dla każdego punktu program:

1. wykonuje pomiar czujnikiem HC-SR04,
2. wyświetla kąt i odległość na LCD,
3. wysyła dane do Serial Monitora,
4. obraca serwo,
5. zmienia jasność LED, jeżeli LED jest włączony,
6. czeka około `400 ms`.

Zakres wykrywania najbliższego obiektu jest ograniczony do wartości mniejszych niż `200 cm` i większych niż `0 cm`.

---

## Zapis do EEPROM

Po zakończeniu skanowania, jeżeli wykryto obiekt, LCD pokazuje:

```text
Angle:<kąt> Dis:<odległość>
Click to save
```

Po kliknięciu joysticka dane są zapisywane do EEPROM:

| Adres EEPROM | Dane |
|---:|---|
| `1` | Kąt |
| `2` | Odległość |

Do usuwania danych wykorzystywana jest wartość `255`, która oznacza brak zapisanego pomiaru.

Usuwanie danych wykonuje opcja:

```text
DEL LAST
```

Odczyt danych wykonuje opcja:

```text
LAST
```

Po wejściu w `LAST` można kliknąć joystick, aby ustawić serwo na ostatnio zapisany kąt.

---

## Komunikacja przez Serial Monitor

Program uruchamia komunikację szeregową z prędkością:

```cpp
Serial.begin(9600);
```

W Serial Monitorze pojawiają się między innymi:

- aktualny kąt,
- aktualna odległość,
- informacja o rozpoczęciu skanowania,
- informacja o znalezieniu danych,
- informacja o zapisie do EEPROM,
- informacja o powrocie do menu,
- stan LED.

Przykładowy format danych:

```text
Angle: 40 Distance: 73
Angle: 60 Distance: 51
Angle: 80 Distance: 0
```

---

## Uruchomienie projektu

1. Złóż układ zgodnie z tabelą połączeń.
2. Otwórz plik `.ino` w Arduino IDE.
3. Wybierz odpowiednią płytkę, np. `Arduino Uno`.
4. Wybierz port szeregowy.
5. Wgraj program na płytkę.
6. Otwórz Serial Monitor z prędkością `9600 baud`.
7. Po starcie serwo wykona krótki test ruchu.
8. Na LCD pojawi się menu.
9. Wybierz opcję `SKAN`, aby rozpocząć skanowanie.

---

## Struktura kodu

Najważniejsze funkcje w programie:

| Funkcja | Opis |
|---|---|
| `setup()` | Konfiguracja pinów, LCD, serwa, Serial Monitora i znaku nieskończoności |
| `loop()` | Główna pętla programu i obsługa menu |
| `millisDelay()` | Prosta funkcja opóźnienia oparta na `millis()` |
| `printAngleDistance()` | Wyświetla i wysyła przez Serial aktualny kąt oraz odległość |
| `printLCD()` | Ułatwia wypisywanie dwóch linii tekstu na LCD |
| `radarInput()` | Pobiera odczyt z czujnika HC-SR04 |
| `skan()` | Wykonuje skanowanie otoczenia |
| `led()` | Włącza lub wyłącza działanie LED podczas skanu |
| `last()` | Odczytuje ostatnio zapisane dane z EEPROM |
| `delEeprom()` | Usuwa zapisane dane z EEPROM |
| `joystickRead()` | Odczytuje wartości z joysticka |
| `menu()` | Wyświetla aktualną opcję menu |

---

## Uwagi techniczne

- Program zakłada maksymalny interesujący zasięg około `200 cm`.
- `pulseIn()` ma ustawiony timeout `15000 µs`, więc brak echa nie blokuje programu zbyt długo.
- W pamięci EEPROM zapisywane są tylko dwie wartości: kąt i odległość.
- Przycisk joysticka działa odwrotnie niż zwykłe wejście cyfrowe, bo używa `INPUT_PULLUP`: kliknięcie oznacza `LOW`.
- W funkcji `radarInput()` intencją jest uśrednianie kilku pomiarów. Warto sprawdzić implementację sumowania pomiarów, jeżeli wyniki będą wyglądały niestabilnie.

---

## Możliwe usprawnienia

Pomysły na dalszy rozwój projektu:

- dodanie wizualizacji radaru w JavaScript,
- rysowanie wykrytych punktów na canvasie w przeglądarce,
- przesyłanie danych z Arduino do komputera przez Serial i renderowanie mapy otoczenia,
- dodanie większej liczby punktów pomiarowych, np. co `5°` lub `10°`,
- zapis historii wielu pomiarów zamiast tylko ostatniego,
- dodanie buzzera ostrzegającego o bliskim obiekcie,
- dodanie obudowy drukowanej w 3D,
- dodanie dokładniejszego filtrowania pomiarów z HC-SR04,
- rozbudowanie menu o ustawienia zakresu skanowania,
- zapis danych w formacie łatwym do późniejszej analizy.

Przykładowy przyszły kierunek dla wizualizacji:

```text
Arduino -> Serial Monitor / Web Serial API -> JavaScript Canvas -> mapa terenu
```

Na tym etapie projekt działa jako samodzielny radar z LCD i menu. Wizualizacja w JavaScript może zostać dodana jako osobny moduł w przyszłości.

---

## Autor

Projekt przygotowany przez:

```text
GitHub: Ochudniak
```

---

## Licencja

Brak określonej licencji w kodzie źródłowym.  
Jeżeli projekt ma być publiczny na GitHubie, warto dodać plik `LICENSE`, np. z licencją MIT.


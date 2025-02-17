# edm
Electrical discharge machining' by Marcin Czernik (&amp; Adam Ziemkiewicz)

## instalacja

0. otworz terminal z (bash) shell

1. wybierz katalog, w ktorym chcesz umiescic projekt i przejdz do niego, np: ~/Devel/micro/Marcinu/arduino/ 

  ```bash
  $ cd ~/Devel/micro/Marcinu/arduino/
  ```

2. sklonuj projekt z github'a

  ```bash
  $ git clone https://github.com/saltana67/edm.git
  ```
  powinien powstac katalog _emd_, a w nim co najmniej sketch *emd.ino* i plik dokumentacji *README.md* w formacie [markdown][Markdown Cheatsheet]

3. otworz arduino IDE
4. zainstaluj konieczne biblioteki
4. otworz menu *File*, wybierz punkt *Open...*
5. otworzy sie dialog wyboru pliku, nawiguj do katalogu, ktory sklonowales, np: **~/Devel/micro/Marcinu/arduino/edm/**
6. wybierz plik edm.ino

> Mozna tez sciagnac to samo jako [archiwum w formacie zip](https://github.com/saltana67/edm/archive/master.zip) > (dostepne ze strony github pod guzikiem `Clone or download`), po czym rozpakowac samemu i otworzyc w IDE ...

## odswierzanie do najnowszej wersji z repozytorium

1. przejdz do katalogu sklonowanego projektu, np: _~/Devel/micro/Marcinu/arduino/edm/_

  ```bash
  $ cd ~/Devel/micro/Marcinu/arduino/edm
  ```
2. sciagnij zmiany z repozytorium kodu 

  ```bash
  $ git pull
  ```
3. otworz ponownie w Arduino IDE ..

## implementacja

TODO: Najwazniejsza funcja: tryb pracy _auto_.



## biblioteki

### [hd44780 _(Bill Perry wersja 1.1.0)_][hd44780 lib] : sterowanie wyświtlaczem lcd 4x20 znakow przez sterownik oparty na hd44780 podłaczony do magistrali I2C

* [sketch diagnostyczny][I2CexpDiag]
* [sketch przykladowy: wlasne/niestandardowe znaki][hd44780 CustomChars example]

### [Encoder _(PaulStoffregen wersja 1.4.1)_][PaulStoffregen Encoder] : czytanie encodera

Dokumentacja mowi:
> Best Performance: Both signals connect to interrupt pins.
> 
> | Board        | Interrupt Pins       | LED Pin(do not use)|
> | -------------|----------------------| -------------------|
> | Arduino Mega | 2, 3, 18, 19, 20, 21 | 13 |
wiec najlepiej by bylo tak zrobic, podwiesic enkoder na ktores z tych pinow ...


### [AccelStepper _(Mike McCauley wersja 1.58.0)_][MikeMcCauley AccelStepper] : sterowanie silnikiem krokowym

### [arduino-timer-api _(Anton wersja 0.1.0)_][sadr0b0t arduino-timer-api] : timer generujący przerwania  

## referencje
#### [bilioteka hd44780 (Bill Perry) dla arduino][hd44780 arduino lib] _([kod @github][hd44780 lib])_
#### [bilioteka Encoder (Paul Stoffregen) dla arduino][PaulStoffregen Encoder] _([kod @github][PaulStoffregen Encoder code])_
#### [bilioteka AccelStepper (Mike McCauley) dla arduino][MikeMcCauley AccelStepper]
#### [bilioteka arduino-timer-api (Anton) dla arduino][sadr0b0t arduino-timer-api]


### linki 
#### [Markdown Cheatsheet][]
#### [Markdown github basics][] 
#### [Markdown TOC creation][] 

[LICENCE]:			LICENSE
[Markdown Cheatsheet]:		https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet
[Markdown github basics]:	https://help.github.com/en/articles/basic-writing-and-formatting-syntax#quoting-code
[Markdown TOC creation]:	https://github.com/ekalinin/github-markdown-toc

[hd44780 arduino lib]:		https://www.arduinolibraries.info/libraries/hd44780
				"bilioteka hd44780 (Bill Perry) dla arduino"
[hd44780 lib]:			https://github.com/duinoWitchery/hd44780
				"bilioteka hd44780 (Bill Perry)dla arduino @github"
[I2CexpDiag]:			https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/I2CexpDiag/I2CexpDiag.ino
				"sketch diagnostyczny hd44780 przez I2"
[hd44780 CustomChars example]:	https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/LCDCustomChars/LCDCustomChars.ino

[PaulStoffregen Encoder]:	https://www.pjrc.com/teensy/td_libs_Encoder.html
				"Quadrature Encoder Library for Arduino by Paul Stoffregen"

[PaulStoffregen Encoder code]:	https://github.com/PaulStoffregen/Encoder
				"Quadrature Encoder Library for Arduino by Paul Stoffregen @gtihub"

[MikeMcCauley AccelStepper]:	http://www.airspayce.com/mikem/arduino/AccelStepper/
				"AccelStepper library for Arduino by Mike McCauley"

[sadr0b0t arduino-timer-api]:	https://github.com/sadr0b0t/arduino-timer-api
				"timer interrupt library for Arduino by Anton (sadr0b0t @ github)"
	
#### polskie znaki diakrytyczne

W alfabecie polskim jest dziewięć liter tworzonych za pomocą znaków diakrytycznych (litery diakrytyzowane, litery diakrytyczne): 
ą, ć, ę, ł, ń, ó, ś, ź, ż
Ą, Ć, Ę, Ł, Ń, Ó, Ś, Ź, Ż 

W składzie komputerowym często do sprawdzania, czy dany font zawiera polskie litery diakrytyzowane, używa się zdania: „Zażółć gęślą jaźń”. Jest to najkrótsze znane zdanie, które zawiera wszystkie polskie litery diakrytyzowane

_wziete z [polskie znaki diakrytyczne][]_

[polskie znaki diakrytyczne]:	https://pl.wikipedia.org/wiki/Znaki_diakrytyczne

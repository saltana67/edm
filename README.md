# edm
Electrical discharge machining' by Marcin Czernik (&amp; Adam Ziemkiewicz)

## instalacja

0. otworz terminal z (bash) shell

1. wybierz katalog, w ktorym chcesz umiescic project i przejdz do niego, np: ~/Devel/micro/Marcinu/arduino/ 

  ```bash
  adam@sabayon ~ $ cd ~/Devel/micro/Marcinu/arduino/
  ```   

2. sklonuj projekt z github'a
3. otworz arduino IDE
4. zainstaluj konieczne biblioteki
4. otworz menu *File*, wybierz punkt *Open...*
5. otworzy sie dialog wyboru pliku, nawiguj do katalowu, ktory sklonowales, np: ~/Devel/micro/Marcinu/arduino/edm
6. wybierz plik edm.ino


## biblioteki

### [hd44780][hd44780 lib] : sterowanie lcd 4x20 znakow przez sterownik oparty na hd44780 podlaczony do magistrali I2C

* [sketch diagnostyczny][I2CexpDiag]
* [sketch przykladowy: wlasne/niestandardowe znaki][hd44780 CustomChars example]


## referencje


### linki 
#### [Markdown Cheatsheet][]
#### [Markdown TOC creation][] 

[Markdown Cheatsheet]:		https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet
[Markdown TOC creation]:	https://github.com/ekalinin/github-markdown-toc
[hd44780 arduino lib]:		https://www.arduinolibraries.info/libraries/hd44780
				"hd44780 bilioteka arduino"
[hd44780 lib]:			https://github.com/duinoWitchery/hd44780
				"hd44780"
[I2CexpDiag]:			https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/I2CexpDiag/I2CexpDiag.ino
				"sketch diagnostyczny hd44780 przez I2"
[hd44780 CustomChars example]:	https://github.com/duinoWitchery/hd44780/blob/master/examples/ioClass/hd44780_I2Cexp/LCDCustomChars/LCDCustomChars.ino

#### polskie znaki diakrytyczne

W alfabecie polskim jest dziewięć liter tworzonych za pomocą znaków diakrytycznych (litery diakrytyzowane, litery diakrytyczne): 
ą, ć, ę, ł, ń, ó, ś, ź, ż
Ą, Ć, Ę, Ł, Ń, Ó, Ś, Ź, Ż 

W składzie komputerowym często do sprawdzania, czy dany font zawiera polskie litery diakrytyzowane, używa się zdania: „Zażółć gęślą jaźń”. Jest to najkrótsze znane zdanie, które zawiera wszystkie polskie litery diakrytyzowane

_wziete z [polskie znaki diakrytyczne][]_

[polskie znaki diakrytyczne]:	https://pl.wikipedia.org/wiki/Znaki_diakrytyczne

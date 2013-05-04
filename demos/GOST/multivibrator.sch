EESchema Schematic File Version 2  date Sat 04 May 2013 02:49:54 PM MSK
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
EELAYER 24 0
EELAYER END
$Descr A4 8268 11693 portrait
encoding utf-8
Sheet 1 1
Title "Мультивибратор"
Date "21 apr 2013"
Rev ""
Comp "ООО \"XXXXX\""
Comment1 "АБВГ.000000.001"
Comment2 "Лунев"
Comment3 "Барановский"
Comment4 "Викулов"
$EndDescr
$Comp
L PN2222A VT1
U 1 1 5173A466
P 3300 5300
F 0 "VT1" H 3300 5152 40  0000 R CNN
F 1 "~" H 3300 5450 40  0001 R CNN
F 2 "TO92" H 3200 5402 29  0001 C CNN
F 3 "~" H 3300 5300 60  0000 C CNN
F 4 "Транзистор" H 3300 5300 60  0001 C CNN "Title"
F 5 "Fairchild" H 3300 5300 60  0001 C CNN "Manufacturer"
F 6 "допускается замена на MMBT2222A" H 3300 5300 60  0001 C CNN "Note"
	1    3300 5300
	-1   0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 5173A497
P 3900 5300
F 0 "C1" H 3900 5400 40  0000 L CNN
F 1 "<00>0,1 мкФ</00><01>0,22 мкФ</01>" H 3906 5215 40  0001 L CNN
F 2 "~" H 3938 5150 30  0000 C CNN
F 3 "~" H 3900 5300 60  0000 C CNN
F 4 "0805" H 3900 5300 60  0001 C CNN "Type"
F 5 "-X7R-50 В-" H 3900 5300 60  0001 C CNN "SType"
F 6 "20%" H 3900 5300 60  0001 C CNN "Precision"
F 7 "AVX" H 3900 5300 60  0001 C CNN "Manufacturer"
F 8 "<00>Конденсатор</00><01>Конденсатор</01>" H 3900 5300 60  0001 C CNN "Title"
	1    3900 5300
	0    1    1    0   
$EndComp
$Comp
L R R1
U 1 1 5173A4D3
P 3200 4650
F 0 "R1" V 3280 4650 40  0000 C CNN
F 1 "1 кОм" V 3207 4651 40  0001 C CNN
F 2 "~" V 3130 4650 30  0000 C CNN
F 3 "~" H 3200 4650 30  0000 C CNN
F 4 "Резистор" V 3200 4650 60  0001 C CNN "Title"
F 5 "0805" V 3200 4650 60  0001 C CNN "Type"
F 6 "5%" V 3200 4650 60  0001 C CNN "Precision"
F 7 "Yageo" V 3200 4650 60  0001 C CNN "Manufacturer"
	1    3200 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 5500 3200 5700
Wire Wire Line
	3200 5700 5700 5700
Wire Wire Line
	5300 5700 5300 5500
Wire Wire Line
	3200 4900 3200 5100
Wire Wire Line
	3700 4900 3700 5300
Wire Wire Line
	3700 5300 3500 5300
Wire Wire Line
	4800 5300 5000 5300
Wire Wire Line
	4800 4900 4800 5300
Wire Wire Line
	5300 4900 5300 5100
Wire Wire Line
	3200 4400 3200 4200
Wire Wire Line
	3200 4200 5700 4200
Wire Wire Line
	5300 4200 5300 4400
Wire Wire Line
	4800 4200 4800 4400
Connection ~ 4800 4200
Wire Wire Line
	3700 4200 3700 4400
Connection ~ 3700 4200
Wire Wire Line
	5300 5000 4100 5000
Wire Wire Line
	4100 5000 4100 5300
Connection ~ 5300 5000
Wire Wire Line
	4400 5300 4400 5100
Wire Wire Line
	4400 5100 3200 5100
Wire Wire Line
	5700 5700 5700 4800
Connection ~ 5300 5700
Wire Wire Line
	5700 4200 5700 4400
Connection ~ 5300 4200
$Comp
L R R2
U 1 1 5173A7AC
P 3700 4650
F 0 "R2" V 3780 4650 40  0000 C CNN
F 1 "100 кОм" V 3707 4651 40  0001 C CNN
F 2 "~" V 3630 4650 30  0000 C CNN
F 3 "~" H 3700 4650 30  0000 C CNN
F 4 "Резистор" V 3700 4650 60  0001 C CNN "Title"
F 5 "0805" V 3700 4650 60  0001 C CNN "Type"
F 6 "5%" V 3700 4650 60  0001 C CNN "Precision"
F 7 "Yageo" V 3700 4650 60  0001 C CNN "Manufacturer"
	1    3700 4650
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 5173A8F6
P 4600 5300
F 0 "C2" H 4600 5400 40  0000 L CNN
F 1 "<00>0,1 мкФ</00><01>0,22 мкФ</01>" H 4606 5215 40  0001 L CNN
F 2 "~" H 4638 5150 30  0000 C CNN
F 3 "~" H 4600 5300 60  0000 C CNN
F 4 "0805" H 4600 5300 60  0001 C CNN "Type"
F 5 "-X7R-50 В-" H 4600 5300 60  0001 C CNN "SType"
F 6 "20%" H 4600 5300 60  0001 C CNN "Precision"
F 7 "AVX" H 4600 5300 60  0001 C CNN "Manufacturer"
F 8 "<00>Конденсатор</00><01>Конденсатор</01>" H 4600 5300 60  0001 C CNN "Title"
	1    4600 5300
	0    1    1    0   
$EndComp
$Comp
L R R3
U 1 1 5173A96B
P 4800 4650
F 0 "R3" V 4880 4650 40  0000 C CNN
F 1 "100 кОм" V 4807 4651 40  0001 C CNN
F 2 "~" V 4730 4650 30  0000 C CNN
F 3 "~" H 4800 4650 30  0000 C CNN
F 4 "Резистор" V 4800 4650 60  0001 C CNN "Title"
F 5 "0805" V 4800 4650 60  0001 C CNN "Type"
F 6 "5%" V 4800 4650 60  0001 C CNN "Precision"
F 7 "Yageo" V 4800 4650 60  0001 C CNN "Manufacturer"
	1    4800 4650
	1    0    0    -1  
$EndComp
$Comp
L R R4
U 1 1 5173A975
P 5300 4650
F 0 "R4" V 5380 4650 40  0000 C CNN
F 1 "1 кОм" V 5307 4651 40  0001 C CNN
F 2 "~" V 5230 4650 30  0000 C CNN
F 3 "~" H 5300 4650 30  0000 C CNN
F 4 "Резистор" V 5300 4650 60  0001 C CNN "Title"
F 5 "0805" V 5300 4650 60  0001 C CNN "Type"
F 6 "5%" V 5300 4650 60  0001 C CNN "Precision"
F 7 "Yageo" V 5300 4650 60  0001 C CNN "Manufacturer"
	1    5300 4650
	1    0    0    -1  
$EndComp
$Comp
L C C3
U 1 1 5173A4B5
P 5700 4600
F 0 "C3" H 5700 4700 40  0000 L CNN
F 1 "10 мкФ" H 5706 4515 40  0001 L CNN
F 2 "~" H 5738 4450 30  0000 C CNN
F 3 "~" H 5700 4600 60  0000 C CNN
F 4 "<01>Конденсатор</01>" H 5700 4600 60  0001 C CNN "Title"
F 5 "танталовый тип B" H 5700 4600 60  0001 C CNN "Type"
F 6 "-16 В-" H 5700 4600 60  0001 C CNN "SType"
F 7 "20%" H 5700 4600 60  0001 C CNN "Precision"
F 8 "Panasonic" H 5700 4600 60  0001 C CNN "Manufacturer"
	1    5700 4600
	1    0    0    -1  
$EndComp
$Comp
L PN2222A VT2
U 1 1 5173AAC5
P 5200 5300
F 0 "VT2" H 5200 5152 40  0000 R CNN
F 1 "~" H 5200 5450 40  0001 R CNN
F 2 "TO92" H 5100 5402 29  0001 C CNN
F 3 "~" H 5200 5300 60  0000 C CNN
F 4 "Транзистор" H 5200 5300 60  0001 C CNN "Title"
F 5 "Fairchild" H 5200 5300 60  0001 C CNN "Manufacturer"
F 6 "допускается замена на MMBT2222A" H 5200 5300 60  0001 C CNN "Note"
	1    5200 5300
	1    0    0    -1  
$EndComp
$Comp
L C C4
U 1 1 5173B0FE
P 6100 4600
F 0 "C4" H 6100 4700 40  0000 L CNN
F 1 "10 мкФ" H 6106 4515 40  0001 L CNN
F 2 "~" H 6138 4450 30  0000 C CNN
F 3 "~" H 6100 4600 60  0000 C CNN
F 4 "Конденсатор" H 6100 4600 60  0001 C CNN "Title"
F 5 "танталовый тип B" H 6100 4600 60  0001 C CNN "Type"
F 6 "-16 В-" H 6100 4600 60  0001 C CNN "SType"
F 7 "20%" H 6100 4600 60  0001 C CNN "Precision"
F 8 "Panasonic" H 6100 4600 60  0001 C CNN "Manufacturer"
F 9 "Не устанавливается" H 6100 4600 60  0001 C CNN "Note"
	1    6100 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 4800 6100 4800
Wire Wire Line
	5700 4400 6100 4400
$EndSCHEMATC

EESchema Schematic File Version 2  date Thu 18 Oct 2012 10:04:05 PM PDT
LIBS:BoosterPack
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
LIBS:BoosterPack40-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date "19 oct 2012"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L CONN_3 J5
U 1 1 5080A33C
P 7600 6800
F 0 "J5" V 7550 6800 50  0000 C CNN
F 1 "CONN_3" V 7650 6800 40  0000 C CNN
	1    7600 6800
	1    0    0    1   
$EndComp
Wire Wire Line
	7250 6700 7000 6700
Wire Wire Line
	7250 6900 7000 6900
$Comp
L VCC #PWR01
U 1 1 5080A56F
P 7000 6900
F 0 "#PWR01" H 7000 7000 30  0001 C CNN
F 1 "VCC" H 7000 7000 30  0000 C CNN
	1    7000 6900
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR02
U 1 1 5080A57E
P 7000 6700
F 0 "#PWR02" H 7000 6700 30  0001 C CNN
F 1 "GND" H 7000 6630 30  0001 C CNN
	1    7000 6700
	0    1    1    0   
$EndComp
Wire Wire Line
	7100 6800 7100 6700
Connection ~ 7100 6700
Wire Wire Line
	7250 6800 7100 6800
$Comp
L GND #PWR03
U 1 1 5080AA99
P 9150 2450
F 0 "#PWR03" H 9150 2450 30  0001 C CNN
F 1 "GND" H 9150 2380 30  0001 C CNN
	1    9150 2450
	0    1    1    0   
$EndComp
$Comp
L VCC #PWR04
U 1 1 5080AA9F
P 9150 900
F 0 "#PWR04" H 9150 1000 30  0001 C CNN
F 1 "VCC" H 9150 1000 30  0000 C CNN
	1    9150 900 
	0    -1   -1   0   
$EndComp
$Comp
L TI_BOOSTER_40_J1 J1
U 1 1 5080DB5C
P 9750 1350
F 0 "J1" H 9700 2000 60  0000 C CNN
F 1 "TI_BOOSTER_40_J1" H 9750 700 60  0000 C CNN
	1    9750 1350
	1    0    0    -1  
$EndComp
$Comp
L TI_BOOSTER_40_J2 J2
U 1 1 5080DBF4
P 9750 2900
F 0 "J2" H 9700 3550 60  0000 C CNN
F 1 "TI_BOOSTER_40_J2" H 9750 2250 60  0000 C CNN
	1    9750 2900
	1    0    0    -1  
$EndComp
$Comp
L TI_BOOSTER_40_J3 J3
U 1 1 5080DC03
P 9750 4450
F 0 "J3" H 9700 5100 60  0000 C CNN
F 1 "TI_BOOSTER_40_J3" H 9750 3800 60  0000 C CNN
	1    9750 4450
	1    0    0    -1  
$EndComp
$Comp
L TI_BOOSTER_40_J4 J4
U 1 1 5080DC12
P 9750 6000
F 0 "J4" H 9700 6650 60  0000 C CNN
F 1 "TI_BOOSTER_40_J4" H 9750 5350 60  0000 C CNN
	1    9750 6000
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 5080DC79
P 9150 4100
F 0 "#PWR05" H 9150 4100 30  0001 C CNN
F 1 "GND" H 9150 4030 30  0001 C CNN
	1    9150 4100
	0    1    1    0   
$EndComp
$Comp
L +5V #PWR06
U 1 1 5080DC8B
P 9150 4000
F 0 "#PWR06" H 9150 4090 20  0001 C CNN
F 1 "+5V" H 9150 4090 30  0000 C CNN
	1    9150 4000
	0    -1   -1   0   
$EndComp
$EndSCHEMATC

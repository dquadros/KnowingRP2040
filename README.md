# KnowingRP2040
Examples for the **Knowing the RP2040** book.

This examples were tested with the Raspberry Pi Pico C/C++ SDK v1.3 and the Raspberry Pi Pico board.

To compile and run the code follow instructions on the Raspberry Pi Pico C/C++ SDK Users Guide.

Organization of the files follow the chapters of the book.

## Chapter 3 - The Cortex-M0+ Processor Cores

### Dual Core

Running code in both ARM cores, with synchronization.

## Chapter 7 - GPIO, Pad and PWM

### GPIO7Segment

Digital output example: driving a four digit seven segment display.

### GPIOKeypad

Digital input example: reading a 4x4 matrix keypad.

### PWMDemo

Generating PWM signals.

### PWMMeasure

Using the PWM peripheral to measure frequency and duty cycle.

## Chapter 8 - The PIO

### SquareWave

Using the PIO to generate a square wave in a pin.

### SerialTx

Serially transmit data with a clock.

### SerialRx

Receive the data sent by SerialTx.

### HCSR04

Using the PIO to interface an HC-SR04 ultrasonic sensor.

## Chapter 9 - The UART

### UartSum

Reading numbers through the UART and print the sum.

## Chapter 12 - Analog Input: The ADC

### AdcDemo

Using the ADC to read the internal temperature sensor and an external light sensor (LDR).  

## Chapter 13 - A Brief Introduction to USB

### KbdDevice

A five key USB keyboard device

### UsbSerial

A very basic serial to USB adapter.

# Smart-Battery-Management-System
Arduino based Battery Management System for Lithium Ion Battery Protection

## Features

- Voltage Monitoring
- Current Monitoring
- Temperature Monitoring
- Overvoltage Protection
- Undervoltage Protection
- Overcurrent Protection
- Overtemperature Protection
- RTC-Based Auto Recovery
- SD Card Data Logging

## Components

- Arduino Uno R3
- ACS712 Current Sensor
- LM35 Temperature Sensor
- DS3231 RTC
- Relay Module
- AMS1117 Regulator
- SD Card Module

## Working

The Arduino continuously monitors battery voltage, current and temperature. If any parameter exceeds safe limits, the relay disconnects the load. The fault is logged and the system recovers automatically after a preset delay.

## Internship

Developed during Summer Internship Program 2026 conducted by PhiScape Robotics Pvt. Ltd. in association with the Department of EEE, TKM College of Engineering, Kollam.

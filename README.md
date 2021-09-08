# Energy-Meter
This code is a 3 phase meter project that uses ESP32, PZEM 004 v3 hardware, and Blynk app software. It can help to calculate the energy bill and manage 3 different electrical phases.
The different types of tax depending on each country, on specification, it is possible to see what tax this code used. 

Specifications:

1. PZEM Module
    - 110 V to 250 V
    - 0 A to 100 A

2. Tax country
    - Saturday or Sunday
      - tax1: 06:00 to 21:30
      - tax3: 21:30 to 06:00
    - Other days
      - tax1: 06:00 to 17:30 and 20:30 to 21:30
      - tax2: 17:30 to 20:30
      - tax3: 21:30 to 06:00

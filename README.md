# Energy-Meter
    This code is a 3 phase meter project that uses ESP32 to run the code, PZEM 004 v3 hardware to make sensor readings, and Blynk app software to visualize data. It can help to calculate the energy bill and manage 3 different electrical phases.
    There are different types of energy bill taxes depending on each country. Below is possible to see whats taxes this code uses depending on the hour or weekday and other specifications.

Specifications:

1. PZEM Module
    - 90 V to 250 V
    - 0 A to 100 A

2. Taxes country
    - Saturday or Sunday
      - tax1: 06:00 to 21:30
      - tax3: 21:30 to 06:00
    - Other days
      - tax1: 06:00 to 17:30 and 20:30 to 21:30
      - tax2: 17:30 to 20:30
      - tax3: 21:30 to 06:00

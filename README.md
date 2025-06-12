# Thermistor-Temperature-Transmission-via-SCIA-on-TMS320F28379D

To continuously read analog temperature data from a 100k NTC thermistor using the ADC and transmit the computed temperature over SCIA (UART) to a serial 
terminal. 

System Overview:

•	Microcontroller: TMS320F28379D

•	Thermistor: NTC 100kΩ, B = 3950

•	Voltage Divider: 100kΩ fixed resistor with thermistor

•	SCIA Baud Rate: 9600 bps

•	ADC Channel: ADCIN3 (Header Pin J3.26)

•	Math Model: Beta equation (simplified Steinhart-Hart)

•	Output Format: “Temp: XX.YY C” using integer formatting for decimal

![image](https://github.com/user-attachments/assets/2aa86906-9a23-4656-a6c3-5ffe882dc69b)
![image](https://github.com/user-attachments/assets/622bde26-b8aa-4091-88f6-2cde43ede6e0)

Functional Description:

•	The analog voltage from the thermistor voltage divider is read using ADCIN3.

•	The ADC value is converted to voltage and then thermistor resistance.

•	The temperature is calculated using the Beta equation:

T(K) = 1 / (1/T0 + (1/BETA) * ln(R/R0))

ADC Configuration

•	ADC clock prescaler: /4

•	Resolution: 12-bit, single-ended

•	SOC0 triggers ADCIN3 manually

•	Interrupt used to poll for conversion completion

Sample Serial Output:

Thermistor Temp

Temp: 28.56 C

Temp: 28.54 C

Temp: 28.53 C

This output is observed on any UART terminal (e.g., Tera Term, PuTTY) set to 9600 baud, 8N1.

Key Functions

•	adc_to_tempC(): Converts ADC reading to temperature using voltage divider math and Beta equation.

•	print_tempC(): Splits float into integer and fraction to print with two decimal precision using only integer math in snprintf().

#include "driverlib.h"
#include "device.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// === Your thermistor/resistor calibration here ===
#define R_FIXED     100000.0f   // 100k fixed resistor
#define BETA        3950.0f
#define R0          100000.0f   // 100k at 25°C
#define T0_KELVIN   298.15f     // 25°C in Kelvin
#define VREF        3.3f
#define ADC_MAX     4095.0f

char msg[64];

// Converts ADC value to temperature (Celsius) using Beta equation
float adc_to_tempC(uint16_t adc)
{
    float v = ((float)adc / ADC_MAX) * VREF;
    float r_therm = (v * R_FIXED) / (VREF - v); // voltage divider
    float steinhart;
    steinhart = r_therm / R0;         // (R/R0)
    steinhart = logf(steinhart);      // ln(R/R0)
    steinhart /= BETA;                // 1/B * ln(R/R0)
    steinhart += (1.0f / T0_KELVIN);  // + (1/T0)
    steinhart = 1.0f / steinhart;     // Invert
    return steinhart - 273.15f;       // Kelvin to Celsius
}

// Print a float as XX.YY with only integer-based formatting
void print_tempC(float tempC)
{
    int t_int = (int)tempC;
    int t_frac = (int)((tempC - t_int) * 100.0f);
    if (t_frac < 0) t_frac = -t_frac; // Handle negatives
    snprintf(msg, sizeof(msg), "Temp: %d.%02d C\r\n", t_int, t_frac);
    SCI_writeCharArray(SCIA_BASE, (uint16_t *)msg, strlen(msg));
}

void main(void)
{
    uint16_t adcValue;
    float tempC;

    Device_init();
    Device_initGPIO();

    // ==== SCIA RX ====
    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCIRXDA, GPIO_CORE_CPU1);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCIRXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCIRXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_QUAL_ASYNC);

    // ==== SCIA TX ====
    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCITXDA, GPIO_CORE_CPU1);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCITXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCITXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_QUAL_ASYNC);

    // ==== SCIA INIT ====
    SCI_performSoftwareReset(SCIA_BASE);
    SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, 9600,
                  (SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));
    SCI_resetChannels(SCIA_BASE);
    SCI_resetRxFIFO(SCIA_BASE);
    SCI_resetTxFIFO(SCIA_BASE);
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
    SCI_enableFIFO(SCIA_BASE);
    SCI_enableModule(SCIA_BASE);
    SCI_performSoftwareReset(SCIA_BASE);

    // ==== ADC SETUP ====
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_enableConverter(ADCA_BASE);
    DEVICE_DELAY_US(1000);

    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN3, 15);
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);

    SCI_writeCharArray(SCIA_BASE, (uint16_t *)"Thermistor Temp (float math, int print)\r\n", 42);

    while(1)
    {
        ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER0);
        while(!ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1));
        ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

        adcValue = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
        tempC = adc_to_tempC(adcValue);

        print_tempC(tempC);

        DEVICE_DELAY_US(500000);
    }
}

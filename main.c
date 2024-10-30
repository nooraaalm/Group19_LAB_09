#include <stdint.h>
#include "tm4c123gh6pm.h"

#define STCTRL *((volatile uint32_t *) 0xE000E010)        // STATUS AND CONTROL REGISTER
#define STRELOAD *((volatile uint32_t *) 0xE000E014)      // RELOAD VALUE REGISTER
#define STCURRENT *((volatile uint32_t *) 0xE000E018)     // CURRENT VALUE REGISTER

#define ENABLE (1 << 0)                                   // ENABLE TIMER BY SETTING BIT 0 OF CSR
#define CLKINT (1 << 2)                                   // SPECIFY CPU CLOCK
#define CLOCK_HZ 16000000                                 // CLOCK FREQUENCY
#define SYSTICK_RELOAD_VALUE(us) ((CLOCK_HZ / 1000000) * (us) - 1)   // SYSTICK RELOAD VALUE CONVERSION IN MICROSECONDS
void I2C0_init(void) {
    SYSCTL_RCGCI2C_R |= 0x01;                             // I2C0 CLOCK ENABLE
    SYSCTL_RCGCGPIO_R |= 0x02;                            // GPIO PORTB CLOCK ENABLE

    while ((SYSCTL_PRGPIO_R & 0x02) == 0)
    {                                                     // DELAY TO ENSURE CLOCK IS STABILIZED
    };
             // RELOAD VALUE FOR 1mS
  GPIO_PORTB_AFSEL_R |= 0x0C;                           // ENABLE ALERNATE FUNCTION FOR PB2 AND PB3
    GPIO_PORTB_ODR_R |= 0x08;                             // ENABLE OPEN DRAIN ON SDA
    GPIO_PORTB_DEN_R |= 0x0C;                             // ENABLE DIGITAL FOR PB2 AND PB3
    GPIO_PORTB_PCTL_R &= ~0xFF00;                         // CLEAR PCTL
    GPIO_PORTB_PCTL_R |= 0x3300;                          // I2C FUNCTIONALITY FOR PB2 AND PB3

    I2C0_MCR_R = 0x10;                                    // I2C MASTER FUNCTION INITIALIZE
    I2C0_MTPR_R = 0x07;                                   // SET CLOCK
}
void I2C0_SEND(uint8_t PERIPHERAL_ADDRESS, uint8_t ANALOG_SAMPLE_MSB, uint8_t ANALOG_SAMPLE_LSB) // DEFINE I2C0 SEND FUNCTION TO PERIPHERAL ADDRESS AND 12 BIT DATA
{
    I2C0_MSA_R = (PERIPHERAL_ADDRESS << 1);               // SET PERIPHERAL ADDRESS, WRITE
    I2C0_MDR_R = ANALOG_SAMPLE_MSB;                       // MSB
    I2C0_MCS_R = 0x03;                                    // SEND START AND RUN
    while (I2C0_MCS_R & 0x01);                            // WAIT FOR TRANSMISSION TO FINISH
    if (I2C0_MCS_R & 0x02) return;                        // RETURN IF ERROR OCCURS

    I2C0_MDR_R = ANALOG_SAMPLE_LSB;                       // LSB
    I2C0_MCS_R = 0x05;                                    // SEND START AND RUN
    while (I2C0_MCS_R & 0x01);                            // WAIT FOR TRANSMISSION TO FINISH
    if (I2C0_MCS_R & 0x02) return;                        // RETURN IF ERROR OCCURS
}
oid systick_setting(void)
{
    STRELOAD = SYSTICK_RELOAD_VALUE(1000);                // RELOAD VALUE FOR 1mS
    STCTRL |= ENABLE | CLKINT;                            // ENABLE SYSTICK WITH SYSTEM CLOCK
    STCURRENT = 0;                                        // CLEAR STCURRENT REGUSTER
}
void delay(int us)
{
    STRELOAD = SYSTICK_RELOAD_VALUE(us);                  // RELOAD VALUE FOR REQUIRED DELAY
    STCURRENT = 0;                                        // CLEAR STCURRENT
    STCTRL |= ENABLE | CLKINT;                            // ENABLE SYSTICK WITH SYSTEM CLOCK
    while ((STCTRL & (1 << 16)) == 0);                    // WAIT FOR FLAG TO SET
    STCTRL &= ~ENABLE;                                    // STOP TIMER
}

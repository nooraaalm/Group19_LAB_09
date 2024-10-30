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

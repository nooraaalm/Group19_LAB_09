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
#define MCP4725_ADDRESS 0x60                              // PERIPHERAL I2C ADDRESS
void AOUT(uint16_t ANALOG_SAMPLE)
{
    uint8_t ANALOG_SAMPLE_MSB = (ANALOG_SAMPLE >> 8) & 0x0F;               // TAKE UPPER 4 BITS OF 12 BIT DATA TO TRANSMIT
    uint8_t ANALOG_SAMPLE_LSB = ANALOG_SAMPLE & 0xFF;                      // TAKE LOWER 8 BITS OF 12 BIT DATA TO TRANSMIT

    I2C0_SEND(MCP4725_ADDRESS, ANALOG_SAMPLE_MSB, ANALOG_SAMPLE_LSB);      // SEND 12 BIT DATA
}
int samples[100] = {                                                       // DECLARE WAVEFORM SHAPE
                    2252, 2436, 2594, 2722, 2819, 2886, 2925, 2942, 2942, 2933,
                    2922, 2916, 2921, 2940, 2977, 3031, 3101, 3185, 3276, 3371,
                    3463, 3547, 3619, 3675, 3714, 3735, 3739, 3729, 3709, 3682,
                    3652, 3622, 3594, 3570, 3547, 3525, 3499, 3466, 3420, 3356,
                    3271, 3161, 3025, 2863, 2678, 2473, 2255, 2031, 1808, 1596,
                    1401, 1229, 1085,  973,  891,  839,  812,  804,  809,  820,
                     829,  831,  819,  791,  746,  683,  606,  518,  424,  330,
                     241,  162,   98,   51,   21,    9,   12,   27,   51,   80,
                     111,  140,  166,  189,  211,  235,  264,  303,  357,  431,
                     528,  651,  800,  974, 1170, 1382, 1604, 1829, 2047, 2252  };

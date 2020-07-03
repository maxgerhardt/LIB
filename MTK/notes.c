void sim1_to_gpio(unsigned volt_3)
{
    VSIM1_CON2 |= 2;           // 1: SIM1 is controlled by VSIM1_CON0
    VSIM1_CON0 = 0x0400;       // reset value
    VSIM1_CON0 |= volt_3 << 4; // 0:1.8v, 1:3.0v
    VSIM1_CON0 |= 1;           // 1: VSIM1 Enable
}

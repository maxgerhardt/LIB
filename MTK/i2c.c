#include <stddef.h>
#include <stdint.h>

#define I2C_CLOCK_RATE 11000 // 11MHz M66, MC60

void dcl_i2c_set_transaction_speed(void *owner, int mode, uint16_t *Fast_Mode_Speed, uint16_t *HS_Mode_Speed)
{
    uint32_t step_cnt_div, sample_cnt_div, temp;

    //Fast Mode Speed
    for (sample_cnt_div = 1; sample_cnt_div <= 8; sample_cnt_div++)
    {
        if (NULL != Fast_Mode_Speed)
            temp = ((*Fast_Mode_Speed) * 2 * sample_cnt_div);
        else
            temp = 1 * 2 * sample_cnt_div;
        step_cnt_div = (I2C_CLOCK_RATE + temp - 1) / temp; //cast the <1 part

        if (step_cnt_div <= 64)
            break;
    }
    if (step_cnt_div > 64 && sample_cnt_div > 8)
    {
        step_cnt_div = 64;
        sample_cnt_div = 8;
    }

    if (NULL != Fast_Mode_Speed)
    {
        *Fast_Mode_Speed = I2C_CLOCK_RATE / 2 / sample_cnt_div / step_cnt_div;

        // i2c_handle[owner].fs_sample_cnt_div = sample_cnt_div - 1;
        // i2c_handle[owner].fs_step_cnt_div = step_cnt_div - 1;
        // i2c_handle[owner].i2c_config.transaction_mode = 0; //I2C_TRANSACTION_FAST_MODE;
    }

    //HS Mode Speed
    if (mode == 1) //I2C_TRANSACTION_HIGH_SPEED_MODE)
    {
        for (sample_cnt_div = 1; sample_cnt_div <= 8; sample_cnt_div++)
        {
            if (NULL != HS_Mode_Speed)
                temp = ((*HS_Mode_Speed) * 2 * sample_cnt_div);
            else
                temp = (1 * 2 * sample_cnt_div);
            step_cnt_div = (I2C_CLOCK_RATE + temp - 1) / temp;
            if (step_cnt_div <= 8)
                break;
        }
        if (NULL != HS_Mode_Speed)
        {
            *HS_Mode_Speed = I2C_CLOCK_RATE / 2 / sample_cnt_div / step_cnt_div;
            // i2c_handle[owner].hs_sample_cnt_div = sample_cnt_div - 1;
            // i2c_handle[owner].hs_step_cnt_div = step_cnt_div - 1;
            // i2c_handle[owner].i2c_config.transaction_mode = 1; // I2C_TRANSACTION_HIGH_SPEED_MODE;
        }
    } //I2C_TRANSACTION_HIGH_SPEED_MODE
}
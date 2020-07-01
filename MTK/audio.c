#include <hal/hal_audio.h>
#include <hal/hal_ustimer.h>

int SpkAmpGain = 3;
int DG_Microphone = 0x1400;
int DG_DL_Speech = 0x1000;

static inline int AFE_IsKaraok(void) { return 0; }

static inline int AM_GetDLULPath(void) { return DL_UL_BOTH_PATH; }

typedef struct
{
    bool dc_calibrate_finish;
    bool loopback;
    bool mute; /// for mute AFE
    bool refresh;

    uint8_t sidetone_volume;
    bool sidetone_disable;

    uint8_t mic_src;
    uint8_t mic_volume;
    bool mic_mute;

    bool hs_on;
    bool hp_on;
} AFE_STRUCT_T;
AFE_STRUCT_T afe;

#ifdef __BT_BOX_SUPPORT__
const unsigned char extPA_underHP = 1;
#else
const unsigned char extPA_underHP = 0;
#endif

/*



*/

//=================================================================================================
//Clock related
//used from AFE_Manager()
//=================================================================================================

void _AFE_DCLK_CtrlSeq(bool turnon, bool audio)
{
    uint32_t savedMask = DisableInterrupts();

    if (turnon)
    {
        *(volatile uint16_t *)(0xA001022C) = (*(volatile uint16_t *)(0xA001022C) | 0x0013);

        ABBA_AUDIODL_CON8 |= (RG_LCLDO_TBST_EN);
        ABBA_AUDIODL_CON10 = 0x1406; //LDO:2.4v

        ////#ifdef __LINE_IN_SUPPORT__ #endif

        //*ABBA_AUDIODL_CON9  |= 0x0001;
        ABBA_AUDIODL_CON11 |= 0x0003;
        delay_u(1);
        if (audio)
        {
            ABBA_AUDIODL_CON7 |= 0x0003;
        }
        else
        {
            ABBA_AUDIODL_CON7 |= 0x0002;
        }

        ABBA_AUDIODL_CON0 |= (RG_AUDDACRPWRUP | RG_AUDDACLPWRUP);
        ABBA_AUDIODL_CON15 = 0x11A1;
        ABBA_AUDIODL_CON14 |= 0x0001;

        MD2GSYS_CG_CLR2 = PDN_CON2_AAFE; //equal PDN_CON2_VAFE (MD2GCONFG_base+0x028)

        if (audio)
        {

            if (AFE_IsKaraok()) //dl + ul
            {
                AFE_MCU_CON1 |= 0x000F;
            }
            else
            {
                AFE_MCU_CON1 |= 0x000C;
            }
            AFE_AMCU_CON0 |= 0x0001;
        }
        else
        {
            if (AFE_IsKaraok() || (AM_GetDLULPath() == DL_UL_BOTH_PATH) || afe.loopback) //dl + ul
            {
                AFE_MCU_CON1 |= 0x000F;
            }
            else if (AM_GetDLULPath() == DL_PATH)
            {
                AFE_MCU_CON1 |= 0x000C;
            }
            else //ul only
            {
                AFE_MCU_CON1 |= 0x0003;
            }
            AFE_VMCU_CON0 |= 0x0001;
        }
        AFE_MCU_CON0 = 0x0001;
    }
    else
    {
        ABBA_AUDIODL_CON0 &= (~(RG_AUDDACRPWRUP | RG_AUDDACLPWRUP));
        ABBA_AUDIODL_CON7 &= (~0x0003);
        ABBA_AUDIODL_CON11 &= (~0x0003);
        AFE_MCU_CON0 &= (~0x0001);
        if (audio)
        {
            AFE_AMCU_CON0 &= (~0x0001);
        }
        else
        {
            AFE_VMCU_CON0 &= (~0x0001);
        }
        AFE_MCU_CON1 &= (~0x000F);

        MD2GSYS_CG_SET2 = PDN_CON2_VAFE; //equal PDN_CON2_AAFE (MD2GCONFG_base+0x018)

        ABBA_AUDIODL_CON14 &= (~0x0001);
        ABBA_AUDIODL_CON15 = 0x1021;

        //*ABBA_AUDIODL_CON9  &= (~0x0001);
        *(volatile uint16_t *)(0xA001022C) = (*(volatile uint16_t *)(0xA001022C) & (~0x0010));
    }
    
    RestoreInterrupts(savedMask);
}

//=================================================================================================
//used from AFE_Manager()
//=================================================================================================
static void _AFE_Switch_IntAmp(bool on) // 1: turn on, 0: turn off
{
    uint16_t restore;
    if (extPA_underHP)
    {
        return;
    }

    if (on)
    {

#if !defined(__CLASSK_CP_SUPPORT__)
        //off -> bypass
        //spk initial setting
        SPK_CON3 = 0x4800;
        SPK_CON6 = 0x0988; //bypass mode
        SPK_CON7 = 0x0180; //vcm fast startup
        delay_u(2000);     //wait 2ms
        SPK_CON7 = 0x0100;
#endif

        ABBA_AUDIODL_CON0 |= RG_AUDHSPWRUP; //HS buffer power on
        delay_u(10);
        ABBA_AUDIODL_CON1 |= 0x0004; // spk mode
        // for adjust gain with ramp
        ABBA_AUDIODL_CON12 &= (~audzcdenable);

        //set to 0dB
        restore = ABBA_AUDIODL_CON13;
        ABBA_AUDIODL_CON13 = (restore & ~0x7C00) | 0x2000; // Default 0dB => [14:10] = 01000

#if __SPK_DC_COMPENSATION__
        //enable compensation
        {
            *AFE_AMCU_CON2 |= 0x8000;
            *AFE_VMCU_CON2 |= 0x8000;
            *AFE_AMCU_CON6 = afe.spk_dc_compensate_value;
            *AFE_AMCU_CON7 = afe.spk_dc_compensate_value;
            *AFE_VMCU_CON4 = afe.spk_dc_compensate_value;
        }
#endif

        SPK_CON7 |= 0x0080; // fast VCM track
        SPK_CON0 |= 0x0001; // enable spk
        delay_u(2000);
        SPK_CON3 |= 0x4000;
        SPK_CON7 &= (~0x0080); // vcm high PSRR mode

        int16_t targ_gain = (int16_t)(restore & 0x7C00);
        int16_t curr_gain = (int16_t)(ABBA_AUDIODL_CON13 & 0x7C00);
        int16_t gain_step = targ_gain >= curr_gain ? 0x0400 : -0x0400;
        while (curr_gain != targ_gain)
        {
            curr_gain += gain_step;
            ABBA_AUDIODL_CON13 = (uint16_t)curr_gain | (restore & ~0x7C00);
            delay_u(500);
        }

        ABBA_AUDIODL_CON1 |= 0x6000; // ZCD: handset mode
        ABBA_AUDIODL_CON12 |= (audzcdenable);
    }
    else
    {
        ABBA_AUDIODL_CON1 &= (~0x0004);
        SPK_CON3 |= 0x0800;
        SPK_CON0 &= (~0x0001);
        SPK_CON3 &= (~0x4000);
        SPK_CON7 &= (~0x0400);
#if __SPK_DC_COMPENSATION__
        //disable compensation
        {
            *AFE_AMCU_CON2 &= ~0x8000;
            *AFE_VMCU_CON2 &= ~0x8000;
        }
#endif
    }
}

//=================================================================================================
//=================================================================================================

void AFE_TurnOnLoopback(void)
{
    AFE_VLB_CON |= 0x0022; // for digital loopback
    AFE_VLB_CON |= 0x0008;
    afe.loopback = true;
    afe.refresh = true;
}

void AFE_TurnOffLoopback(void)
{
    AFE_VLB_CON &= (~0x62); // for digital loopback
    AFE_VLB_CON &= (~0x0008);
    afe.loopback = false;
    afe.refresh = true;
}

//=================================================================================================
//=================================================================================================

void AFE_SetSamplingRate(uint32_t freq)
{
    uint16_t SampleRate;
    switch (freq)
    {
    case ASP_FS_8K:
        SampleRate = 0;
        break;
    case ASP_FS_11K:
        SampleRate = 1;
        break;
    case ASP_FS_12K:
        SampleRate = 2;
        break;
    case ASP_FS_16K:
        SampleRate = 4;
        break;
    case ASP_FS_22K:
        SampleRate = 5;
        break;
    case ASP_FS_24K:
        SampleRate = 6;
        break;
    case ASP_FS_32K:
        SampleRate = 8;
        break;
    case ASP_FS_44K:
        SampleRate = 9;
        break;
    case ASP_FS_48K:
        SampleRate = 10;
        break;
    default:
        SampleRate = 0;
        break;
    }
    AFE_AMCU_CON1 = ((AFE_AMCU_CON1) & (~0x03C0)) | (SampleRate << 6);

    //afe.audio_fs = freq;
}

//=================================================================================================
//=================================================================================================

void AFE_Chip_Init(void)
{
    //Digital part Initialization
    AFE_AMCU_CON1 = 0x0E00;
    AFE_AMCU_CON5 = 0x0002;
    AFE_VMCU_CON3 = 0x0002;
    AFE_VMCU_CON2 = 0x083C;
    AFE_AMCU_CON2 = 0x003C;
    AFE_VMCU_CON1 = 0x0080;

    //Uplink PGA Gain : 6dB
    ABBA_VBITX_CON0 |= (0x2 << 6);

    //Analog part Initialization and power-on control sequence
    ABBA_AUDIODL_CON8 |= (RG_LCLDO_TBST_EN);
    ABBA_AUDIODL_CON4 = 0x01B9;

    delay_u(10);

    ABBA_AUDIODL_CON4 |= 0x0040;
    // should wait 2s~3s to charge cap
    ABBA_AUDIODL_CON10 = 0x1406; //LDO:2.4v
    ABBA_AUDIODL_CON9 |= 0x0001;

    //Speaker Amp setting
    SPK_CON0 = ((SPK_CON0 & 0xFFCF) | (SpkAmpGain << 4));

    //AFE_MIC_Bias_Set();

    //afe.class_d_gain = SpkAmpGain;
    //DP_VOL_IN_PCM = DG_Microphone;
    //DP_VOL_OUT_PCM = DG_DL_Speech;

    //PcmSink_SetGain(DG_DAF);
    //PcmSink_InitDigitalGain();

    //DP_AUDIO_16K_TX_VOL = DG_Microphone;
    //DP_AUDIO_16K_RX_VOL = DG_DL_Speech; // 16K WB DownLink path digital gain

    //afe.dc_calibrate_writeback = KAL_FALSE;
    //afe.bReceiverOut = KAL_FALSE;
    //afe.dc_cali_new_callback = KAL_FALSE;
    //afe.retrieved_dc_data = 0;
#if __AFE_CURRENT_SENSOR_ENABLE__
    //afe.cali_cf_data = 0;
    //afe.vib_cali_done = KAL_FALSE;
    //afe.vib_first_cali = KAL_TRUE;
#endif
    //afe.bPowerDown = KAL_TRUE;
    //afe.KaraokMode = KAL_FALSE;
    //afe.powerOffWait = -1;
}

//==============================================
//      Set volume / gain control
//==============================================
static const uint16_t SwAgc_Gain_Map[42] =
    {
        23, 22, 21, 20, 19, 18, 17, 16, 15,
        14, 13, 12, 11, 10, 9,
        14, 13, 12, 11, 10, 9,
        14, 13, 12, 11, 10, 9,
        14, 13, 12, 11, 10, 9,
        8, 7, 6, 5, 4, 3, 2, 1, 0};

static const uint16_t HwPga_Gain_Map[4] =
    {
        3, // +18dB
        2, // +12dB
        1, // +6dB
        0  // 0dB
};

static const uint16_t SideToneTable[] = {
    32767, 26027, 20674, 16422, 13044, 10361, 8230, 6537, // 2 dB per step
    5193, 4125, 3276, 2602, 2066, 1641, 1304, 1035,       //
    822, 653, 519, 412, 327, 260, 206, 164                //
};

void UpdateSidetone(int8_t vol)
{
    int8_t J;
    uint32_t R;
    uint16_t sidetone_vol = -1;
    if (afe.sidetone_disable || afe.sidetone_volume == 0)
    {
        MEMORY[0x82207AF6] = 0; //DP_SIDETONE_VOL
        return;
    }
    sidetone_vol = afe.sidetone_volume;
    {
        int16_t i, j;
        uint8_t agc_ul_gain = 0;

        i = afe.mic_volume + 10;
        j = 63 - i;
        if (j > 41)
            j = 41;

        agc_ul_gain = SwAgc_Gain_Map[j];

        i = agc_ul_gain;
        i = i << 3;
        sidetone_vol += i;

        //trace("[AFE] Sidetone %d %d", afe.sidetone_volume, sidetone_vol);
        if (sidetone_vol > 255)
            sidetone_vol = 255;
    }
    R = (uint32_t)(sidetone_vol & 0xF);
    vol = vol + 1 - (int8_t)(sidetone_vol >> 4);
    if (vol < 0)
        vol = 0;
    if (vol > 23)
        vol = 23;
    J = (vol == 0) ? 0 : vol - 1;
    //DP_SIDETONE_VOL
    MEMORY[0x82207AF6] = (uint16_t)(((uint32_t)SideToneTable[vol] * (16 - R) + (uint32_t)SideToneTable[J] * R) >> 4);
}

#if 0
static void AFE_SwitchHPon(void)
{

    uint16_t restore, tmp;
    ABBA_AUDIODL_CON11 |= 0x0003;
    ABBA_AUDIODL_CON7 |= 0x0003;
    ABBA_AUDIODL_CON0 |= (RG_AUDDACRPWRUP | RG_AUDDACLPWRUP);
    ABBA_AUDIODL_CON1 &= (~0x6000);
    ABBA_AUDIODL_CON1 |= 0x4000; // enable HP zcd
    restore = ABBA_AUDIODL_CON13;
    ABBA_AUDIODL_CON13 = (restore & ~0x03FF) | 0x02F7; // set to 0dB and enable zcd
    ABBA_AUDIODL_CON12 |= (audzcdenable);

    ABBA_AUDIODL_CON0 |= RG_AUDHSPWRUP; //HS buffer power on [must]
    delay_u(10);
    ABBA_AUDIODL_CON5 |= 0x0001;
    delay_u(10);
    ABBA_AUDIODL_CON0 |= 0x0001;
    delay_u(10);
    ABBA_AUDIODL_CON2 |= 0x0001;
    ABBA_AUDIODL_CON0 |= 0x000C;
    ABBA_AUDIODL_CON2 &= (~0x0001);
    //delay_u(5);
    tmp = ABBA_AUDIODL_CON1 & (~0x1FF0); // to avoid set path pop
    ABBA_AUDIODL_CON1 = tmp | 0x0120;
    ABBA_AUDIODL_CON5 &= (~0x0001);
    ABBA_AUDIODL_CON0 &= (~RG_AUDHSPWRUP);
    ABBA_AUDIODL_CON4 &= (~(RG_ADEPOP_EN | RG_DEPOP_VCM_EN));
    ABBA_AUDIODL_CON13 = restore;

    afe.hp_on = true;
}

static void AFE_SwitchHSon(void)
{
    uint16_t restore;
    ABBA_AUDIODL_CON11 |= 0x0003;
    ABBA_AUDIODL_CON7 |= 0x0003;
    ABBA_AUDIODL_CON0 |= (RG_AUDDACRPWRUP | RG_AUDDACLPWRUP);
    ABBA_AUDIODL_CON1 &= (~0x6000);
    ABBA_AUDIODL_CON1 |= 0x6000; // enable HS zcd
    restore = ABBA_AUDIODL_CON13;
    ABBA_AUDIODL_CON13 = (restore & ~0x7C00) | 0x2000; // set to 0dB and enable zcd
    ABBA_AUDIODL_CON12 |= (audzcdenable);
    ABBA_AUDIODL_CON13 = restore;
    ABBA_AUDIODL_CON1 &= (~0x1FF0);
    ABBA_AUDIODL_CON1 |= 0x0800;
    ABBA_AUDIODL_CON0 |= RG_AUDHSPWRUP; //HS buffer power on

#if 1 //HP_EN_UNDERSPK
    if (afe.dc_calibrate_finish)
    // force to turn on earphone under spk
    {
        ABBA_AUDIODL_CON2 |= 0x0001;
        ABBA_AUDIODL_CON0 |= 0x000D;
        delay_u(1);
        ABBA_AUDIODL_CON2 &= (~0x0001);
        ABBA_AUDIODL_CON1 |= 0x0090;
    }
#endif
    afe.hs_on = true;
}
#endif

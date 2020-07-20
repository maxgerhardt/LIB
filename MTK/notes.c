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
    ABBA_VBITX_CON0 |= (0x2 << 6); //0x80
    //Analog part Initialization and power-on control sequence
    ABBA_AUDIODL_CON8 |= (RG_LCLDO_TBST_EN);
    ABBA_AUDIODL_CON4 = 0x01B9;
    delay_u(10);
    ABBA_AUDIODL_CON4 |= 0x0040;
    // should wait 2s~3s to charge cap
    ABBA_AUDIODL_CON10 = 0x1406; // LDO:2.4v
    ABBA_AUDIODL_CON9 |= 0x0001;
    //Speaker Amp setting
    SPEAKER_CON0 = ((SPEAKER_CON0 & 0xFFCF) | (3 << 4)); // SpkAmpGain << 4
    //printf("[AFE] INIT\n");
}

static void AFE_SwitchHPon(void) //<---
{
    short restore, tmp;
    ABBA_AUDIODL_CON11 |= 0x0003;
    ABBA_AUDIODL_CON7 |= 0x0003;
    ABBA_AUDIODL_CON0 |= (RG_AUDDACRPWRUP | RG_AUDDACLPWRUP);
    ABBA_AUDIODL_CON1 &= (~0x6000);
    ABBA_AUDIODL_CON1 |= 0x4000; // enable HP zcd
    restore = ABBA_AUDIODL_CON13;
    ABBA_AUDIODL_CON13 = (restore & ~0x03FF) | 0x02F7; // set to 0dB and enable zcd
    ABBA_AUDIODL_CON12 |= (audzcdenable);
    ABBA_AUDIODL_CON0 |= RG_AUDHSPWRUP; //HS buffer power on [must]
    delay_m(10);
    ABBA_AUDIODL_CON5 |= 0x0001;
    delay_m(10);
    ABBA_AUDIODL_CON0 |= 0x0001;
    delay_m(10);
    ABBA_AUDIODL_CON2 |= 0x0001;
    ABBA_AUDIODL_CON0 |= 0x000C;
    ABBA_AUDIODL_CON2 &= (~0x0001);
    //delay_m(5);
    tmp = ABBA_AUDIODL_CON1 & (~0x1FF0); // to avoid set path pop
    ABBA_AUDIODL_CON1 = tmp | 0x0120;
    ABBA_AUDIODL_CON5 &= (~0x0001);
    ABBA_AUDIODL_CON0 &= (~RG_AUDHSPWRUP);
    ABBA_AUDIODL_CON4 &= (~(RG_ADEPOP_EN | RG_DEPOP_VCM_EN));
    ABBA_AUDIODL_CON13 = restore;
}

void audio_test(void)
{
    AFE_Chip_Init();
    AFE_SwitchHPon();
    AFE_TurnOn8K(); // voice dac
    AFE_DAC_TEST = (1<<15) + (7 << 8) + 2; // test over voice, 7 amp, freq[8]
}

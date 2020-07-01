/*

*/

#ifndef _AFE_H_
#define _AFE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <hal/hal.h>

#define  AFE_VMCU_CON0      (*(volatile uint32_t*)(AFE_base+0x0000))       /* AFE MCU Control Register */
#define  AFE_VMCU_CON1      (*(volatile uint32_t*)(AFE_base+0x000C))       /* AFE Voice Analog Circuit Control Register 1 */
#define  AFE_VMCU_CON2      (*(volatile uint32_t*)(AFE_base+0x0010))       
#define  AFE_VMCU_CON3      (*(volatile uint32_t*)(AFE_base+0x001C)) 
#define  AFE_VDB_CON        (*(volatile uint32_t*)(AFE_base+0x0014))       /* AFE Voice DAI Blue Tooth Control Register */
#define  AFE_VLB_CON        (*(volatile uint32_t*)(AFE_base+0x0018))       /* AFE Voice Loop-back Mode Control Register */
#define  AFE_AMCU_CON0      (*(volatile uint32_t*)(AFE_base+0x0020))       /* AFE Audio MCU Control Register Register */
#define  AFE_AMCU_CON1      (*(volatile uint32_t*)(AFE_base+0x0024)) 
#define  AFE_AMCU_CON2      (*(volatile uint32_t*)(AFE_base+0x002C))
#define  AFE_AMCU_CON3      (*(volatile uint32_t*)(AFE_base+0x0038)) 
#define  AFE_AMCU_CON4      (*(volatile uint32_t*)(AFE_base+0x003C))      
#define  AFE_EDI_CON        (*(volatile uint32_t*)(AFE_base+0x0028))       /* AFE Audio EDI(I2S , EIAJ) Control Register */
#define  AFE_DAC_TEST       (*(volatile uint32_t*)(AFE_base+0x0030))       /* AUDIO/VOICE sinewave generator and others */
#define  AFE_VAM_SET        (*(volatile uint32_t*)(AFE_base+0x0034))       /* Audio/Voice Interactive Mode Setting */

#define  AFE_AMCU_CON5      (*(volatile uint32_t*)(AFE_base+0x0180))
#define  AFE_AMCU_CON6      (*(volatile uint32_t*)(AFE_base+0x0184))
#define  AFE_AMCU_CON7      (*(volatile uint32_t*)(AFE_base+0x0188))

#define  AFE_VMCU_CON4      (*(volatile uint32_t*)(AFE_base+0x01A0))
#define  AFE_VMCU_CON6      (*(volatile uint32_t*)(AFE_base+0x01A8)) 

#define  AFE_MCU_CON0       (*(volatile uint32_t*)(AFE_base+0x0210)) 
#define  AFE_MCU_CON1       (*(volatile uint32_t*)(AFE_base+0x0214))  


#define ABBSYS_SD_base      (0xA0720000) /*Analog baseband (ABB) controller */
#define ABBA_WR_PATH0      (*(volatile uint32_t*)(ABBSYS_SD_base+0x0000))
#define ABBA_VBITX_CON0    (*(volatile uint32_t*)(ABBSYS_SD_base+0x0050))
#define ABBA_VBITX_CON1    (*(volatile uint32_t*)(ABBSYS_SD_base+0x0054))
#define ABBA_VBITX_CON2    (*(volatile uint32_t*)(ABBSYS_SD_base+0x0058))
#define ABBA_VBITX_CON3    (*(volatile uint32_t*)(ABBSYS_SD_base+0x005C))
#define ABBA_VBITX_CON4    (*(volatile uint32_t*)(ABBSYS_SD_base+0x0060)) 
#define ABBA_VBITX_CON5    (*(volatile uint32_t*)(ABBSYS_SD_base+0x0064)) 
#define ABBA_AUDIODL_CON0  (*(volatile uint32_t*)(ABBSYS_SD_base+0x0080)) 
#define ABBA_AUDIODL_CON1  (*(volatile uint32_t*)(ABBSYS_SD_base+0x0084)) 
#define ABBA_AUDIODL_CON2  (*(volatile uint32_t*)(ABBSYS_SD_base+0x0088))
#define ABBA_AUDIODL_CON3  (*(volatile uint32_t*)(ABBSYS_SD_base+0x008C)) 
#define ABBA_AUDIODL_CON4  (*(volatile uint32_t*)(ABBSYS_SD_base+0x0090)) 
#define ABBA_AUDIODL_CON5  (*(volatile uint32_t*)(ABBSYS_SD_base+0x0094)) 
#define ABBA_AUDIODL_CON6  (*(volatile uint32_t*)(ABBSYS_SD_base+0x0098))  
#define ABBA_AUDIODL_CON7  (*(volatile uint32_t*)(ABBSYS_SD_base+0x009C)) 
#define ABBA_AUDIODL_CON8  (*(volatile uint32_t*)(ABBSYS_SD_base+0x00A0)) 
#define ABBA_AUDIODL_CON9  (*(volatile uint32_t*)(ABBSYS_SD_base+0x00A4)) 
#define ABBA_AUDIODL_CON10 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00A8))
#define ABBA_AUDIODL_CON11 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00AC))
#define ABBA_AUDIODL_CON12 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00B0))
#define ABBA_AUDIODL_CON13 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00B4))
#define ABBA_AUDIODL_CON14 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00B8))
#define ABBA_AUDIODL_CON15 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00BC))
#define ABBA_AUDIODL_CON16 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00C0))
#define ABBA_AUDIODL_CON17 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00C4))
#define ABBA_AUDIODL_CON18 (*(volatile uint32_t*)(ABBSYS_SD_base+0x00C8))

   //ABBA_VBITX_CON0
   #define RG_VCFG_1                  (0x1 << 10) 
   #define RG_VPWDB_PGA               (0x1 << 5)   
   //ABBA_VBITX_CON1
   #define RG_VPWDB_ADC               (0x1 << 6)
   #define RG_VREF24_EN               (0x1 << 2)  
   #define RG_VCM14_EN                (0x1 << 1) 
   //ABBA_VBITX_CON4
   #define RG_VPWDB_MBIAS             (0x1 << 1) 
   //ABBA_AUDIODL_CON0   
   #define RG_AUDIBIASPWRUP           (0x1)
   #define RG_AUDHPRPWRUP             (0x1 << 2)
   #define RG_AUDHPLPWRUP             (0x1 << 3)              
   #define RG_AUDHSPWRUP              (0x1 << 4)
   #define RG_AUDDACRPWRUP            (0x1 << 5)
   #define RG_AUDDACLPWRUP            (0x1 << 6)
   //ABBA_AUDIODL_CON4
   #define RG_DEPOP_VCM_EN            (0x1)
   #define RG_DEPOP_CHARGEOPTION      (0x1 << 5)
   #define RG_ADEPOP_EN               (0x1 << 6)
   //ABBA_AUDIODL_CON8 
   #define RG_LCLDO_TBST_EN           (0x1 << 1)  //Audio/Voice TOP bias current from ABB TOP
   //ABBA_AUDIODL_CON10                
   #define RG_AUDHSBIAS               (0x1 << 12)
   //ABBA_AUDIODL_CON12
   #define audzcdenable               (0x1)
   //ABBA_AUDIODL_CON16
   #define RG_AMUTER                  (0x1 << 1)
   #define RG_AMUTEL                  (0x1)   
   //VSBST_CON0 
   #define QI_VSBST_EN                (0x1)   
   #define RG_VSBST_BP                (0x1 << 1) 
   #define NI_VSBST_SSBP              (0x1 << 5) 
   #define VSBST_VIO28_PG_STATUS      (0x1 << 10)
   //VSBST_CON4
   #define QI_VSBST_PG_STATUS         (0x1 << 9)



#define SPEAKER_base        PMU_base
#define SPK_CON0            (*(volatile uint32_t*)(SPEAKER_base + 0x0D00))
#define SPK_CON3            (*(volatile uint32_t*)(SPEAKER_base + 0x0D0C))
#define SPK_CON4            (*(volatile uint32_t*)(SPEAKER_base + 0x0D10))   
#define SPK_CON6            (*(volatile uint32_t*)(SPEAKER_base + 0x0D18))
#define SPK_CON7            (*(volatile uint32_t*)(SPEAKER_base + 0x0D1C))
#define SPK_CON8            (*(volatile uint32_t*)(SPEAKER_base + 0x0D20)) // calibration
#define SPK_CON9            (*(volatile uint32_t*)(SPEAKER_base + 0x0D24)) // current sensor register

#define SPK_OC_FLAG        (0x1 << 14) //SPK_CON0
#define SPK_OC_AUTOFF      (0x1 << 12) //SPK_CON0
#define SPK_CLASSAB_OC_EN  (0x1 << 8)  //SPK_CON7

//=============================================================================================
//=============================================================================================

#define  ASP_FS_8K          0x00
#define  ASP_FS_11K         0x11
#define  ASP_FS_12K         0x12
#define  ASP_FS_16K         0x20
#define  ASP_FS_22K         0x21
#define  ASP_FS_24K         0x22
#define  ASP_FS_32K         0x40
#define  ASP_FS_44K         0x41
#define  ASP_FS_48K         0x42

//=============================================================================================
//=============================================================================================

#define DL_PATH             0
#define UL_PATH             1
#define DL_UL_BOTH_PATH     2

//=============================================================================================
//                  Section: Feature Power Down Control Registers Settings
//=============================================================================================
#define PDN_CON2_VAFE           0x0100
#define PDN_CON2_AAFE           PDN_CON2_VAFE
//#define PDN_CON2_AAFE         0x1000
#define CG_CON2_MIXED           0x0200

#define MD2GSYS_CG_CON0         (*(volatile uint32_t*)(MD2GCONFG_base+0x000)) /* Power Down Control 0 Register */
#define MD2GSYS_CG_CON2         (*(volatile uint32_t*)(MD2GCONFG_base+0x008)) /* Power Down Control 2 Register */
#define MD2GSYS_CG_CON4         (*(volatile uint32_t*)(MD2GCONFG_base+0x030)) /* Power Down Control 4 Register */
#define MD2GSYS_CG_SET0         (*(volatile uint32_t*)(MD2GCONFG_base+0x010)) /* Power Down Disable 0 Register */
#define MD2GSYS_CG_SET2         (*(volatile uint32_t*)(MD2GCONFG_base+0x018)) /* Power Down Disable 2 Register */
#define MD2GSYS_CG_SET4         (*(volatile uint32_t*)(MD2GCONFG_base+0x034)) /* Power Down Disable 4 Register */
#define MD2GSYS_CG_CLR0         (*(volatile uint32_t*)(MD2GCONFG_base+0x020)) /* Power Down Enable  0 Register */
#define MD2GSYS_CG_CLR2         (*(volatile uint32_t*)(MD2GCONFG_base+0x028)) /* Power Down Enable  2 Register */
#define MD2GSYS_CG_CLR4         (*(volatile uint32_t*)(MD2GCONFG_base+0x038)) /* Power Down Enable  4 Register */

#ifdef __cplusplus
}
#endif
#endif /* _AFE_H_ */

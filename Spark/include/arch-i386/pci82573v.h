/*
 * pci82573v.h 
 *
 */

#ifndef _PCI82573v_H_
#define _PCI82573v_H_

#include <rtl_core.h>
#include <rtl_pci.h>

//#define PCIETH_DEBUG
#ifdef PCIETH_DEBUG
#define pcieth_debugpr(format, args...)        rtl_printf(format, ## args)
#else
#define pcieth_debugpr(format, args...)        do { ; } while (0)
#endif

#define ETH_DEBUG
#ifdef  ETH_DEBUG
#define eth_debugpr(format, args...)        rtl_printf(format, ## args)
#else
#define eth_debugpr(format, args...)        do { ; } while (0)
#endif

#define _INB(hw, reg)       inb((hw)->ioaddr+reg)
#define _OUTB(hw, val, reg) outb(val, (hw)->ioaddr+reg)

#define TX_QUEUE_NO         1   
#define TCR_CHAIN           0x8000
#define MAX_DESC_STORAGE    4096
#define INIT_POOL_DMA       0

/* Error Codes */
#define E1000_SUCCESS       0
#define MAC_ADDR_SIZE       6

/* Revision ID */     
#define REV_ID_VT86C100A_E  0x04
#define REV_ID_VT3071_A     0x20
#define REV_ID_VT3071_B     0x21
#define REV_ID_VT6102_A     0x40
#define REV_ID_VT6102_C     0x42
#define REV_ID_VT6105_A0    0x80
#define REV_ID_VT6105_B0    0x83
#define REV_ID_VT6105_LOM   0x8A
#define REV_ID_VT6105_OUR   0x8B
#define REV_ID_VT6107_A0    0x8C
#define REV_ID_VT6107_A1    0x8D
#define REV_ID_VT6105M_A0   0x90
#define REV_ID_VT6105M_B1   0x94

#define PCI_REVISION_ID         0x08    /* Revision ID */
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c
#define PCI_SUBSYSTEM_ID        0x2e

#ifndef RHINE_VERSION
#define RHINE_VERSION       "5.08"
#endif 

/* Support Adapter Name */
#define RHINE_FULL_DRV_NAM      "VIA Rhine Family Fast Ethernet Adapter Driver"


/* physical address */
#define MAC_REG_PAR         0x00        
#define MAC_REG_RCR         0x06        
#define MAC_REG_TCR         0x07        
#define MAC_REG_IMR         0x0E        
#define MAC_REG_BCR0        0x6E        
#define MAC_REG_BCR1        0x6F        
#define MAC_REG_CFGB        0x79      
#define MAC_REG_WOLCG_SET   0xA3    
#define MAC_REG_CFGD        0x7B        
#define MAC_REG_CUR_RD_ADDR 0x18        
#define MAC_REG_CUR_TD_ADDR 0x1C         
#define MAC_REG_CR0         0x08        
#define MAC_REG_FLOWCR0     0x98
#define MAC_REG_FLOWCR1     0x99
#define MAC_REG_PAUSE_TIMER 0x9A
#define MAC_REG_MISC_CR0    0x80    
#define MAC_REG_MISC_CR1    0x81    
#define MAC_REG_MIMR        0x86    
#define MAC_REG_SOFT_TIMER0 0x9C    
#define MAC_REG_ISR         0x0C
#define MAC_REG_MISR        0x84
#define MAC_REG_MIISR       0x6D
#define MAC_REG_MIIAD       0x71
#define MAC_REG_MIICR       0x70
#define MAC_REG_MIIDATA     0x72
#define MAC_REG_WOLCR_CLR   0xA4
#define MAC_REG_TSTREG_CLR  0xA6
#define MAC_REG_PWRCSR1_CLR 0xAD
#define MAC_REG_PWRCSR_CLR  0xAC
#define MAC_REG_WOLCG_CLR   0xA7
#define MAC_REG_STICKHW     0x83
#define MAC_REG_CR1         0x09
#define MAC_REG_EECSR       0x74         
#define MAC_REG_TQWK        0x0A
#define MAC_REG_CFGA        0x78
#define MAC_REG_MAR         0x10


/* Bits in the EECSR register */
#define EECSR_EEPR          0x80        /* eeprom programed status, 73h means programed */
#define EECSR_EMBP          0x40        /* eeprom embeded programming */
#define EECSR_AUTOLD        0x20        /* eeprom content reload */
#define EECSR_DPM           0x10        /* eeprom direct programming */


/* Bits in STICKHW */
#define STICKHW_LEGWOLEN        0x0080  /* status for software reference */
#define STICKHW_LEGACY_WOLSR    0x0008
#define STICKHW_LEGACY_WOLEN    0x0004
#define STICKHW_DS1_SHADOW      0x0002  /* R/W by software/cfg cycle */
#define STICKHW_DS0_SHADOW      0x0001  /* suspend well DS write port */

/* Bits in WOLCFG */
#define WOLCFG_PME_OVR          0x80    /* for legacy use, force PMEEN always */
#define WOLCFG_SFDX             0x40    /* full duplex while in WOL mode */


/* Bits in the MIISR register */
#define MIISR_N_FDX         0x04
#define MIISR_LNKFL         0x02
#define MIISR_SPEED         0x01


/* Bits in the MIICR register */
#define MIICR_MAUTO         0x80
#define MIICR_RCMD          0x40
#define MIICR_WCMD          0x20
#define MIICR_MDPM          0x10
#define MIICR_MOUT          0x08
#define MIICR_MDO           0x04
#define MIICR_MDI           0x02
#define MIICR_MDC           0x01


/* Registers in the MII (offset unit is WORD) */
#define MII_REG_BMCR        0x00        /* physical address */
#define MII_REG_BMSR        0x01
#define MII_REG_PHYID1      0x02        /* OUI */
#define MII_REG_PHYID2      0x03        /* OUI + Module ID + REV ID */
#define MII_REG_ANAR        0x04
#define MII_REG_ANLPAR      0x05
#define MII_REG_MODCFG      0x10
/* NS, MYSON only  */
#define MII_REG_PCR         0x17
/* ESI only  */
#define MII_REG_PCSR        0x17


/* Bits in the ANAR register */
#define ANAR_ASMDIR         0x0800      /* Asymmetric PAUSE support */
#define ANAR_PAUSE          0x0400      /* Symmetric PAUSE Support */
#define ANAR_T4             0x0200
#define ANAR_TXFD           0x0100
#define ANAR_TX             0x0080
#define ANAR_10FD           0x0040
#define ANAR_10             0x0020

/* Bits in the ANLPAR register */
#define ANLPAR_ASMDIR       0x0800      /* Asymmetric PAUSE support */
#define ANLPAR_PAUSE        0x0400      /* Symmetric PAUSE Support */
#define ANLPAR_T4           0x0200
#define ANLPAR_TXFD         0x0100
#define ANLPAR_TX           0x0080
#define ANLPAR_10FD         0x0040
#define ANLPAR_10           0x0020


/* Bits in the BMCR register */
#define BMCR_RESET          0x8000
#define BMCR_LBK            0x4000
#define BMCR_SPEED          0x2000
#define BMCR_AUTO           0x1000
#define BMCR_PD             0x0800
#define BMCR_ISO            0x0400
#define BMCR_REAUTO         0x0200
#define BMCR_FDX            0x0100


/* Bits in the ISR (MISR) register */
#define ISR_GENI            0x00008000UL    /*  for 6102 */
#define ISR_SRCI            0x00004000UL
#define ISR_ABTI            0x00002000UL
#define ISR_NORBF           0x00001000UL
#define ISR_PKTRA           0x00000800UL
#define ISR_OVFI            0x00000400UL
#define ISR_UDFI            0x00000200UL    /*  for 6102 */
#define ISR_ERI             0x00000100UL
#define ISR_CNT             0x00000080UL
#define ISR_BE              0x00000040UL
#define ISR_RU              0x00000020UL
#define ISR_TU              0x00000010UL
#define ISR_TXE             0x00000008UL
#define ISR_RXE             0x00000004UL
#define ISR_PTX             0x00000002UL
#define ISR_PRX             0x00000001UL


/* Bits in MISR */
#define ISR_PWEINT          0x00800000UL    /* power event report in test mode */
#define ISR_UDPINT_CLR      0x00400000UL    /* userdefined, host driven interrupt.clear */
#define ISR_UDPINT_SET      0x00200000UL    /* userdefined, host driven interrupt.Set */
#define ISR_SSRCI           0x00100000UL    /* suspend well mii polling status change interrupt */
#define ISR_TDWBRAI         0x00080000UL    /* TD WB queue race */
#define ISR_PHYINT          0x00040000UL    /* PHY state change interrupt, active by PHYINTEN (misc.cr[9]) in normal mode */
 

/* Bits in the TSR1 register */
#define TSR1_TERR           0x8000
#define TSR1_JAB            0x4000      /* jabber condition occured */
#define TSR1_SERR           0x2000
#define TSR1_TBUFF          0x1000
#define TSR1_UDF            0x0800
#define TSR1_CRS            0x0400
#define TSR1_OWC            0x0200      /* late collision */
#define TSR1_ABT            0x0100


/* Bits in the BCR0 register */
#define BCR0_BOOT_MASK      ((unsigned char) 0xC0)
#define BCR0_BOOT_INT19     ((unsigned char) 0x00)
#define BCR0_BOOT_INT18     ((unsigned char) 0x40)
#define BCR0_BOOT_LOCAL     ((unsigned char) 0x80)
#define BCR0_BOOT_BEV       ((unsigned char) 0xC0)

#define BCR0_MED2           0x80        
#define BCR0_LED100M        0x40        
#define BCR0_CRFT2          0x20        
#define BCR0_CRFT1          0x10        
#define BCR0_CRFT0          0x08        
#define BCR0_DMAL2          0x04        
#define BCR0_DMAL1          0x02         
#define BCR0_DMAL0          0x01   

/* Bits in the CR1 register */
#define CR1_SFRST           0x80        /* software reset */
#define CR1_RDMD1           0x40
#define CR1_TDMD1           0x20
#define CR1_REAUTO          0x10        /* for VT6105 */
#define CR1_KEYPAG          0x10
#define CR1_DPOLL           0x08        /* disable rx/tx auto polling */
#define CR1_FDX             0x04        /* full duplex mode */
#define CR1_DISAU           0x02        /* for VT6105 */
#define CR1_ETEN            0x02        /* early tx mode */
#define CR1_EREN            0x01        /* early rx mode */

/* Bits in the TCR register */
#define TCR_IC              0x800000    /* assert interrupt immediately
                                           while descriptor has been send complete */
#define TCR_EDP             0x400000    /* end of packet */
#define TCR_STP             0x200000    /* start of packet */
#define TCR_TCPCK           0x100000    /* request TCP checksum calculation. */
#define TCR_UDPCK           0x080000    /* request UDP checksum calculation. */
#define TCR_IPCK            0x040000    /* request TCP checksum calculation. */
#define TCR_TAG             0x020000    /* Do insert tag */
#define TCR_CRC             0x010000    /* disable CRC generation */
#define TSR_OWN             0x80000000
#define TCR_CHAIN           0x8000


/* Bits in the BCR1 register */
#define BCR1_MED1           0x80        /* for VT6102 */ 
#define BCR1_MED0           0x40        /* for VT6102 */ 
#define BCR1_VIDFR          0x80        /* for VT6105 */ 
#define BCR1_TXQNOBK        0x40        /* for VT6105 */ 
#define BCR1_CTSF           0x20        
#define BCR1_CTFT1          0x10        
#define BCR1_CTFT0          0x08        
#define BCR1_POT2           0x04         
#define BCR1_POT1           0x02        
#define BCR1_POT0           0x01        


/* Bits in the RCR register */
#define RCR_RRFT2           0x80        
#define RCR_RRFT1           0x40         
#define RCR_RRFT0           0x20        
#define RCR_PROM            0x10
#define RCR_AB              0x08 
#define RCR_AM              0x04
#define RCR_AR              0x02
#define RCR_SEP             0x01 


/* Bits in the TCR register */
#define TCR_RTSF            0x80        
#define TCR_RTFT1           0x40        
#define TCR_RTFT0           0x20 
#define TCR_RTGOPT          0x10        
#define TCR_OFSET           0x08        
#define TCR_LB1             0x04        /* loopback[1] */ 
#define TCR_LB0             0x02        /* loopback[0] */ 
#define TCR_PQEN            0x01


/* Bits in the CFGA register */
#define CFGA_EELOAD         0x80        /* enable eeprom embeded and direct programming */
#define CFGA_LED0S0         0x01


/* Bits in the CFGB register */
#define CFGB_QPKTDIS        0x80    


/* Bits in the CFGD register */
#define CFGD_GPIOEN         0x80        
#define CFGD_DIAG           0x40        
#define CFGD_MAGIC          0x10        
#define CFGD_CRADOM         0x08        
#define CFGD_CAP            0x04        
#define CFGD_MBA            0x02        
#define CFGD_BAKOPT         0x01  


/* Bits in WOLCFG */
#define WOLCFG_SAM              0x20    /* accept multicast case reset, default=0 */        
#define WOLCFG_SAB              0x10    /* accept broadcast case reset, default=0 */ 


/* Bits in the CR0 register */
#define CR0_RDMD            0x40        /* rx descriptor polling demand */ 
#define CR0_TDMD            0x20        /* tx descriptor polling demand */ 
#define CR0_TXON            0x10        
#define CR0_RXON            0x08        
#define CR0_STOP            0x04        /* stop MAC, default = 1 */ 
#define CR0_STRT            0x02        /* start MAC */ 
#define CR0_INIT            0x01        /* start init process */ 

#define CR0_SFRST           0x8000      /* software reset */ 
#define CR0_RDMD1           0x4000      
#define CR0_TDMD1           0x2000      
#define CR0_KEYPAG          0x1000      
#define CR0_DPOLL           0x0800      /* disable rx/tx auto polling */ 
#define CR0_FDX             0x0400      /* full duplex mode */ 
#define CR0_ETEN            0x0200      /* early tx mode */ 
#define CR0_EREN            0x0100      /* early rx mode */ 
        
     
/* Bits in the IMR register */
#define IMR_GENM            0x8000      
#define IMR_SRCM            0x4000      
#define IMR_ABTM            0x2000      
#define IMR_NORBFM          0x1000      
#define IMR_PKTRAM          0x0800      
#define IMR_OVFM            0x0400      
#define IMR_ETM             0x0200      
#define IMR_ERM             0x0100      
#define IMR_CNTM            0x0080      
#define IMR_BEM             0x0040      
#define IMR_RUM             0x0020      
#define IMR_TUM             0x0010      
#define IMR_TXEM            0x0008      
#define IMR_RXEM            0x0004      
#define IMR_PTXM            0x0002      
#define IMR_PRXM            0x0001      

#define IMR_TDWBRAI         0x00080000UL 
#define IMR_TM1_INT         0x00020000UL 

                
/* Bits in MISC.CR0 */
#define MISC_CR0_TM0US          0x20
#define MISC_CR0_FDXTFEN        0x10    /* Full-duplex flow control TX enable */
#define MISC_CR0_FDXRFEN        0x08    /* Full-duplex flow control RX enable */
#define MISC_CR0_HDXFEN         0x04    /* Half-duplex flow control enable */ 
#define MISC_CR0_TIMER0_SUSPEND 0x02 
#define MISC_CR0_TIMER0_EN      0x01
            

/* Bits in MISC.CR1 */
#define MISC_CR1_FORSRST        0x40
#define MISC_CR1_VAUXJMP        0x20
#define MISC_CR1_PHYINT         0x02
#define MISC_CR1_TIMER1_EN      0x01


/* Bits in the MODE2 register */
#define MODE2_DISABT        0x40        
#define MODE2_MRDPL         0x08        /* VT6107A1 and above */ 
#define MODE2_MODE10T       0x02        
        
    
/* Bits in the MODE3 register */
#define MODE3_XONOPT        0x80
#define MODE3_TPACEN        0x40
#define MODE3_BACKOPT       0x20
#define MODE3_DLTSEL        0x10
#define MODE3_MIIDMY        0x08
#define MODE3_MIION         0x04


/* Bits in the MIIAD register */
#define MIIAD_MIDLE         0x80
#define MIIAD_MSRCEN        0x40
#define MIIAD_MDONE         0x20
#define MIIAD_MAD0          0x01


/* Bits in the COMMAND register */
#define COMMAND_BUSM        0x04


/* Bits in the FlowCR1 register */
#define FLOWCR1_XHITH1      0x80
#define FLOWCR1_XHITH0      0x40
#define FLOWCR1_XLTH1       0x20
#define FLOWCR1_XLTH0       0x10
#define FLOWCR1_XONEN       0x08
#define FLOWCR1_FDXTFCEN    0x04
#define FLOWCR1_FDXRFCEN    0x02
#define FLOWCR1_HDXFCEN     0x01


/* Bits in the MODE2 register */
#define MODE2_PCEROPT       0x80        /* VT6102 only */
#define MODE2_DISABT        0x40
#define MODE2_MRDPL         0x08        /* VT6107A1 and above */
#define MODE2_MODE10T       0x02


/* Bits in the RSR0 register */
#define RSR0_BUFF           0x80
#define RSR0_FRAG           0x40
#define RSR0_SERR           0x40
#define RSR0_RUNT           0x20
#define RSR0_LONG           0x10
#define RSR0_FOV            0x08
#define RSR0_FAE            0x04
#define RSR0_CRC            0x02
#define RSR0_RERR           0x01


/* Bits in the RSR1 register */
#define RSR1_RXOK           0x8000     /* rx OK */
#define RSR1_VIDHIT         0x4000     /* VID Hit */
#define RSR1_MAR            0x2000     /* MAC accept multicast address packet */
#define RSR1_BAR            0x1000     /* MAC accept broadcast address packet */
#define RSR1_PHY            0x0800     /* MAC accept physical address packet */
#define RSR1_CHN            0x0400     /* chain buffer, always = 1 */ 
#define RSR1_STP            0x0200     /* start of packet */
#define RSR1_EDP            0x0100     /* end of packet */

#define RSR_OWN             0x80000000

#define PQSTS_RXLERR        0x800000
#define PQSTS_SNPTAG        0x400000
#define PQSTS_IPOK          0x200000        /*IP Checkusm validatiaon ok */
#define PQSTS_TUOK          0x100000        /*TCP/UDP Checkusm validatiaon ok */
#define PQSTS_IPKT          0x080000        /*Received an IP packet */
#define PQSTS_TCPKT         0x040000        /*Received a TCP packet */
#define PQSTS_UDPKT         0x020000        /*Received a UDP packet */
#define PQSTS_TAG           0x010000        /*Received a tagged packet */


/* Registers in the PCI configuration space */
#define PCI_REG_COMMAND     0x04        
#define PCI_REG_MODE0       0x50        
#define PCI_REG_FIFOTST     0x51        
#define PCI_REG_MODE2       0x52        
#define PCI_REG_MODE3       0x53        
#define PCI_REG_DELAY_TIMER 0x54        
#define PCI_REG_FIFOCMD     0x56        
#define PCI_REG_FIFOSTA     0x57        
#define PCI_REG_BNRY        0x58        
#define PCI_REG_CURR        0x5A        
#define PCI_REG_FIFO_DATA   0x5C  
        
/* flags for options */
#define     RHINE_FLAGS_TAGGING         0x00000001UL
#define     RHINE_FLAGS_TX_CSUM         0x00000002UL
#define     RHINE_FLAGS_RX_CSUM         0x00000004UL
#define     RHINE_FLAGS_IP_ALIGN        0x00000008UL
#define     RHINE_FLAGS_VAL_PKT_LEN     0x00000010UL
/* flags for driver status */
#define     RHINE_FLAGS_OPENED          0x00010000UL
#define     RHINE_FLAGS_WOL_ENABLED     0x00080000UL
/*flags for capbilities*/
#define     RHINE_FLAGS_TX_ALIGN        0x01000000UL
#define     RHINE_FLAGS_HAVE_CAM        0x02000000UL
#define     RHINE_FLAGS_FLOW_CTRL       0x04000000UL
/* for rhine_set_media_duplex  */
#define     RHINE_LINK_CHANGE           0x00000001UL
#define     RHINE_LINK_UNCHANGE         0x00000002UL
/* flags for MII status  */
#define     RHINE_LINK_FAIL             0x00000001UL
#define     RHINE_SPEED_10              0x00000002UL
#define     RHINE_SPEED_100             0x00000004UL
#define     RHINE_SPEED_1000            0x00000008UL
#define     RHINE_DUPLEX_FULL           0x00000010UL
#define     RHINE_AUTONEG_ENABLE        0x00000020UL
#define     RHINE_FORCED_BY_EEPROM      0x00000040UL
/* flags for driver status */
#define     RHINE_FLAGS_OPENED          0x00010000UL
#define     RHINE_FLAGS_WOL_ENABLED     0x00080000UL

#define AVAIL_TD(hw,q)   ((hw)->sOpts.nTxDescs-((hw)->iTDUsed[(q)]))

/* IMR Mask [4.39] */
/* initial value of IMR */
#define IMR_MASK_VALUE          0x000BD7FFUL                              
/* Except PRXM */
#define IMR_MASK_EXCEPT_PRX     (IMR_MASK_VALUE & (~(unsigned int)IMR_PRXM)) 
/* Except PTXM */
#define IMR_MASK_EXCEPT_PTX     (IMR_MASK_VALUE & (~(unsigned int)IMR_PTXM)) 
/* Except PTXM & PRXM */
#define IMR_MASK_EXCEPT_PTXPRX  (IMR_MASK_VALUE & (~(unsigned int)(IMR_PRXM | IMR_PTXM)))  


#define netif_start_queue(dev)   ( ((dev)->tbusy) =0 )
#define netif_stop_queue(dev)    ( ((dev)->tbusy) =1 )
#define netif_wake_queue(dev)    ( ((dev)->tbusy) =0 )

/* register space access macros */
#define CSR_WRITE_4(hw, val, reg) writel(val, (hw)->hw_addr+reg)
#define CSR_WRITE_2(hw, val, reg) writew(val, (hw)->hw_addr+reg)
#define CSR_WRITE_1(hw, val, reg) writeb(val, (hw)->hw_addr+reg)

#define CSR_READ_4(hw, reg)   readl((hw)->hw_addr+reg)
#define CSR_READ_2(hw, reg)   readw((hw)->hw_addr+reg)
#define CSR_READ_1(hw, reg)   readb((hw)->hw_addr+reg)

#define TX_THRESH_DEF   0
#define RX_THRESH_DEF   0
#define DMA_LENGTH_DEF  0
#define WOL_OPT_DEF     0

#define W_MAX_TIMEOUT       0x3fff

#define WAIT_MAC_TX_OFF(pInfo)   do {} while (BYTE_REG_BITS_IS_ON(&(pInfo)->hw, CR0_TXON,MAC_REG_CR0))

#define ADD_ONE_WITH_WRAP_AROUND(uVar, uModulo) {   \
    if ((uVar) >= ((uModulo) - 1))                  \
        (uVar) = 0;                                 \
    else                                            \
        (uVar)++;                                   \
}


#define BYTE_REG_BITS_SET(hw,x,m,p)    do { CSR_WRITE_1(hw,(CSR_READ_1(hw,(p)) & (~(m))) |(x),(p));} while (0)
#define WORD_REG_BITS_SET(hw,x,m,p)    do { CSR_WRITE_2(hw,(CSR_READ_2(hw,(p)) & (~(m))) |(x),(p));} while (0)
#define DWORD_REG_BITS_SET(hw,x,m,p)   do { CSR_WRITE_4(hw,(CSR_READ_4(hw,(p)) & (~(m)))|(x),(p));}  while (0)

//DEEPTI : To be merged with direct.c
#define PCI_CONF1_ADDRESS(bus, devfn, reg) \
        (0x80000000 | (bus << 16) | (devfn << 8) | (reg & ~3))

//DEEPTI END	

#define PCI_BYTE_REG_BITS_ON(x,i,bus,devfn) do{\
    unsigned long byReg;\
    pci_conf1_read(0, bus, devfn, i, 1, &byReg); \
    (byReg) |= (x);\
    pci_conf1_write(0, bus, devfn, i, 1, byReg); \
    pci_conf1_read(0, bus, devfn, i, 1, &byReg); \
} while (0)
    
#define PCI_BYTE_REG_BITS_OFF(x,i,bus,devfn) do{\
    unsigned long byReg;\
    pci_conf1_read(0, bus, devfn, i, 1, &byReg); \
    (byReg) &= (~(x));\
    pci_conf1_write(0, bus, devfn, i, 1, byReg); \
    pci_conf1_read(0, bus, devfn, i, 1, &byReg); \
} while (0)


#define ETH_FRAME_LEN   1514            /* Max. octets in frame sans FCS */
#define ETH_P_802_2     0x0004          /* 802.2 frames                 */


#define BYTE_REG_BITS_ON(hw,x,p)   do { CSR_WRITE_1(hw,(CSR_READ_1(hw,(p))|(x)),(p));} while (0)
#define WORD_REG_BITS_ON(hw,x,p)   do { CSR_WRITE_2(hw,(CSR_READ_2(hw,(p))|(x)),(p));} while (0)
#define DWORD_REG_BITS_ON(hw,x,p)  do { CSR_WRITE_4(hw,(CSR_READ_4(hw,(p))|(x)),(p));} while (0)

#define BYTE_REG_BITS_IS_ON(hw,x,p) (CSR_READ_1(hw,(p)) & (x))
#define WORD_REG_BITS_IS_ON(hw,x,p) (CSR_READ_2(hw,(p)) & (x))
#define DWORD_REG_BITS_IS_ON(hw,x,p) (CSR_READ_4(hw,(p)) & (x))

#define BYTE_REG_BITS_OFF(hw,x,p)  do { CSR_WRITE_1(hw,(CSR_READ_1(hw,(p)) & (~(x))),(p));} while (0)
#define WORD_REG_BITS_OFF(hw,x,p)  do { CSR_WRITE_2(hw,(CSR_READ_2(hw,(p)) & (~(x))),(p));} while (0)
#define DWORD_REG_BITS_OFF(hw,x,p) do { CSR_WRITE_4(hw,(CSR_READ_4(hw,(p)) & (~(x))),(p));} while (0)


#define cpu_to_le32 __cpu_to_le32 
#define __cpu_to_le32(x) ((unsigned int)(x)) 


/* Cofiguration parameters */
#define RX_DESC_MIN     16
#define RX_DESC_MAX     128
#define RX_DESC_DEF     16

#define TX_DESC_MIN     16
#define TX_DESC_MAX     128
#define TX_DESC_DEF     16 

/* VID_setting[] is used for setting the VID of NIC.
   0: default VID.
   1-4094: other VIDs.
*/
#define VLAN_ID_MIN     0
#define VLAN_ID_MAX     4094
#define VLAN_ID_DEF     0

/* flow_control[] is used for setting the flow control ability of NIC.
   1: hardware deafult(default). Use Hardware default value in ANAR.
   2: disable PAUSE in ANAR.
   3: enable PAUSE in ANAR.
*/
#define FLOW_CNTL_DEF   2
#define FLOW_CNTL_MIN   1
#define FLOW_CNTL_MAX   3

/* txcsum_offload[] is used for setting the transmit checksum offload ability
   of NIC.
   0: disable (Default)
   1: enable
*/
#define TX_CSUM_DEF     0

/* wol_opts[] is used for controlling wake on lan behavior.
   0: Wake up if recevied a magic packet. (Default)
   1: Wake up if link status is on/off.
   2: Wake up if recevied an arp packet.
   4: Wake up if recevied any unicast packet.
   Those value can be sumed up to support more than one option.
*/
#define WOL_OPT_DEF     0
#define WOL_OPT_MIN     0
#define WOL_OPT_MAX     7

/* ValPktLen[] is used for setting the checksum offload ability of NIC.
   0: Receive frame with invalid layer 2 length (Default)
   1: Drop frame with invalid layer 2 length
*/
#define VAL_PKT_LEN_DEF     0

#define INT_WORKS_DEF   32
#define INT_WORKS_MIN   10
#define INT_WORKS_MAX   64

/* tx_thresh[] is used for controlling the transmit fifo threshold.
   0: indicate the txfifo threshold is 128 bytes.
   1: indicate the txfifo threshold is 256 bytes.
   2: indicate the txfifo threshold is 512 bytes.
   3: indicate the txfifo threshold is 1024 bytes.
   4: indicate that we use store and forward
*/
#define TX_THRESH_MIN   0
#define TX_THRESH_MAX   4
#define TX_THRESH_DEF   0

/* rx_thresh[] is used for controlling the receive fifo threshold.
   0: indicate the rxfifo threshold is 64 bytes.
   1: indicate the rxfifo threshold is 32 bytes.
   2: indicate the rxfifo threshold is 128 bytes.
   3: indicate the rxfifo threshold is 256 bytes.
   4: indicate the rxfifo threshold is 512 bytes.
   5: indicate the rxfifo threshold is 768 bytes.
   6: indicate the rxfifo threshold is 1024 bytes.
   7: indicate that we use store and forward
*/
#define RX_THRESH_MIN   0
#define RX_THRESH_MAX   7
#define RX_THRESH_DEF   0

/* DMA_length[] is used for controlling the DMA length
   0: 8 DWORDs
   1: 16 DWORDs
   2: 32 DWORDs
   3: 64 DWORDs
   4: 128 DWORDs
   5: 256 DWORDs
   6: SF(flush till emply)
   7: SF(flush till emply)
*/
#define DMA_LENGTH_MIN  0
#define DMA_LENGTH_MAX  7
#define DMA_LENGTH_DEF  0

/* speed_duplex[] is used for setting the speed and duplex mode of NIC.
   0: indicate autonegotiation for both speed and duplex mode
   1: indicate 100Mbps half duplex mode
   2: indicate 100Mbps full duplex mode
   3: indicate 10Mbps half duplex mode
   4: indicate 10Mbps full duplex mode
   Note:
        if EEPROM have been set to the force mode, this option is ignored
            by driver.
*/
#define MED_LNK_DEF 0
#define MED_LNK_MIN 0
#define MED_LNK_MAX 4

/* enable_tagging[] is used for enabling 802.1Q VID tagging.
   0: disable VID seeting(default).
   1: enable VID setting.
*/
#define TAGGING_DEF     0

/* IP_byte_align[] is used for IP header DWORD byte aligned
   0: indicate the IP header won't be DWORD byte aligned.(Default) .
   1: indicate the IP header will be DWORD byte aligned.
      In some enviroment, the IP header should be DWORD byte aligned,
      or the packet will be droped when we receive it. (eg: IPVS)
*/
#define IP_ALIG_DEF     0

/* rxcsum_offload[] is used for setting the receive checksum offload ability
   of NIC.
   0: disable
   1: enable (Default)
*/
#define RX_CSUM_DEF     1

#define MII_REG_BITS_ON(x,i,hw) do {\
    unsigned short w;\
    rhine_mii_read((hw),(i),&(w));\
    (w)|=(x);\
    rhine_mii_write((hw),(i),(w));\
} while (0)

#define MII_REG_BITS_OFF(x,i,hw) do {\
    unsigned short w;\
    rhine_mii_read((hw),(i),&(w));\
    (w)&=(~(x));\
    rhine_mii_write((hw),(i),(w));\
} while (0)

#define IS_PHY_VT6103(p)    (( ((p)->dwPHYId & 0xFFFFFFF0UL) ==0x010108F20UL)\
                             && ( ((p)->dwPHYId & 0xFUL) >4) )


#define FET_RXCTL_BUFLEN        0x000007ff

static inline unsigned char readb(const volatile void *addr)
{
        return *(volatile unsigned char *) addr;
}
static inline unsigned short readw(const volatile void *addr)
{
        return *(volatile unsigned short *) addr;
}
static inline unsigned int readl(const volatile void *addr)
{
        return *(volatile unsigned int *) addr;
}
static inline void writeb(unsigned char b, volatile void *addr)
{
        *(volatile unsigned char *) addr = b;
}
static inline void writew(unsigned short b, volatile void *addr)
{
        *(volatile unsigned short *) addr = b;
}
static inline void writel(unsigned int b, volatile void *addr)
{
        *(volatile unsigned int *) addr = b;
}

/* Standard well-defined IP protocols.  */
enum 
{
  IPPROTO_IP = 0,       /* Dummy protocol for TCP       */
  IPPROTO_ICMP = 1,     /* Internet Control Message Protocol    */
  IPPROTO_IGMP = 2,     /* Internet Group Management Protocol   */
  IPPROTO_TCP = 6,      /* Transmission Control Protocol    */
  IPPROTO_UDP = 17,     /* User Datagram Protocol       */
  IPPROTO_RSVP = 46,        /* RSVP protocol            */
  IPPROTO_IPV6   = 41,      /* IPv6-in-IPv4 tunnelling      */
};

typedef enum _speed_opt 
{
    SPD_DPX_AUTO     = 0,
    SPD_DPX_100_HALF = 1,
    SPD_DPX_100_FULL = 2,
    SPD_DPX_10_HALF  = 3,
    SPD_DPX_10_FULL  = 4
} SPD_DPX_OPT, *PSPD_DPX_OPT;

typedef enum __rhine_init_type 
{
    RHINE_INIT_COLD  = 0,
    RHINE_INIT_RESET = 1,     
    RHINE_INIT_WOL   = 2 
} RHINE_INIT_TYPE, *PRHINE_INIT_TYPE;

typedef struct pci82573v_device_struct pci82573v_device;
typedef unsigned long dma_addr_t;

typedef struct __rx_desc 
{
    volatile unsigned int                rdesc0;
    volatile unsigned int                rdesc1;
    volatile unsigned int                buff_addr;
    volatile unsigned int                next_desc;
    volatile unsigned int                Reserved[4]; /* 16 bytes */
} __attribute__ ((__packed__))
RX_DESC, *PRX_DESC;
  
typedef struct _tx_desc 
{     
    volatile unsigned int                tdesc0;
    volatile unsigned int                tdesc1;
    volatile unsigned int                buff_addr;
    volatile unsigned int                next_desc;
    volatile unsigned int                Reserved[4]; /* 16 bytes */
} __attribute__ ((__packed__))
TX_DESC, *PTX_DESC;


struct pci82573v_device_struct
{
    void *private;
    int minor;
    char *board_name;
    int attached;

    unsigned int iobase;
    unsigned char *membase;

    unsigned char tbusy;
    
    unsigned char dev_addr[MAC_ADDR_SIZE]; 

}; 

typedef struct __rhine_opt 
{
    int              nRxDescs;       /* Number of RX descriptors */
    int              nTxDescs;       /* Number of TX descriptors */
    int              tx_thresh;      /* Tx Fifo threshold */
    int              rx_thresh;      /* Rx fifo threshold */
    int              DMA_length;     /* DMA length */
    int              flow_cntl;
    int              wol_opts;       /* Wake on lan options */
    unsigned int     flags;
    int         vid;            /* vlan id */
    SPD_DPX_OPT spd_dpx;        /* Media link mode */
    int         int_works;
} OPTIONS, *POPTIONS;

typedef struct 
{
    struct sk_buff*     skb;
    PRX_DESC            curr_desc;
} RHINE_RD_INFO,    *PRHINE_RD_INFO;

typedef struct 
{
    struct sk_buff*     skb;
    unsigned char *     buf;
    PRX_DESC            curr_desc;
} RHINE_TD_INFO,    *PRHINE_TD_INFO;

struct rhine_hw 
{
    long                        memaddr;
    long                        ioaddr;
    unsigned char *             hw_addr;
    unsigned int                dwPHYId;
    unsigned short int          SubSystemID;
    unsigned short int          SubVendorID;

#define AVAIL_TD(hw,q)   ((hw)->sOpts.nTxDescs-((hw)->iTDUsed[(q)]))

    int                         iCurrRDIdx;
    unsigned int                io_size;
    unsigned int                flags;
    unsigned int                IntMask;
    unsigned char               byRevId;
    volatile int                iTDUsed[TX_QUEUE_NO];
    int                         aiCurrTDIdx[TX_QUEUE_NO];
    int                         aiTailTDIdx[TX_QUEUE_NO];
    PTX_DESC                    apTDRings[TX_QUEUE_NO];
    PRX_DESC                    aRDRing;
    OPTIONS                     sOpts;
    int                         nTxQueues;
    unsigned int                rx_buf_sz;
    int                         multicast_limit;
};

typedef struct __rhine_info 
{
    unsigned char               abyIPAddr[4];
    unsigned char *             tx_bufs;

    /* define in rhine_hw.h */
    struct rhine_hw             hw;
    PRHINE_TD_INFO              apTDInfos[TX_QUEUE_NO];
    int                         wol_opts;

    PRHINE_RD_INFO              aRDInfo;

} RHINE_INFO, *PRHINE_INFO;


#define PKT_BUF_SZ           1540
#define ETH_ZLEN             60      /* Min. octets in frame sans FCS */
#define FET_TXSTAT_PQMASK    0x7FFF0000
#define FET_TXCTL_BUFLEN     0x000007ff 

#define CHECKSUM_HW          1
#define CHECKSUM_NONE        0
#define CHECKSUM_UNNECESSARY 2

#define FET_RXSTAT_RXLEN     0x07FF0000

#define htons(A) ((((unsigned short)(A) & 0xff00) >> 8) | \
                  (((unsigned short)(A) & 0x00ff) << 8))


struct sk_buff 
{
    unsigned int            len;
    unsigned char           ip_summed;
    unsigned char           *data;
};


int rhine_initdevice();
long rhine_reset_configure(void);
long rhine_xmit(unsigned char *skb, unsigned int size);
long rhine_get_ip(unsigned char * ifa_addr);
long rhine_isready(void);

/* 1 : VIA Rhine III card operating in INTRRUPT enabled mode *
 * 0 : VIA Rhine III card operating in POLLING          mode
 */
#define ETH_INTR_BASED 0
		
#endif /* _PCI82573v_H_ */


/*
 *  i386_pci82573v.c
 *  Developed at ERTS, IIT Powai
 */

#include <rtl_conf.h>
#include <rtl_devices.h>
#include <rtl_posixio.h>
#include <rtl_sync.h>
#include <arch/pci82573v.h>
#include <rtl_pci.h>
#include <sys/io.h>
#include <arch/vga.h>
#include <arch/rtl_screen.h>
#include <arch/processor.h>
#include <arch/hw_irq.h>
#include <rtl_time.h>
#include <pthread.h>
#include <errno.h>
#include <rtl_printf.h>
#include <guests.h>

#include <arch/circqueue.h>


#define FALSE               0x0
#define TRUE                0x1
#define NO_OF_TX_QUEUES     0x1

#define VIR_POOL_SIZE       (RX_DESC_DEF * sizeof(RX_DESC) + \
                             TX_DESC_DEF * sizeof(TX_DESC)* NO_OF_TX_QUEUES)
#define TX_BUFF_SIZE        (TX_DESC_DEF * PKT_BUF_SZ * NO_OF_TX_QUEUES)
#define RD_INFO_SIZE        ((sizeof(RHINE_RD_INFO)) * RX_DESC_DEF)
#define RX_BUFF_SIZE        (PKT_BUF_SZ * RX_DESC_DEF)
#define TD_INFO_SIZE        ((sizeof(RHINE_TD_INFO)) * \
                             TX_DESC_DEF*NO_OF_TX_QUEUES)
#define SKB_BUFF_SIZE       ((PKT_BUF_SZ + (sizeof(struct sk_buff))))

#define ETH_DEVICE_FOUND    1
#define ETH_DEVICE_RESET    2
#define ETH_DEVICE_READY    3

/* PCI 82573v device structure 
 * All PCI related information stored here
 */
struct pci82573v_device_struct pci82573v_dev;

/* ethernet device structure 
 * All ethernet related information stored here
 */
struct pci_device_info ethdevinfo[1];

struct circ_queue_monitor rxbuff_cq_mon;

PRHINE_INFO         pInfo;
RHINE_INFO          deviceInfo;

unsigned char vir_pool[VIR_POOL_SIZE];
unsigned char spark_tx_buffer[TX_BUFF_SIZE];
unsigned char spark_aRDInfo[RD_INFO_SIZE];
unsigned char spark_rx_buffer[RX_BUFF_SIZE];
unsigned char spark_aTDInfo[TD_INFO_SIZE];
unsigned char skb_recv_buffer[SKB_BUFF_SIZE * RX_DESC_DEF];
unsigned int recv_lengths[RX_DESC_DEF];

void rhine_set_td_own(PTX_DESC pTD);
void rhine_set_rd_own( PRX_DESC pRD);
static unsigned char rhine_alloc_rx_buf(int idx);
void rhine_print_link_status(unsigned int status); 
void rhine_disable_mii_auto_poll(struct rhine_hw *hw);
unsigned int rhine_check_media_mode(struct rhine_hw *hw);
void rhine_enable_flow_control_ability(struct rhine_hw *hw);
void rhine_set_tx_thresh(struct rhine_hw *hw, int tx_thresh); 
ssize_t pci82573v_dev_read (struct rtl_file *, char *, size_t , loff_t *);
ssize_t pci82573v_dev_write (struct rtl_file *, const char*, size_t, loff_t *);

extern void enable_pagebit(void);

extern struct thread_struct guestOS_thread[NUM_OF_GUESTOS+1] ;

/* This is for consistency with Linux standard device interface 
 * Not used by SParK presently
 */
static struct rtl_file_operations rtl_pci82573v_fops = 
{
        NULL,
        pci82573v_dev_read,
        pci82573v_dev_write,
        NULL,
        NULL,
        NULL,
        NULL
};

ssize_t pci82573v_dev_read (struct rtl_file *file, char *buffer, 
                            size_t size, loff_t *offset)
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    return 0;
}

ssize_t pci82573v_dev_write (struct rtl_file *file, const char* buffer,
                             size_t size, loff_t *offset)
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    return 0;
}

unsigned int DEEPTI_TX = 0;
unsigned int DEEPTI_RX = 0;

inline static void usec_delay(int usec)
{
    int i,j;
    for(i=0;i<usec*2;i++)
         j=i^2;
}

void rhine_clearISR(struct rhine_hw *hw) 
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    CSR_WRITE_2(hw, 0xFFFF, MAC_REG_ISR);
    CSR_WRITE_1(hw, 0xDF, MAC_REG_MISR);
}

unsigned int rhine_ReadISR(struct rhine_hw *hw) 
{
    unsigned int status=0;
    pcieth_debugpr("%s \n", __FUNCTION__);
    status=CSR_READ_2(hw, MAC_REG_ISR);
    status |= CSR_READ_2(hw, MAC_REG_MISR) << 16;
    return status;
}

void rhine_WriteISR(unsigned int status,struct rhine_hw *hw)    
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    CSR_WRITE_2(hw, (status & 0xFFFF), MAC_REG_ISR);
    CSR_WRITE_2(hw, (status>>16), MAC_REG_MISR);
}

void rhine_disable_int(struct rhine_hw *hw) 
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    CSR_WRITE_2(hw, 0, MAC_REG_IMR);
    if(hw->byRevId > REV_ID_VT6102_A)
        CSR_WRITE_1(hw, 0, MAC_REG_MIMR);
}

void rhine_enable_int(struct rhine_hw *hw, unsigned int IsrStatus) 
{
    unsigned char        bDeferRx = FALSE;
    unsigned char        bDeferTx = FALSE;
    pcieth_debugpr("%s \n", __FUNCTION__);

    if (IsrStatus & ISR_PRX)
        bDeferRx = TRUE;
    if (IsrStatus & ISR_PTX)
        bDeferTx = TRUE;

    if (bDeferRx && bDeferTx) 
    {
        /* Defer PTXM, PRXM */
        CSR_WRITE_2(hw, (unsigned short)IMR_MASK_EXCEPT_PTXPRX, MAC_REG_IMR);
    }
    else if (bDeferRx && !bDeferTx) 
    {
        /* Only defer PRXM */
        CSR_WRITE_2(hw, (unsigned short)IMR_MASK_EXCEPT_PRX, MAC_REG_IMR);
    }
    else if (!bDeferRx && bDeferTx) 
    {
        /* Only defer PTXM */
        CSR_WRITE_2(hw, (unsigned short)IMR_MASK_EXCEPT_PTX, MAC_REG_IMR);
    }
    else 
    {
        /* No defer */
        CSR_WRITE_2(hw, (unsigned short)IMR_MASK_VALUE, MAC_REG_IMR);
    }
    CSR_WRITE_1(hw, 0x0B, MAC_REG_MIMR);

    /* Set Timer0 */
    if (bDeferTx || bDeferRx)
    {
         pcieth_debugpr("%s %d\n", __FUNCTION__, __LINE__);
         /* [VT6105 above use micro-second timer]
         // VT6105 only, set Timer0 in mini-second resolution */
         BYTE_REG_BITS_ON(hw, MISC_CR0_TM0US, MAC_REG_MISC_CR0);
         /* set delay time to udelay, unit is us */
         CSR_WRITE_2(hw, 1000, MAC_REG_SOFT_TIMER0);
         /* set time0 */
         BYTE_REG_BITS_ON(hw, MISC_CR0_TIMER0_EN, MAC_REG_MISC_CR0);
         BYTE_REG_BITS_OFF(hw, MISC_CR0_TIMER0_SUSPEND, MAC_REG_MISC_CR0);
    }
}

static void rhine_shutdown() 
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    rhine_disable_int(&pInfo->hw);
    CSR_WRITE_1(&pInfo->hw, CR0_STOP, MAC_REG_CR0);
    rhine_disable_mii_auto_poll(&pInfo->hw);
    rhine_clearISR(&pInfo->hw); 
}

void rhine_set_promiscuous(struct rhine_hw* hw)
{
    CSR_WRITE_4(hw, 0xffffffff, MAC_REG_MAR);
    CSR_WRITE_4(hw, 0xffffffff, MAC_REG_MAR+4);
    BYTE_REG_BITS_ON(hw, RCR_AM|RCR_AB|RCR_PROM, MAC_REG_RCR);
}

static void rhine_error(int status) 
{
    unsigned int mii_status;
    pcieth_debugpr("%s\n", __FUNCTION__);

    if (status & ISR_BE) 
    {
        rtl_printf("%s: Hardware fatal error.\n", "VIA Rhine III on SParK");
        rhine_shutdown(); 
        return;
    }
    if (status & ISR_SRCI) 
    {
        mii_status = rhine_check_media_mode(&pInfo->hw);
        rhine_print_link_status(mii_status);

        if ((pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL) && (pInfo->hw.sOpts.flow_cntl == 1))
            rhine_enable_flow_control_ability(&pInfo->hw);

        if (mii_status & RHINE_LINK_FAIL) 
        {
            rtl_printf("%d %s: Link failed.\n", __LINE__, "VIA Rhine III on SParK");
        }
    };
    if (status & ISR_NORBF) 
    {
        rtl_printf("No more receive buffer to be used\n");
    }
    if (status & ISR_OVFI) 
    {
        rtl_printf("Received FIFO Overflow\n");
    }
}

unsigned char rhine_td_own_bit_on(PTX_DESC pTD)
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    return (pTD->tdesc0 & cpu_to_le32(TSR_OWN));
}

/*  Drop the frame */
static void rhine_tx_srv_drop(int idx, int iQNo) 
{
    PTX_DESC    pTD;
    PRHINE_TD_INFO pTDInfo;
    pcieth_debugpr("%s \n", __FUNCTION__);

    WAIT_MAC_TX_OFF(pInfo);

    ADD_ONE_WITH_WRAP_AROUND(idx, pInfo->hw.sOpts.nTxDescs);
    pTD = &(pInfo->hw.apTDRings[iQNo][idx]);
    pTDInfo = &(pInfo->apTDInfos[iQNo][idx]);

    CSR_WRITE_4(&pInfo->hw, pTDInfo->curr_desc,
                MAC_REG_CUR_TD_ADDR+(iQNo*4));
    BYTE_REG_BITS_ON(&pInfo->hw, CR0_TXON, MAC_REG_CR0);

    if( pInfo->hw.flags & RHINE_FLAGS_TAGGING)
         BYTE_REG_BITS_ON(&pInfo->hw, 1 << (7-iQNo), MAC_REG_TQWK);

    BYTE_REG_BITS_ON(&pInfo->hw, CR1_TDMD1, MAC_REG_CR1);
}

/* Re-transmited the frame */
static void rhine_tx_srv_resend(int idx, int iQNo) 
{
    PTX_DESC    pTD = &(pInfo->hw.apTDRings[iQNo][idx]);
    PRHINE_TD_INFO pTDInfo = &(pInfo->apTDInfos[iQNo][idx]);
    pcieth_debugpr("%s \n", __FUNCTION__);

    WAIT_MAC_TX_OFF(pInfo);

    rhine_set_td_own(pTD);

    CSR_WRITE_4(&pInfo->hw, pTDInfo->curr_desc, MAC_REG_CUR_TD_ADDR+(4*iQNo));

    BYTE_REG_BITS_ON(&pInfo->hw, CR0_TXON, MAC_REG_CR0);

    if( pInfo->hw.flags & RHINE_FLAGS_TAGGING)
         BYTE_REG_BITS_ON(&pInfo->hw, 1 << (7-iQNo), MAC_REG_TQWK);

    BYTE_REG_BITS_ON(&pInfo->hw, CR1_TDMD1, MAC_REG_CR1);
}

static void rhine_free_tx_buf(int iQNo, int idx) 
{
    struct sk_buff* skb;

    pcieth_debugpr("%s \n", __FUNCTION__);
    PRHINE_TD_INFO pTDInfo=&pInfo->apTDInfos[iQNo][idx];
#ifdef RHINE_ZERO_COPY_SUPPORT
    PTX_DESC pTD=&pInfo->hw.apTDRings[iQNo][idx];
#endif
    skb=pTDInfo->skb;
    pTDInfo->skb=0;
} 

static int rhine_tx_srv(unsigned int status) 
{
    PTX_DESC                    pTD;
    int                         iQNo;
    unsigned char               bFull = FALSE;
    int                         idx;
    int                         works = 0;
    PRHINE_TD_INFO              pTDInfo;
    unsigned int                tdesc0;
    pcieth_debugpr("%s\n", __FUNCTION__);

    for (iQNo = 0; iQNo < pInfo->hw.nTxQueues; iQNo++) 
    {
        for (idx = pInfo->hw.aiTailTDIdx[iQNo];
            pInfo->hw.iTDUsed[iQNo] > 0;
            idx = ((idx+1) % pInfo->hw.sOpts.nTxDescs)) 
        {

            //Get Tx Descriptor
            pTD = &(pInfo->hw.apTDRings[iQNo][idx]);
            pTDInfo = &(pInfo->apTDInfos[iQNo][idx]);

            if(rhine_td_own_bit_on(pTD))
                break;

            if ( (works++ > INT_WORKS_DEF) &&
                 (!(status & (ISR_UDFI|ISR_ABTI))) )
            {
                break;
            }

            tdesc0 = pTD->tdesc0;
            /* clear status first */
            pTD->tdesc0=0;

            // Only the status of first TD in the chain is correct
            if (pTD->tdesc1 & cpu_to_le32(TCR_STP)) 
            {
                if (tdesc0 & cpu_to_le32(TSR1_TERR)) 
                {
                    pcieth_debugpr("%s : td error 0x%4X\n", "VIA Rhine III on SParK", tdesc0);

                    if (tdesc0 & cpu_to_le32(TSR1_UDF))
                    {
                        if (pInfo->hw.sOpts.tx_thresh < 4) 
                        {
                            pInfo->hw.sOpts.tx_thresh++;
                            pcieth_debugpr("%s: transmitter fifo underrun occurred, increase fifo threshold to  %d\n", "VIA Rhine III on SParK", pInfo->hw.sOpts.tx_thresh);

                            rhine_set_tx_thresh(&pInfo->hw, pInfo->hw.sOpts.tx_thresh);
                        }
                        rhine_tx_srv_resend(idx, iQNo);
                        break;
                    }

                    if (tdesc0 & cpu_to_le32(TSR1_ABT)) {
                        pcieth_debugpr("%s: transmitter fifo abort occurred\n", "VIA Rhine III on SParK"); 
                        rhine_tx_srv_drop(idx, iQNo);
                    }
                }
                rhine_free_tx_buf(iQNo, idx);
                pInfo->hw.iTDUsed[iQNo]--;
            }
        }
        pInfo->hw.aiTailTDIdx[iQNo] = idx;

        if (AVAIL_TD(&pInfo->hw, iQNo) < 4) {
            bFull = TRUE;
        }
    }
    return works;
}

unsigned short rhine_get_rx_frame_length( PRX_DESC pRD)
{
    unsigned short length;
    pcieth_debugpr("%s \n", __FUNCTION__);
    length = ((cpu_to_le32(pRD->rdesc0) & FET_RXSTAT_RXLEN) >> 16);
    return length;
}

static inline void rhine_rx_csum(PRX_DESC pRD, struct sk_buff* skb) 
{ 
    unsigned int status;
    skb->ip_summed = CHECKSUM_NONE;
    pcieth_debugpr("%s \n", __FUNCTION__);
    if (pRD->rdesc0 & cpu_to_le32(RSR0_FRAG))
        return;

    status = cpu_to_le32(pRD->rdesc1);
    if (status & PQSTS_IPKT) 
    {
            if (status & PQSTS_IPOK) 
            {
            if ((status & PQSTS_TCPKT)
                ||(status & PQSTS_UDPKT)) 
            {
                if (!(status & PQSTS_TUOK )) 
                {
                    return;
                }
            }
            skb->ip_summed = CHECKSUM_UNNECESSARY;
        }
    }
}

static unsigned char rhine_receive_frame(int idx) 
{
    PRHINE_RD_INFO              pRDInfo = &(pInfo->aRDInfo[idx]);
    PRX_DESC                    pRD = &(pInfo->hw.aRDRing[idx]);
    struct sk_buff*             skb;
    unsigned short              wTag;
    unsigned short              frame_length;
    int i;
    int ret;
    pcieth_debugpr("%s \n", __FUNCTION__);
    pcieth_debugpr("idx = %d %s\n",idx, __FUNCTION__);
    pcieth_debugpr("pRDInfo = 0x%x %s\n", pRDInfo, __FUNCTION__);


    if ((cpu_to_le32(pRD->rdesc0) & (RSR1_STP|RSR1_EDP)) != (RSR1_STP|RSR1_EDP)) 
    {
        rtl_printf(" %s : the received frame span multple RDs\n",
                        "VIA Rhine III on SParK");
        return FALSE;
    }

    frame_length = rhine_get_rx_frame_length(pRD);

    //Drop long packet
    if((frame_length-4) > (ETH_FRAME_LEN)) 
    {
        return FALSE;
    }

    skb = pRDInfo->skb;

    //Get Tag
    wTag = htons(*(unsigned short *)((unsigned char *)skb->data+((frame_length+3) & ~3)+2));

    if (pRD->rdesc1 & cpu_to_le32(PQSTS_TAG)) 
    {
        if (!(pInfo->hw.flags & RHINE_FLAGS_TAGGING)) 
        {
            if ((wTag & 0x0FFF) != 0) 
            {
                return FALSE;
            }
        }
    }
    if (pInfo->hw.flags & RHINE_FLAGS_IP_ALIGN)  
    {
        for (i = frame_length; i >= 0 ; i--)
            *(skb->data + i + 2) = *(skb->data + i);
        skb->data += 2;
    }

    skb->ip_summed = CHECKSUM_NONE;

    if(pInfo->hw.flags & RHINE_FLAGS_RX_CSUM)
        rhine_rx_csum(pRD, skb);

    if(pRD->rdesc0 & cpu_to_le32(RSR0_FRAG))
        skb->ip_summed = CHECKSUM_NONE;

//DEEPTI
    //if(*(((unsigned char *)(pRD->buff_addr)) + 1) == 0x1c)
    if( (*(((unsigned char *)(pRD->buff_addr)) + 1) == 0xFF)) //broadcast
    {
        if(  
              (*(((unsigned char *)(pRD->buff_addr)) + 12) == 0x08) //ARP
              && (*(((unsigned char *)(pRD->buff_addr)) + 13) == 0x06) //ARP
              && (*(((unsigned char *)(pRD->buff_addr)) + 20) == 0x00) //REQUEST
              && (*(((unsigned char *)(pRD->buff_addr)) + 21) == 0x01) //REQUEST
              && (*(((unsigned char *)(pRD->buff_addr)) + 41) == 0xa3)) //TO ME
        {
            /* Then only pass to upper layer*/
            // DEEPTI Circular Buffer 
            ret = insertq(&rxbuff_cq_mon, (unsigned char *)(pRD->buff_addr), frame_length);
            recv_lengths[ret] = frame_length; 
             
            // DEEPTI Circular Buffer ~
            DEEPTI_RX++;
        }
        else
        {
            /* Discard other broadcasts */
        }
    }
    else
    {
         /* pass to upper layer */
         // DEEPTI Circular Buffer 
         ret = insertq(&rxbuff_cq_mon, (unsigned char *)(pRD->buff_addr), frame_length);
         recv_lengths[ret] = frame_length;
         //if(*(((unsigned char *)(pRD->buff_addr)) + 58) == 0x22);
         //{
         //     rtl_printf("\n*********Command Query Rcvd***************\n");
         //}
         // DEEPTI Circular Buffer ~
    }
//END DEEPTI

#if 0
    skb_put(skb, frame_length-4);

    skb->protocol = eth_type_trans(skb, skb->dev);

    //drop frame not met IEEE 802.3
    if (pInfo->hw.flags & RHINE_FLAGS_VAL_PKT_LEN) 
    {
        if ( (skb->protocol == htons(ETH_P_802_2)) &&
             (skb->len != htons(*(unsigned short *)(skb->mac.raw + 12))) )
        {
            skb_put(skb, -(frame_length-4));
            return FALSE;
        }
    }
#endif
    return TRUE;
}


/* RD operation helper function. */
unsigned int rhine_rd_own_bit_on( PRX_DESC pRD )
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    return (pRD->rdesc0 & cpu_to_le32(RSR_OWN));
}

static int rhine_rx_srv(unsigned int status)
{
    PRX_DESC                    pRD;
    PRHINE_RD_INFO              pRDInfo;
    int                         iCurrRDIdx = pInfo->hw.iCurrRDIdx;
    int                         works = 0;
    unsigned short int          frame_length;
    unsigned int temp =0;
    pcieth_debugpr("%s \n", __FUNCTION__);

    while (TRUE)
    {
        pRD = &(pInfo->hw.aRDRing[iCurrRDIdx]);
        pRDInfo = &(pInfo->aRDInfo[iCurrRDIdx]);

         //No more skb buff?
        if (pRDInfo->skb == NULL) 
        {
            rtl_printf("%s %s %d\n","pRDInfo->skb found NULL: ", __LINE__);
            if (!rhine_alloc_rx_buf(iCurrRDIdx))
            {
                rtl_printf("%s %d\n", __FUNCTION__, __LINE__);
                break;
            }
        }

        if (works++ > INT_WORKS_DEF)
        {
            break;
        }
        if(rhine_rd_own_bit_on(pRD))
        {
            break;
        }

        frame_length = rhine_get_rx_frame_length(pRD);

        /*Removed the stats part */

        if (pRD->rdesc0 & cpu_to_le32(RSR1_RXOK)) 
        {    
            if (rhine_receive_frame(iCurrRDIdx)) 
            {
                if (!rhine_alloc_rx_buf(iCurrRDIdx)) 
                {
                    rtl_printf("%s: can not allocate rx buf\n",
                                    "VIA Rhine III on SParK");
                    break;
                }
            }
        }
        /* Removing the error analysis part */

        rhine_set_rd_own(pRD);
        if (pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL)
            CSR_WRITE_1(&pInfo->hw, 1, MAC_REG_FLOWCR0);

        ADD_ONE_WITH_WRAP_AROUND(iCurrRDIdx, pInfo->hw.sOpts.nRxDescs);
    }

    pInfo->hw.iCurrRDIdx = iCurrRDIdx;
    return works;
}

unsigned int rhine_intr(unsigned int irq,struct pt_regs *r)
{
    unsigned int        isr_status;
    unsigned long result1;
    unsigned long result2;
    unsigned long result3;
    unsigned long result4;
    unsigned int        AllIsrStatus = 0;
    int                 max_count = 0;
    int                 handled = 0;
    pcieth_debugpr("%s \n", __FUNCTION__);
    result1 = GET_CR3();
/* This is to load SParK page tables so that we can access the 
   PCI configuration space
 */
    enable_pagebit(); 

    isr_status = rhine_ReadISR(&pInfo->hw);

    if ( ((isr_status & pInfo->hw.IntMask) == 0) ||
         (isr_status == 0x00FFFFFFUL) )
    {
        /* Loading back the GOS pages as the control will now return to
         * where we were before the interrupt occured
         */
        SET_CR3(result1);
        //load_pde(guestOS_thread[iCurrGuestOsIndex].iCurrTaskId);
        return handled;
    }

    handled = 1;
    rhine_disable_int(&pInfo->hw);

    while (isr_status != 0)  
    {
        AllIsrStatus |= isr_status;

        rhine_WriteISR(isr_status, &pInfo->hw);

        if (isr_status & (ISR_SRCI|ISR_TDWBRAI|ISR_BE|ISR_CNT))
            rhine_error(isr_status);

        max_count += rhine_rx_srv(isr_status);
 
        max_count += rhine_tx_srv(isr_status);

        max_count += rhine_rx_srv(isr_status);

        max_count += rhine_tx_srv(isr_status);

        isr_status = rhine_ReadISR(&pInfo->hw);
        if (max_count > pInfo->hw.sOpts.int_works)
        {
            break;
        }
    } // while
    rhine_enable_int(&pInfo->hw, AllIsrStatus);
/* Loading back the GOS pages as the control will now return to
 * where we were before the interrupt occured
 */
    SET_CR3(result1);
    //load_pde(guestOS_thread[iCurrGuestOsIndex].iCurrTaskId);
    return handled;
}

void rhine_disable_mii_auto_poll(struct rhine_hw *hw)
{
    unsigned short int     ww;
    pcieth_debugpr("%s \n", __FUNCTION__);

    /* turn off MAUTO */
    CSR_WRITE_1(hw, 0, MAC_REG_MIICR);

    /* as soon as MIDLE is on, MAUTO is really stoped */
    for (ww = 0; ww < W_MAX_TIMEOUT; ww++)
    {
        usec_delay(5);
        if (BYTE_REG_BITS_IS_ON(hw, MIIAD_MIDLE, MAC_REG_MIIAD))
            break;
    }
}

void rhine_enable_mii_auto_poll(struct rhine_hw *hw) 
{
    int ii;
    pcieth_debugpr("%s \n", __FUNCTION__);
    CSR_WRITE_1(hw, 0, MAC_REG_MIICR);
    CSR_WRITE_1(hw, MIIAD_MSRCEN|0x01, MAC_REG_MIIAD);
    CSR_WRITE_1(hw, MIICR_MAUTO, MAC_REG_MIICR);

    for (ii=0;ii<W_MAX_TIMEOUT; ii++)
        if (BYTE_REG_BITS_IS_ON(hw, MIIAD_MDONE, MAC_REG_MIIAD))
            break;
    BYTE_REG_BITS_ON(hw, MIIAD_MSRCEN,MAC_REG_MIIAD);
}

static unsigned char rhine_init_rings() 
{
    int     i;

    pcieth_debugpr("%s \n", __FUNCTION__);
    /*allocate all RD/TD rings a single pool*/
    
#if DEEPTI 
    pcieth_debugpr("%s %d %d %d %d %d \n", __FUNCTION__,
                                                pInfo->hw.sOpts.nRxDescs,
                                                sizeof(RX_DESC),
                                                pInfo->hw.sOpts.nTxDescs,
                                                sizeof(TX_DESC),
                                                pInfo->hw.nTxQueues);

    pcieth_debugpr("%s %d\n", __FUNCTION__, pInfo->hw.sOpts.nRxDescs * sizeof(RX_DESC) + pInfo->hw.sOpts.nTxDescs * sizeof(TX_DESC)*pInfo->hw.nTxQueues);
#endif 
    if (sizeof(vir_pool) < pInfo->hw.sOpts.nRxDescs * sizeof(RX_DESC) + pInfo->hw.sOpts.nTxDescs * sizeof(TX_DESC)*pInfo->hw.nTxQueues)
    {
         rtl_printf("Statically allocated virtual pool is insufficient\n");
         rtl_printf("Please check the default and present values of \n");
         rtl_printf("nRxDescs nTxDescs nTxQueues\n");
         return FALSE;
    }
    else if (vir_pool==NULL) 
    {
         rtl_printf("Static allocation of virtual pool failed\n");
         return FALSE;
    }

    memset(vir_pool,0, pInfo->hw.sOpts.nRxDescs * sizeof(RX_DESC) +
        pInfo->hw.sOpts.nTxDescs * sizeof(TX_DESC)*pInfo->hw.nTxQueues); 
    pInfo->hw.aRDRing = (PRX_DESC)vir_pool;
    //rtl_printf("%s : pInfo->hw.aRDRing = 0x%x\n", __FUNCTION__, pInfo->hw.aRDRing);

    pInfo->tx_bufs = spark_tx_buffer;

#if DEEPTI 
    pcieth_debugpr("%s 0x%x\n", __FUNCTION__, spark_tx_buffer);
    pcieth_debugpr("%s 0x%x\n", __FUNCTION__, pInfo->tx_bufs);
    pcieth_debugpr("%s %d %d %d \n", __FUNCTION__,
                                     pInfo->hw.sOpts.nTxDescs,
                                     PKT_BUF_SZ,
                                     pInfo->hw.nTxQueues);

    pcieth_debugpr("%s %d\n", __FUNCTION__, pInfo->hw.sOpts.nTxDescs * PKT_BUF_SZ*pInfo->hw.nTxQueues);
#endif

    if (sizeof(spark_tx_buffer) < pInfo->hw.sOpts.nTxDescs * PKT_BUF_SZ*pInfo->hw.nTxQueues)
    {
         rtl_printf("Statically allocated TX buffer is insufficient\n");
         rtl_printf("Please check the default and present values of \n");
         rtl_printf("nTxDescs nTxQueues\n");
         return FALSE;
    }
    else if (pInfo->tx_bufs==NULL)
    {
        rtl_printf("Static allocation of TX buffer failed \n");
        return FALSE;
    }

    memset(pInfo->tx_bufs,0,pInfo->hw.sOpts.nTxDescs * PKT_BUF_SZ*pInfo->hw.nTxQueues);

#if DEEPTI 
    pcieth_debugpr("%s vir_pool = 0x%x\n", __FUNCTION__, vir_pool);
    pcieth_debugpr("%s pInfo->hw.aRDRing  = 0x%x\n", __FUNCTION__, pInfo->hw.aRDRing);
    pcieth_debugpr("%s pInfo->tx_bufs = 0x%x\n", __FUNCTION__, pInfo->tx_bufs);
#endif

    for (i=0;i<pInfo->hw.nTxQueues;i++) 
    {
        pInfo->hw.apTDRings[i] = vir_pool +
                pInfo->hw.sOpts.nRxDescs*sizeof(RX_DESC)+
                pInfo->hw.sOpts.nTxDescs*sizeof(TX_DESC)*i;
#if DEEPTI 
        pcieth_debugpr("%s pInfo->hw.apTDRings[%d] = 0x%x\n", __FUNCTION__, i, pInfo->hw.apTDRings[i]);
#endif
    }

    return TRUE;
}

void rhine_set_rd_own( PRX_DESC pRD)
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    pRD->rdesc0 |= cpu_to_le32(RSR_OWN);
}

void rhine_set_rx_buf_sz(PRX_DESC pRD, unsigned short size)
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    pRD->rdesc1 &= cpu_to_le32(~FET_RXCTL_BUFLEN);
    pRD->rdesc1 |= cpu_to_le32(size);
}

static unsigned char rhine_alloc_rx_buf(int idx)
{
    PRX_DESC        pRD = &(pInfo->hw.aRDRing[idx]);
    PRHINE_RD_INFO pRDInfo = &(pInfo->aRDInfo[idx]);
    pcieth_debugpr("%s \n", __FUNCTION__);

    pRDInfo->skb = (struct sk_buff*)(&(skb_recv_buffer[SKB_BUFF_SIZE * idx]));
    pRDInfo->skb->data = ( ((unsigned char *)(pRDInfo->skb)) + sizeof(struct sk_buff) + 1);
#if DEEPTI
    pcieth_debugpr("pRD = 0x%x %d\n", pRD, idx);
    pcieth_debugpr("pRDInfo = 0x%x %d\n", pRDInfo, idx);
    pcieth_debugpr("skb_address = 0x%x %s %d\n", pRDInfo->skb, __FUNCTION__, idx);
    pcieth_debugpr("pRDInfo->skb->data = 0x%x  %d\n", pRDInfo->skb->data, idx);
#endif
    if (pRDInfo->skb == NULL)
        return FALSE;

    pRD->rdesc0=0;

    rhine_set_rx_buf_sz(pRD, (unsigned short)pInfo->hw.rx_buf_sz);
    rhine_set_rd_own(pRD);
    pRD->buff_addr = cpu_to_le32(&(spark_rx_buffer[pInfo->hw.rx_buf_sz * idx]));

#if DEEPTI
    rtl_printf("pRD->buff_addr = 0x%x %d\n", pRD->buff_addr, idx);
#endif
    return TRUE;
}

static unsigned char rhine_init_rd_ring() 
{
    int i;
    PRX_DESC        pDesc;
    PRHINE_RD_INFO  pRDInfo;
    pcieth_debugpr("%s \n", __FUNCTION__);

    if ((spark_aRDInfo==NULL) || 
        (RD_INFO_SIZE < sizeof(RHINE_RD_INFO)*pInfo->hw.sOpts.nRxDescs))
    {
        rtl_printf("Static allocation of spark_aRDInfo buffer failed \n");
        return FALSE;
    } 
#if DEEPTI
    pcieth_debugpr("skb_recv_buffer = 0x%x", skb_recv_buffer);
#endif

    pInfo->aRDInfo = (PRHINE_RD_INFO)spark_aRDInfo;

    memset(pInfo->aRDInfo, 0, sizeof(RHINE_RD_INFO)*pInfo->hw.sOpts.nRxDescs);
    memset(skb_recv_buffer, 0, SKB_BUFF_SIZE * pInfo->hw.sOpts.nRxDescs);

    /* Init the RD ring entries */
    for (i = 0; i < pInfo->hw.sOpts.nRxDescs; i++)
    {
        pDesc = &(pInfo->hw.aRDRing[i]);
        pRDInfo = &(pInfo->aRDInfo[i]);

        if (!rhine_alloc_rx_buf(i))
        {
            rtl_printf("%s: can not alloc rx bufs\n",__FUNCTION__);
            return FALSE;
        }

        pRDInfo->curr_desc = cpu_to_le32(pDesc);
        pDesc->next_desc = cpu_to_le32(pDesc + 1);
    }

    pInfo->hw.aRDRing[i-1].next_desc =  cpu_to_le32(&(pInfo->hw.aRDRing[0]));
    pInfo->hw.iCurrRDIdx = 0;

// DEEPTI Circular Buffer
    init_cq_monitor(&rxbuff_cq_mon, (RX_DESC_DEF));
// DEEPTI Circular Buffer ~

#if DEEPTI
    pcieth_debugpr("pInfo->aRDInfo = 0x%x\n", pInfo->aRDInfo);
    pcieth_debugpr("%s pInfo->hw.aRDRing  = 0x%x\n", __FUNCTION__, pInfo->hw.aRDRing);
    pcieth_debugpr("%s spark_rx_buffer = 0x%x\n",__FUNCTION__, spark_rx_buffer);
    pcieth_debugpr("%s RX_BUFF_SIZE = 0x%x\n",__FUNCTION__, RX_BUFF_SIZE);
#endif
    return TRUE;
}

static unsigned char rhine_init_td_ring()
{
    int i, j;
    PTX_DESC    pDesc;
    PRHINE_TD_INFO  pTDInfo;
    pcieth_debugpr("%s\n", __FUNCTION__);
    
    if ((spark_aTDInfo==NULL) || 
        (TD_INFO_SIZE < sizeof(RHINE_RD_INFO)*pInfo->hw.sOpts.nRxDescs*pInfo->hw.nTxQueues))
    {
        rtl_printf("Static allocation of spark_aTDInfo buffer failed \n");
        return FALSE;
    }

    /* Init the TD ring entries */
    for (j=0; j<pInfo->hw.nTxQueues; j++)
    {
        pInfo->apTDInfos[j] = &(spark_aTDInfo[(sizeof(RHINE_TD_INFO))*(pInfo->hw.sOpts.nTxDescs)*j]);
        memset(pInfo->apTDInfos[j], 0,
                sizeof(RHINE_TD_INFO)*pInfo->hw.sOpts.nTxDescs);

        for (i = 0; i < pInfo->hw.sOpts.nTxDescs; i++) 
        {
            pDesc = &(pInfo->hw.apTDRings[j][i]);
            pTDInfo = &(pInfo->apTDInfos[j][i]);

            pTDInfo->buf = pInfo->tx_bufs+(i+j)*PKT_BUF_SZ;
            pTDInfo->curr_desc = cpu_to_le32(pDesc);

            pDesc->tdesc1 |= cpu_to_le32(TCR_CHAIN);
            pDesc->next_desc = cpu_to_le32(pDesc + 1);
        }

        pInfo->hw.apTDRings[j][i-1].next_desc = cpu_to_le32(&pInfo->hw.apTDRings[j][0]);
        pInfo->hw.aiTailTDIdx[j] = pInfo->hw.aiCurrTDIdx[j] = pInfo->hw.iTDUsed[j] = 0;
    }
    return TRUE;
}

/*Reads data from configuration space*/
int my_pci_conf1_read(unsigned int seg, unsigned int bus,
                      unsigned int devfn, int reg, int len, unsigned long *value)
{
    unsigned long flags;

    if (!value || (bus > 255) || (devfn > 255) || (reg > 255))
    {
        return -1;
    }

    outl(PCI_CONF1_ADDRESS(bus, devfn, reg), 0xCF8);

    switch (len) {
    case 1:
        *value = inb(0xCFC + (reg & 3));
        break;
    case 2:
        *value = inw(0xCFC + (reg & 2));
        break;
    case 4:
        *value = inl(0xCFC);
        break;
    }
    return 0;
}

int my_pci_conf1_write(unsigned int seg, unsigned int bus,
                       unsigned int devfn, int reg, int len, unsigned long value)
{
    unsigned long flags;

    if ((bus > 255) || (devfn > 255) || (reg > 255))
        return -1;

    outl(PCI_CONF1_ADDRESS(bus, devfn, reg), 0xCF8);
    switch (len) {
    case 1:
        outb((unsigned char)value, 0xCFC + (reg & 3));
        break;
    case 2:
        outw((unsigned short)value, 0xCFC + (reg & 2));
        break;
    case 4:
        outl((unsigned int)value, 0xCFC);
        break;
    }
    return 0;
}

static void rhine_init_pci() 
{
    pcieth_debugpr("%s \n", __FUNCTION__);

    // turn this on to avoid retry forever
    PCI_BYTE_REG_BITS_ON(MODE2_PCEROPT, PCI_REG_MODE2, ethdevinfo[0].bus, ethdevinfo[0].devfn);
    // for some legacy BIOS and OS don't open BusM
    // bit in PCI configuration space. So, turn it on.
    PCI_BYTE_REG_BITS_ON(COMMAND_BUSM, PCI_REG_COMMAND, ethdevinfo[0].bus, ethdevinfo[0].devfn);
    // turn this on to detect MII coding error
    PCI_BYTE_REG_BITS_ON(MODE3_MIION, PCI_REG_MODE3, ethdevinfo[0].bus, ethdevinfo[0].devfn);
    // Turn on MODE10T if it is 3206
    PCI_BYTE_REG_BITS_ON(MODE2_MODE10T, PCI_REG_MODE2, ethdevinfo[0].bus, ethdevinfo[0].devfn);
}

void rhine_set_tx_thresh(struct rhine_hw *hw, int tx_thresh) 
{
    pcieth_debugpr("%s \n", __FUNCTION__);     
    BYTE_REG_BITS_SET(hw, tx_thresh <<3,
        (BCR1_CTSF|BCR1_CTFT1|BCR1_CTFT0), MAC_REG_BCR1);

    BYTE_REG_BITS_SET(hw, tx_thresh <<5,         
        (TCR_RTSF|TCR_RTFT1|TCR_RTFT0), MAC_REG_TCR);
}

void rhine_set_rx_thresh(struct rhine_hw *hw, int rx_thresh) 
{
    pcieth_debugpr("%s \n", __FUNCTION__);     
    BYTE_REG_BITS_SET(hw, rx_thresh <<3,
        (BCR0_CRFT2|BCR0_CRFT1|BCR0_CRFT0), MAC_REG_BCR0);

    BYTE_REG_BITS_SET(hw, rx_thresh <<5,
        (RCR_RRFT2|RCR_RRFT1|RCR_RRFT0), MAC_REG_RCR);
}

void rhine_set_DMA_length(struct rhine_hw *hw, int DMA_length) 
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    BYTE_REG_BITS_SET(hw, DMA_length,
        (BCR0_DMAL2|BCR0_DMAL1|BCR0_DMAL0),MAC_REG_BCR0); 
}

/*
// This function check if MAC operation at Full duplex mode
*/
unsigned char rhine_is_full_duplex (struct rhine_hw *hw)
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    /* if in AUTO-NEGO mode */
    return BYTE_REG_BITS_IS_ON(hw, MIISR_N_FDX, MAC_REG_MIISR);
}

void rhine_set_duplex(struct rhine_hw *hw, unsigned char bFlag) 
{
    pcieth_debugpr("%s \n", __FUNCTION__);

    if (bFlag)
        BYTE_REG_BITS_ON(hw, CR1_FDX, MAC_REG_CR1);
    else
        BYTE_REG_BITS_OFF(hw, CR1_FDX, MAC_REG_CR1);

    if(bFlag)
        CSR_WRITE_1(hw, WOLCFG_SFDX, MAC_REG_WOLCG_SET);
    else
        CSR_WRITE_1(hw, WOLCFG_SFDX, MAC_REG_WOLCG_CLR);
}

static void rhine_set_multi(struct rhine_hw *hw) 
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    
    CSR_WRITE_1(&pInfo->hw, (pInfo->hw.sOpts.rx_thresh<<5), MAC_REG_RCR);
 
    /* SParK does not support promiscuous mode 
     * Removing rhine_set_promiscuous(&pInfo->hw)
     */
    /* SParK does not support all multicast mode 
     * Removing rhine_set_all_multicast(&pInfo->hw)
     */
    /* SParK does not support CAM mode
     * Removing rhine_set_cam(); rhine_set_cam_mask();
     */
    /* DEEPTI : No mutlicast as of now
     * DEEPTI : The interface will not recv any mcast packet
     */
//DEEPTI KEPT OFF for stack
     BYTE_REG_BITS_ON(hw, (RCR_AB), MAC_REG_RCR);
 
}

void rhine_init_flow_control_register(struct rhine_hw *hw, unsigned short RxDescs)
{
    pcieth_debugpr("%s \n",, __FUNCTION__);

    /* Set {XHITH1, XHITH0, XLTH1, XLTH0} in FlowCR1 to {1, 0, 1, 1}
    // depend on RD=64, and Turn on XNOEN in FlowCR1 */    
    BYTE_REG_BITS_SET( hw,
        (FLOWCR1_XONEN|FLOWCR1_XHITH1|FLOWCR1_XLTH1|FLOWCR1_XLTH0),
        0xFF,MAC_REG_FLOWCR1);
     /* Set TxPauseTimer to 0xFFFF */
    CSR_WRITE_2(hw, 0xFFFF, MAC_REG_PAUSE_TIMER);
     /* Initialize RBRDU to Rx buffer count */
    CSR_WRITE_1(hw, RxDescs, MAC_REG_FLOWCR0);
}

unsigned char rhine_mii_read(struct rhine_hw *hw, unsigned char byIdx, unsigned short int * pdata)
{
    unsigned short int         ww;
    pcieth_debugpr("%s \n", __FUNCTION__);

    /* disable MIICR_MAUTO, so that mii addr can be set normally */
    rhine_disable_mii_auto_poll(hw);

    CSR_WRITE_1(hw, byIdx, MAC_REG_MIIAD);

    BYTE_REG_BITS_ON(hw, MIICR_RCMD, MAC_REG_MIICR);

    for (ww = 0; ww < W_MAX_TIMEOUT; ww++) 
    {
        if(!BYTE_REG_BITS_IS_ON(hw, MIICR_RCMD, MAC_REG_MIICR))
            break;
    }

    *pdata=CSR_READ_2(hw, MAC_REG_MIIDATA);

    rhine_enable_mii_auto_poll(hw);

    if (ww == W_MAX_TIMEOUT) 
    {
        return FALSE;
    }
    return TRUE;
}

unsigned char rhine_mii_write (struct rhine_hw *hw, unsigned char byMiiAddr, unsigned short wData)
{
    unsigned short        ww;
    pcieth_debugpr("%s \n", __FUNCTION__);

    /* disable MIICR_MAUTO, so that mii addr can be set normally */
    rhine_disable_mii_auto_poll(hw);

    /* MII reg offset */
    CSR_WRITE_1(hw, byMiiAddr, MAC_REG_MIIAD);
    /* set MII data */
    CSR_WRITE_2(hw, wData, MAC_REG_MIIDATA);

    /* turn on MIICR_WCMD */
    BYTE_REG_BITS_ON(hw, MIICR_WCMD, MAC_REG_MIICR);

    /* W_MAX_TIMEOUT is the timeout period */
    for (ww = 0; ww < W_MAX_TIMEOUT; ww++) 
    {
        usec_delay(5);
        if(!BYTE_REG_BITS_IS_ON(hw, MIICR_WCMD, MAC_REG_MIICR))
            break;
    }

    rhine_enable_mii_auto_poll(hw);

    if (ww == W_MAX_TIMEOUT) 
    {
        return FALSE;
    }
    return TRUE;
}


/*
// Get the media mode stored in EEPROM or module options
*/
void mii_set_auto_on(struct rhine_hw *hw)
{
    pcieth_debugpr("%s \n", __FUNCTION__);

    if ((hw->byRevId >= REV_ID_VT6105_A0) || IS_PHY_VT6103(hw))
        MII_REG_BITS_OFF(1, MII_REG_MODCFG, hw);

    MII_REG_BITS_ON((BMCR_AUTO | BMCR_REAUTO), MII_REG_BMCR, hw);
}

int rhine_set_media_mode(struct rhine_hw *hw, POPTIONS option) 
{
    unsigned char          byFlowCR;
    unsigned short         wOrigANAR; 
    unsigned short         wANAR=0;
    unsigned short         wANARMask;
    unsigned int           status=RHINE_LINK_UNCHANGE;
    pcieth_debugpr("%s\n", __FUNCTION__); 
    /* get original ANAR */     
    rhine_mii_read(hw, MII_REG_ANAR, &wOrigANAR);

    wANARMask = wOrigANAR;
    wOrigANAR &= (ANAR_10|ANAR_10FD|ANAR_TX|ANAR_TXFD|ANAR_PAUSE|ANAR_ASMDIR);
    wANARMask &= ~(ANAR_10|ANAR_10FD|ANAR_TX|ANAR_TXFD|ANAR_PAUSE|ANAR_ASMDIR); 

    /* read original flow control setting from MAC */
    byFlowCR = CSR_READ_1(hw, MAC_REG_FLOWCR1); 

    /* Disable PAUSE in ANAR, disable TX/RX flow control in MAC */
    if (option->flow_cntl == 2) 
    {
        byFlowCR &= ~FLOWCR1_FDXTFCEN;
        byFlowCR &= ~FLOWCR1_FDXRFCEN;
    }
    else if (option->flow_cntl == 3) 
    {
        /* Enable PAUSE in ANAR, enable TX/RX flow control in MAC */
        wANAR |= ANAR_PAUSE;

        byFlowCR |= FLOWCR1_FDXTFCEN;
        byFlowCR |= FLOWCR1_FDXRFCEN;
    }
    byFlowCR &= ~FLOWCR1_HDXFCEN;
    CSR_WRITE_1(hw, byFlowCR, MAC_REG_FLOWCR1);

    /* if connection type is AUTO */
    if (option->spd_dpx == SPD_DPX_AUTO) 
    {
        wANAR |= (ANAR_TXFD|ANAR_TX|ANAR_10FD|ANAR_10);

        if(wANAR != wOrigANAR)
        {
            wANAR |= wANARMask;
            rhine_mii_write(hw, MII_REG_ANAR, wANAR);
            /* enable AUTO-NEGO mode */
            mii_set_auto_on(hw);
            status = RHINE_LINK_CHANGE;
        }

        /* set duplex mode of MAC according to duplex mode of MII */
        if (rhine_is_full_duplex(hw)) 
        {
            rhine_set_duplex(hw, TRUE);
        }
        else 
        {
            rhine_set_duplex(hw, FALSE);
        }
    }
    return status;
}

/*
   Only check current connection type and does not modify any internal register.*/ 
unsigned int rhine_check_media_mode(struct rhine_hw *hw)
{
    unsigned int             status=0;
    pcieth_debugpr("%s \n", __FUNCTION__);

    if (BYTE_REG_BITS_IS_ON(hw, MIISR_LNKFL, MAC_REG_MIISR)) 
    {
        status|=RHINE_LINK_FAIL;
    }
    else
    {
        if(hw->sOpts.spd_dpx == SPD_DPX_AUTO)
        {
            if (BYTE_REG_BITS_IS_ON(hw, MIISR_SPEED, MAC_REG_MIISR))
                status|=RHINE_SPEED_10;
            else
                status|=RHINE_SPEED_100;
             /* check duplex mode*/
            /* if VT6105, check N_FDX bit in MII Status Register directly */
            if (rhine_is_full_duplex(hw)) {
                status|=RHINE_DUPLEX_FULL;
                rhine_set_duplex(hw, TRUE);
            }
            else {
                rhine_set_duplex(hw, FALSE);
            }
            status |= RHINE_AUTONEG_ENABLE;
        }
        else
        {
            switch(hw->sOpts.spd_dpx)
            {
                case SPD_DPX_100_HALF:
                    status |= RHINE_SPEED_100;
                    break;
                case SPD_DPX_100_FULL:
                    status |= (RHINE_SPEED_100|RHINE_DUPLEX_FULL);
                    break;
                case SPD_DPX_10_FULL:
                    status |= RHINE_DUPLEX_FULL;
                    break;
                case SPD_DPX_10_HALF:
                default:
                    break;
            }
        }
    }
    return status;
}

void rhine_wol_reset(struct rhine_hw *hw) 
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    /* clear sticky bits */
    BYTE_REG_BITS_OFF(hw, (STICKHW_DS1_SHADOW|STICKHW_DS0_SHADOW),
        MAC_REG_STICKHW);
    /* disable force PME-enable */
    CSR_WRITE_1(hw, WOLCFG_PME_OVR, MAC_REG_WOLCG_CLR);
    /* disable power-event config bit */
    CSR_WRITE_1(hw, 0xFF, MAC_REG_WOLCR_CLR);
    CSR_WRITE_1(hw, 0x03, MAC_REG_TSTREG_CLR);
    /* clear power status */
    CSR_WRITE_1(hw, 0xFF, MAC_REG_PWRCSR_CLR);
    CSR_WRITE_1(hw, 0x03, MAC_REG_PWRCSR1_CLR);
}

void rhine_print_link_status(unsigned int status) 
{
    pcieth_debugpr("%s\n", __FUNCTION__);

    if (status & RHINE_LINK_FAIL) 
    {
        rtl_printf("failed to detect cable link\n");
    }
    else 
    {
        if (status & RHINE_AUTONEG_ENABLE)
            pcieth_debugpr("Link auto-negotiation");
        else
            pcieth_debugpr("Link forced");

        if (status & RHINE_SPEED_100)
            pcieth_debugpr(" speed 100M bps");
        else
            pcieth_debugpr(" speed 10M bps");

        if (status & RHINE_DUPLEX_FULL)
            pcieth_debugpr(" full duplex\n");
        else
            pcieth_debugpr(" half duplex\n");
    }
}

void rhine_enable_flow_control_ability(struct rhine_hw *hw) 
{
    unsigned short         wANAR,wANLPAR;
    unsigned char          byFlowCR;
    unsigned int           status;
    pcieth_debugpr("%s \n", __FUNCTION__);

    status = rhine_check_media_mode(hw);

    rhine_mii_read(hw,MII_REG_ANAR,&wANAR);
    rhine_mii_read(hw,MII_REG_ANLPAR,&wANLPAR);

    byFlowCR=CSR_READ_1(hw, MAC_REG_FLOWCR1);

    if (!(status & RHINE_DUPLEX_FULL)) 
    {
        /* Half duplex mode */
        byFlowCR&=~FLOWCR1_FDXTFCEN;
    }
    else 
    {
        /*Full duplxe mode */
        if (((wANAR & (ANAR_ASMDIR|ANAR_PAUSE))==ANAR_ASMDIR)
            && ((wANLPAR & (ANLPAR_ASMDIR|ANLPAR_PAUSE))
            ==(ANLPAR_ASMDIR|ANLPAR_PAUSE))) 
        {
                byFlowCR|=FLOWCR1_FDXTFCEN;
                byFlowCR&=~FLOWCR1_FDXRFCEN;
        }
        else if ((wANAR & ANAR_PAUSE) && (wANLPAR & ANLPAR_PAUSE)) 
        {
                byFlowCR|=(FLOWCR1_FDXTFCEN|FLOWCR1_FDXRFCEN);
        }
        else if (((wANAR & (ANAR_ASMDIR|ANAR_PAUSE))==(ANAR_ASMDIR|ANAR_PAUSE))
            && ((wANLPAR & (ANLPAR_ASMDIR|ANLPAR_PAUSE)) ==ANLPAR_ASMDIR)) 
        {
                byFlowCR&=~FLOWCR1_FDXTFCEN;
                byFlowCR|=FLOWCR1_FDXRFCEN;
        }
        else 
        {
                byFlowCR&=~(FLOWCR1_FDXTFCEN|FLOWCR1_FDXRFCEN);
        }
    }
    byFlowCR &=~FLOWCR1_HDXFCEN;
    CSR_WRITE_1(hw, byFlowCR, MAC_REG_FLOWCR1);
}

void rhine_set_td_own(PTX_DESC pTD)
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    pTD->tdesc0 |= cpu_to_le32(TSR_OWN);
}

void rhine_set_tx_buf_sz(PTX_DESC pTD, unsigned short size)
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    pTD->tdesc1 &= cpu_to_le32(~FET_TXCTL_BUFLEN);
    pTD->tdesc1 |= cpu_to_le32(size);
}

long rhine_xmit(unsigned char * skb, unsigned int size) 
{
    int i;
    int             iQNo = 0;
    PTX_DESC        pTD, pHeadTD;
    PRHINE_TD_INFO  pTDInfo, pHeadTDInfo;
    unsigned long   flags;
    int             iCurrTDIdx, iHeadTDIdx;
    unsigned char * addr_holder;
     
    pcieth_debugpr("\n%s \n", __FUNCTION__);

    DEEPTI_TX++;
    //rtl_printf("\nTransmitting packet #:= %d\n", DEEPTI_TX);
    //for(i=0; i<size; i++)
    //     rtl_printf("0x%x ", *(skb + i));

    iCurrTDIdx = iHeadTDIdx = pInfo->hw.aiCurrTDIdx[iQNo];

    pTD = pHeadTD = &(pInfo->hw.apTDRings[iQNo][iCurrTDIdx]);
    pTDInfo = pHeadTDInfo = &(pInfo->apTDInfos[iQNo][iCurrTDIdx]);

    pHeadTD->tdesc1 |= cpu_to_le32(TCR_IC|TCR_STP|TCR_EDP);
    if (((pInfo->hw.flags & RHINE_FLAGS_TX_ALIGN) && ((long)skb & 3))
            || (size<ETH_ZLEN)) 
    {
        addr_holder = pHeadTDInfo->buf;
        memcpy(pHeadTDInfo->buf, skb, size);
        pHeadTDInfo->buf = addr_holder;
        pHeadTDInfo->skb = skb;
        pHeadTD->buff_addr = skb;

        rhine_set_tx_buf_sz(pHeadTD, (size >= ETH_ZLEN ? size : ETH_ZLEN));

        // padding zero if packet size is less than 60 bytes
        if (size < ETH_ZLEN)
        {
            memset(pHeadTDInfo->buf+size, 0, ETH_ZLEN-size);
        }
    }
    else 
    {
        pHeadTDInfo->skb = skb;
        pHeadTD->buff_addr = skb;
         
        rhine_set_tx_buf_sz(pHeadTD, size);
    }
    if (pInfo->hw.flags & RHINE_FLAGS_TAGGING) {
        pHeadTD->tdesc0 &= cpu_to_le32(~FET_TXSTAT_PQMASK);
        pHeadTD->tdesc0 |= cpu_to_le32((pInfo->hw.sOpts.vid & 0xfff) << 16);
        pHeadTD->tdesc1 |= cpu_to_le32(TCR_TAG);
    }
    
    rhine_set_td_own(pHeadTD);

    pInfo->hw.iTDUsed[iQNo]++;

    if (AVAIL_TD(&pInfo->hw,iQNo)<=1) 
    {
         rtl_printf("SparK : Transmission not able to progress\n");
    }
    pInfo->hw.aiCurrTDIdx[iQNo]=
            (iCurrTDIdx+1) % pInfo->hw.sOpts.nTxDescs;

    if (pInfo->hw.flags & RHINE_FLAGS_TAGGING)
    {
        BYTE_REG_BITS_ON(&pInfo->hw, 1 << (7-iQNo), MAC_REG_TQWK);
    }

    BYTE_REG_BITS_ON(&pInfo->hw, CR1_TDMD1,MAC_REG_CR1);

    rtl_printf("Xmit Complete\n");
//DEEPTI If this statement is removed, transmission becomes too lossy
    return 0;
}

/* Initialiation of adapter */
static void rhine_init_adapter(RHINE_INIT_TYPE InitType) 
{
    int           i;
    unsigned int  status;
    unsigned int  isr_status;

    pcieth_debugpr("%s \n", __FUNCTION__);

    rhine_wol_reset(&pInfo->hw);
    switch (InitType) {
    case RHINE_INIT_RESET:
         rtl_printf("%s %s %d\n","Execution should not reach here\n", __FUNCTION__, __LINE__);
    case RHINE_INIT_WOL:
         rtl_printf("%s %s %d\n","Execution should not reach here\n", __FUNCTION__, __LINE__);
        return;
        break;
    case RHINE_INIT_COLD:
    default:
        pcieth_debugpr("%s %d\n", __FUNCTION__, __LINE__);
        // NO MAC address overrides. Removed code

        BYTE_REG_BITS_OFF(&pInfo->hw, CFGA_LED0S0, MAC_REG_CFGA);

        rhine_set_tx_thresh(&pInfo->hw, pInfo->hw.sOpts.tx_thresh);
        rhine_set_rx_thresh(&pInfo->hw, pInfo->hw.sOpts.rx_thresh);
        rhine_set_DMA_length(&pInfo->hw, pInfo->hw.sOpts.DMA_length);
        // enable queue packet
        BYTE_REG_BITS_OFF(&pInfo->hw, CFGB_QPKTDIS, MAC_REG_CFGB);
        // suspend-well accept broadcast, multicast
        CSR_WRITE_1(&pInfo->hw, WOLCFG_SAM|WOLCFG_SAB, MAC_REG_WOLCG_SET);

        // back off algorithm use original IEEE standard
        BYTE_REG_BITS_ON(&pInfo->hw, TCR_OFSET,MAC_REG_TCR);
        BYTE_REG_BITS_OFF(&pInfo->hw, (CFGD_CRADOM | CFGD_CAP | CFGD_MBA | CFGD_BAKOPT), MAC_REG_CFGD);
        // set packet filter
        // receive directed and broadcast address
        rhine_set_multi(&pInfo->hw);

        pInfo->hw.IntMask=
            (IMR_PRXM   | IMR_PTXM  | IMR_RXEM  | IMR_TXEM  | IMR_TUM   |
            IMR_RUM     | IMR_BEM   | IMR_CNTM      | IMR_ERM   | IMR_ETM   |
            IMR_ABTM    | IMR_SRCM  | IMR_NORBFM | IMR_OVFM);

        pInfo->hw.IntMask |= (IMR_GENM|IMR_TDWBRAI);
        
        //DEEPTI : if not working check here
        CSR_WRITE_4(&pInfo->hw, pInfo->hw.aRDRing, MAC_REG_CUR_RD_ADDR);

        for (i=0; i<pInfo->hw.nTxQueues; i++)
            CSR_WRITE_4(&pInfo->hw, pInfo->hw.apTDRings[i], MAC_REG_CUR_TD_ADDR+(4*i));

        if (pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL)
            rhine_init_flow_control_register(&pInfo->hw, pInfo->hw.sOpts.nRxDescs);

        CSR_WRITE_2(&pInfo->hw, (CR0_DPOLL|CR0_TXON|CR0_RXON|CR0_STRT), MAC_REG_CR0);

        if (rhine_set_media_mode(&pInfo->hw, &pInfo->hw.sOpts) != RHINE_LINK_CHANGE) 
        {
            status = rhine_check_media_mode(&pInfo->hw);
            pcieth_debugpr("%s: ","VIA Rhine III on SParK");
            rhine_print_link_status(status);
            if (!(status & RHINE_LINK_FAIL))
               pcieth_debugpr("Failed to Detect Link %d", __LINE__);
        }
        else 
        {
            status = rhine_check_media_mode(&pInfo->hw);
            if(status & RHINE_LINK_FAIL)
            {
                pcieth_debugpr("%s: ","VIA Rhine III on SParK");
                rhine_print_link_status(status);
            }
        }
        if ((pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL) && (pInfo->hw.sOpts.flow_cntl == 1))
            rhine_enable_flow_control_ability(&pInfo->hw);

        rhine_clearISR(&pInfo->hw);
    }
}

int rhine_initdevice() 
{
    int i;
    pcieth_debugpr("%s \n", __FUNCTION__);

    pInfo->hw.rx_buf_sz = PKT_BUF_SZ ;

    if (!rhine_init_rings(pInfo)) 
    {
        return -ENOMEM;
    }
    if (!rhine_init_rd_ring()) 
    {
        return -ENOMEM;
    }
    if (!rhine_init_td_ring()) 
    {
        return -ENOMEM;
    }
    rhine_init_pci();
    rhine_init_adapter(RHINE_INIT_COLD);
#if DEEPTI
    pcieth_debugpr("MAC_REG_CUR_RD_ADDR = 0x%x\n", CSR_READ_4(&pInfo->hw, MAC_REG_CUR_RD_ADDR));
#endif

// DEEPTI Reconsider allow_interrupts when making these hypercalls
    /* request irq */

#if ETH_INTR_BASED
    rtl_stop_interrupts ();

    rtl_request_global_irq(((ethdevinfo[0]).intr_line), &rhine_intr);
    enable_8259_irq(((ethdevinfo[0]).intr_line));
    if((ethdevinfo[0]).intr_line == 9)
    {
       enable_8259_irq(2);
    }
    rtl_printf("\n\tOperating the VIA Rhine III card in INTERRUPT mode\n");
#else
    rtl_printf("\n\tOperating the VIA Rhine III card in Polling mode\n");
#endif
#if 0
Removed because interrupts should be allowed when SParK is done with
all its other configuration 
    rtl_allow_interrupts ();
#endif

    rhine_enable_int(&pInfo->hw, 0x00000000UL);

    pInfo->hw.flags |=RHINE_FLAGS_OPENED;
    ethdevinfo[0].device_status = ETH_DEVICE_READY;

    return 0;
}

static void rhine_set_int_opt(int *opt, int val, int min, int max, int def, char* name) 
{
    pcieth_debugpr("%s\n", __FUNCTION__);

    if (val==-1)
        *opt=def;
    else if (val<min || val>max) 
    {
         pcieth_debugpr("The value of parameter %s is invalid, the valid range is (%d-%d)\n" , name, min, max);
        *opt=def;
    } 
    else 
    {
        pcieth_debugpr("Set value of parameter %s to %d\n", name, val);
        *opt=val;
    }
}

static void rhine_set_bool_opt(unsigned int * opt, int val, unsigned char def, unsigned int  flag, char* name) 
{
    pcieth_debugpr("%s \n", __FUNCTION__);

    (*opt)&=(~flag);

    if (val==-1)
        *opt|=(def ? flag : 0);
    else if (val<0 || val>1) 
    {
        pcieth_debugpr("The value of parameter %s is invalid, the valid range is (0-1)\n", name);
        *opt|=(def ? flag : 0);
    } 
    else 
    {
        pcieth_debugpr("Set parameter %s to %s\n", name , val ? "TRUE" : "FALSE");
        *opt|=(val ? flag : 0);
    }
}

void rhine_enable_mmio(struct rhine_hw *hw) 
{
    int n;
    pcieth_debugpr("%s \n", __FUNCTION__);
    n = _INB(hw, MAC_REG_CFGD)  & ~CFGD_MBA | CFGD_GPIOEN ;
    _OUTB(hw, n, MAC_REG_CFGD);
}

void rhine_reload_eeprom(struct rhine_hw *hw) 
{
    int i;
    pcieth_debugpr("%s\n", __FUNCTION__);

    i = _INB(hw, MAC_REG_EECSR) |EECSR_AUTOLD;
    _OUTB(hw, i, MAC_REG_EECSR);

    /* Typically 2 cycles to reload. */
    for (i = 0; i < 150; i++)
            if(!(_INB(hw, MAC_REG_EECSR) & EECSR_AUTOLD))
                break;

    rhine_enable_mmio(hw);
}

static void rhine_get_options(POPTIONS pOpts) 
{
    pcieth_debugpr("%s\n", __FUNCTION__);

    rhine_set_int_opt(&pOpts->tx_thresh, TX_THRESH_DEF,
        TX_THRESH_MIN, TX_THRESH_MAX, TX_THRESH_DEF,"tx_thresh");

    rhine_set_int_opt(&pOpts->rx_thresh, RX_THRESH_DEF,
        RX_THRESH_MIN, RX_THRESH_MAX, RX_THRESH_DEF,"rx_thresh");

    rhine_set_int_opt(&pOpts->DMA_length, DMA_LENGTH_DEF,
        DMA_LENGTH_MIN, DMA_LENGTH_MAX, DMA_LENGTH_DEF,"DMA_length");

    rhine_set_int_opt(&pOpts->nRxDescs, RX_DESC_DEF,
        RX_DESC_MIN, RX_DESC_MAX, RX_DESC_DEF, "RxDescriptors");

    rhine_set_int_opt(&pOpts->nTxDescs, TX_DESC_DEF,
        TX_DESC_MIN, TX_DESC_MAX, TX_DESC_DEF, "TxDescriptors");

    rhine_set_int_opt(&pOpts->vid, VLAN_ID_DEF,
        VLAN_ID_MIN, VLAN_ID_MAX, VLAN_ID_DEF,"VID_setting");

    rhine_set_bool_opt(&pOpts->flags, TAGGING_DEF,
        TAGGING_DEF,RHINE_FLAGS_TAGGING, "enable_tagging");

    rhine_set_bool_opt(&pOpts->flags, RX_CSUM_DEF,
        RX_CSUM_DEF,RHINE_FLAGS_RX_CSUM,"rxcsum_offload");

    rhine_set_bool_opt(&pOpts->flags, TX_CSUM_DEF,
        TX_CSUM_DEF,RHINE_FLAGS_TX_CSUM,"txcsum_offload");

    rhine_set_int_opt(&pOpts->flow_cntl, FLOW_CNTL_DEF,
        FLOW_CNTL_MIN,FLOW_CNTL_MAX, FLOW_CNTL_DEF, "flow_control");

    rhine_set_bool_opt(&pOpts->flags, IP_ALIG_DEF,
        IP_ALIG_DEF, RHINE_FLAGS_IP_ALIGN, "IP_byte_align");

    rhine_set_bool_opt(&pOpts->flags, VAL_PKT_LEN_DEF,
        VAL_PKT_LEN_DEF, RHINE_FLAGS_VAL_PKT_LEN, "ValPktLen");

    rhine_set_int_opt((int*) &pOpts->spd_dpx, MED_LNK_DEF,
        MED_LNK_MIN, MED_LNK_MAX, MED_LNK_DEF,"Media link mode");

    rhine_set_int_opt((int*) &pOpts->wol_opts, WOL_OPT_DEF,
        WOL_OPT_MIN, WOL_OPT_MAX, WOL_OPT_DEF,"Wake On Lan options");

    rhine_set_int_opt((int*) &pOpts->int_works, INT_WORKS_DEF,
        INT_WORKS_MIN, INT_WORKS_MAX, INT_WORKS_DEF,"Interrupt service works");

}

static void rhine_print_info()
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    pcieth_debugpr("MAC=%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
        pci82573v_dev.dev_addr[0],pci82573v_dev.dev_addr[1],pci82573v_dev.dev_addr[2],
        pci82573v_dev.dev_addr[3],pci82573v_dev.dev_addr[4],pci82573v_dev.dev_addr[5]);
    pcieth_debugpr("IO=0x%lx Mem=0x%lx\n", pInfo->hw.ioaddr, pInfo->hw.memaddr);
}

int pci82573v_attach()
{
    int ret,subdev,i,j;
    unsigned int boardid;
    unsigned int io_addr[6];
    unsigned int iobase;

    pcieth_debugpr("%s\n", __FUNCTION__);

    ret = get_device_info(ETH_PCI_VIA_VENDOR_ID, ETH_PCI_VIA_DEV_ID,
                            1, &ethdevinfo[0]);
    //rtl_printf("\nIntr line = %d\n", (ethdevinfo[0]).intr_line);
    //rtl_printf("Intr pin = %d\n", (ethdevinfo[0]).intr_pin);

    if (ret == 0)
        return -1;

#if DEEPTI
    pcieth_debugpr("%s: Driver: pci info of VIA ETH device 0:\n", __FUNCTION__);
    pcieth_debugpr("%s: Num of mem regions %d\n", __FUNCTION__,ethdevinfo[0].num_mem);
    pcieth_debugpr("%s: Num of io regions %d\n", __FUNCTION__,ethdevinfo[0].num_io);
#endif

    for(j=0; j < ethdevinfo[0].num_mem; j++)
    {
        pcieth_debugpr("%s: mem region at 0x%x of size 0x%x\n", __FUNCTION__,ethdevinfo[0].mem_addr[j],ethdevinfo[0].mem_size[j]);
        pci82573v_dev.membase= (unsigned char *)ethdevinfo[0].mem_addr[0] ;
    }
    for(j=0; j < ethdevinfo[0].num_io ; j++)
    {
        pcieth_debugpr("%s: io region at 0x%x of size 0x%x\n", __FUNCTION__,ethdevinfo[0].io_addr[j],ethdevinfo[0].io_size[j]);
        pci82573v_dev.iobase= ethdevinfo[0].io_addr[0] - 1;
    }
#if DEEPTI
    pcieth_debugpr("%s: Inter line 0x%x\n", __FUNCTION__,ethdevinfo[0].intr_line);
    pcieth_debugpr("%s: Inter pin 0x%x\n", __FUNCTION__,ethdevinfo[0].intr_pin);
#endif
    return 0;
}

int pci82573v_detach()
{
    pcieth_debugpr("%s\n", __FUNCTION__);
    return 0;
}

int init_i386_pci82573v(void)
{
    int rc;
    int minor;
    int use_count;

    pcieth_debugpr("%s\n", __FUNCTION__);
    if (rtl_register_chrdev (I386_PCIVIA_MAJOR, 
                             I386_PCIVIA_DEVICE_NAME,
                             &rtl_pci82573v_fops))
    {
        pcieth_debugpr("%s : i386_pci82573v: unable to get major %d\n",
                        __FUNCTION__, I386_PCIVIA_MAJOR);
        return -EIO;
    }
    memset(&pci82573v_dev,0,sizeof(pci82573v_device));

    /* Invoke the device attach() function here */
    rc = pci82573v_attach();
    if (rc<0)
    {
        pci82573v_detach();
        return rc;
    }
    pci82573v_dev.attached=1;
    ethdevinfo[0].device_status = ETH_DEVICE_FOUND;
    pcieth_debugpr("Exiting %s\n", __FUNCTION__);
    return 0;
}

void rhine_get_phy_id(struct rhine_hw *hw)
{
    pcieth_debugpr("%s \n", __FUNCTION__);
    rhine_mii_read(hw, MII_REG_PHYID2, (short int *) &hw->dwPHYId);
    rhine_mii_read(hw, MII_REG_PHYID1, ((short int *) &(hw->dwPHYId) + 1));
}

void rhine_get_mac_addr(struct rhine_hw *hw)
{
    int i;
    pcieth_debugpr("%s \n", __FUNCTION__);
    for (i=0; i<6; i++)
        pci82573v_dev.dev_addr[i] = CSR_READ_1(hw, MAC_REG_PAR+i); 
}

unsigned char rhine_soft_reset(struct rhine_hw *hw) 
{
    int i=0;
    pcieth_debugpr("%s\n", __FUNCTION__);

    BYTE_REG_BITS_ON(hw, CR1_SFRST, MAC_REG_CR1);

    for (i=0; i<W_MAX_TIMEOUT; i++) 
    {
        usec_delay(5);
        if ( !BYTE_REG_BITS_IS_ON(hw, CR1_SFRST, MAC_REG_CR1))
            break;
    }

    if (i == W_MAX_TIMEOUT) 
    {
        BYTE_REG_BITS_ON(hw, MISC_CR1_FORSRST, MAC_REG_MISC_CR1);
        usec_delay(2000);
    }
    return TRUE;
}

long rhine_reset_configure()
{
    int                 i;
    char *              hw_addr;
    long                ioaddr, memaddr;
    unsigned int flags = 0 ;

    pcieth_debugpr("%s\n", __FUNCTION__);

    ioaddr  = pci82573v_dev.iobase;
    memaddr = pci82573v_dev.membase;
    pInfo = &deviceInfo;
    pcieth_debugpr("%s Ver. %s\n",RHINE_FULL_DRV_NAM, RHINE_VERSION);
    pcieth_debugpr("Copyright (c) 2002 VIA Technologies, Inc.\n");

    // set up chip io size
    pInfo->hw.io_size = 256;

    pInfo->hw.ioaddr = ioaddr;
    pInfo->hw.memaddr = memaddr;
    pInfo->hw.hw_addr = memaddr;

    pInfo->hw.multicast_limit = 32;

    rhine_enable_mmio(&pInfo->hw);

    rhine_wol_reset(&pInfo->hw);

    rhine_get_phy_id(&pInfo->hw);
    
    rhine_get_mac_addr(&pInfo->hw);

    // software reset
    rhine_soft_reset(&pInfo->hw);
    usec_delay(5000);

    // EEPROM reload
    rhine_reload_eeprom(&pInfo->hw);

    rhine_get_options(&pInfo->hw.sOpts);

    // set up TX queue number
    pInfo->hw.nTxQueues = NO_OF_TX_QUEUES;

    // set up chip flags
    flags |= (RHINE_FLAGS_IP_ALIGN|RHINE_FLAGS_VAL_PKT_LEN);

    flags |= RHINE_FLAGS_FLOW_CTRL;

    // Mask out the options cannot be set to the chip
    pInfo->hw.sOpts.flags &= flags;

    //Enable the chip specified capbilities that is not in the option list
    pInfo->hw.flags = pInfo->hw.sOpts.flags | (flags & 0xFF000000UL);
    pInfo->wol_opts = pInfo->hw.sOpts.wol_opts;
    pInfo->hw.flags |= RHINE_FLAGS_WOL_ENABLED;

#if DEEPTI
    pcieth_debugpr("pInfo->hw.sOpts.flags = 0x%x\n", pInfo->hw.sOpts.flags);
    pcieth_debugpr("pInfo->wol_opts = 0x%x\n", pInfo->wol_opts);
    pcieth_debugpr("pInfo->hw.flags = 0x%x\n", pInfo->hw.flags);
#endif

    rhine_print_info(pInfo);

    pcieth_debugpr("%s %d\n", __FUNCTION__, __LINE__);
    ethdevinfo[0].device_status = ETH_DEVICE_RESET;
    return 0;
}//rhine_found1()

long rhine_get_ip(unsigned char * ifa_addr) 
{
    int i;
    pcieth_debugpr("%s \nIPAddress : ", __FUNCTION__);
    if(ifa_addr != NULL)
    {
    	memcpy(pInfo->abyIPAddr, ifa_addr, 4);
        for(i=0; i<3; i++)
           rtl_printf("%d:", pInfo->abyIPAddr[i]);
        rtl_printf("%d\n", pInfo->abyIPAddr[i]);
    	return TRUE;
    }
    else
    	return FALSE;
}

long rhine_isready()
{
    if(ethdevinfo[0].device_status == ETH_DEVICE_READY)
    {
        //rtl_printf("rhine is ready\n");
        return 1;
    }
    else
    {
        rtl_printf("rhine is not ready\n");
        return 0;
    }
}

long rhine_receive(char * buff, unsigned long * ethhdr_type)
{
    PRX_DESC                    pRD;
    unsigned short              frame_length;
    int ret;
    int i;
    ret = removeq(&rxbuff_cq_mon, (unsigned char *)buff);
    //ret = removeq(&rxbuff_cq_mon);

    if(ret == CQ_ELEMENT_REMOVE_ERROR)
    {
       return 0;
    }
    else 
    {
        frame_length = recv_lengths[ret];
        //pRD = &(pInfo->hw.aRDRing[ret]);
        //memcpy((void *)buff, (void *)pRD->buff_addr, frame_length);
        recv_lengths[ret] = 0;
        //rtl_printf(" \n\n\n\n\n ");
        //*ethhdr_type = *(unsigned short *)(((unsigned short *)(pRD->buff_addr)) + 6);
        *ethhdr_type = *(unsigned short *)(((unsigned short *)(buff)) + 6);
        return frame_length;
    }
}

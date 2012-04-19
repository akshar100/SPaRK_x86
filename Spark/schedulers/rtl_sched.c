/*
 * Developed at IITB, ERTS LAB
 * 
 */

#include <arch/rtl_switch.h>  // Needed for Context Switch
#include <arch/processor.h>  // Needed for Context Switch
#include <rtl_conf.h>
#include <rtl_printf.h>
#include <arch/kbd.h>

/* A Large prime number within the range of unsigned int
 * This is least likely to be a major cycle 
 */
#define SPARK_SCHED_EOF                 3581

int	iCurrGuestOsIndex = NUM_OF_GUESTOS; 
unsigned int eta;
extern char _scheduler_data_;
unsigned int gCurrEtaCount;

/* Switches between the OSs. 
 * The number varies depending on the cyclic schedule created by 
 * the offline scheduler 
 */
unsigned int no_of_switches;

/* Structure to contain the details of one cycle of the schedule */
typedef struct cycle
{
    /* Start at start_t the execution of gos_id till end_t */
    unsigned int start_t;
    unsigned int gos_id;
    unsigned int end_t;
}schedule_cycle;

schedule_cycle offline_schedule[MAX_NO_OF_SWITCHES];

/* Code inserted for demo START */
#ifdef PERF_ANALYSIS_ON
extern struct part_switch_perf partition_switch_performace;
extern struct sched_decision_perf schedule_decision_performace;
extern struct schedule_over_eta current_schedule[2];
extern unsigned long c_s_run_no;
#endif 
/* Code inserted for demo END */        


/* Finds if the current GOS can continue executing OR
 * there is a context switch required
 * Return 1 if continue with the same context
 * Return 0 if change of context required
 */
int continue_same_context(unsigned int cur_gos_id)
{
    int i=0;  

    /* Find if the current eta is within [start_t, end_t] interval 
     * for the current_GOS
     */
    for(i=0; i<no_of_switches; i++)
    {
        if((offline_schedule[i].start_t <= gCurrEtaCount) &&
            ((offline_schedule[i].gos_id == cur_gos_id)) &&
            (offline_schedule[i].end_t > gCurrEtaCount))
        {
            /* Can continue the same context */
            return 1;
        }
    }
    /* Change of context required */
    return 0;

}

/* Function to pick the scheduler data from the binary and 
 * fill it in a usable structure
 */
int init_sched()
{
    int i,dummy;
    
    unsigned int *ptr = (unsigned int *)&_scheduler_data_; 
    /* Extract number of guest OSs */
    dummy = *ptr;
    if( dummy < NUM_OF_GUESTOS )
    	rtl_printf ("***ALERT:: Mismatch in the Number of Guest OSes ****");
    
    ptr++;
    /* Extract value of eta. This is the cycle time after which the 
     * execution will repeat 
     */
    eta = *ptr;
    /* Extract the number of switches */
    /* Search for EOF marker 
     * The value previous to that is the value of no_of_switches
     */
    while(*ptr != SPARK_SCHED_EOF)
        ptr++;
    ptr--;
    no_of_switches = *ptr;
    ptr = (unsigned int *)&_scheduler_data_;
    ptr++;
    /* Extract cyclic schedule detaiils */
    for(i=0; i< no_of_switches; i++ )
    {
        ptr++;
        offline_schedule[i].start_t = *ptr;
        ptr++;
        offline_schedule[i].gos_id = *ptr;
        ptr++;
        offline_schedule[i].end_t = *ptr;
    }
    gCurrEtaCount = eta-1;

#ifdef PERF_ANALYSIS_ON
    /* Code inserted for demo START */
    current_schedule[0].no_of_switches = no_of_switches + 1;
    current_schedule[1].no_of_switches = no_of_switches + 1;
    /* Code inserted for demo END */
#endif
    
    return 0;
}

/* Function to find the next GOS to be scheduled if a context switch*/
struct thread_struct * fGetNextGOSToSchedule()
{
    int i;
    for(i=0; i<no_of_switches; i++)
    {
        /* Schedule GOS with start_t = current Eta */
        if(offline_schedule[i].start_t == gCurrEtaCount)
        {
             return &(guestOS_thread[offline_schedule[i].gos_id]);
        }    
    }        
    /* If no such GOS present return NULL
     * This caueses control to go to SParK
     */ 
    return NULL;

}

/* Function to carry out context switch between partitions */
int spark_switch_to(struct thread_struct *next_gos)
{
    struct thread_struct *prev_gos = &guestOS_thread[iCurrGuestOsIndex];
    iCurrGuestOsIndex = next_gos->iGOSNumber;
    fLoadNewMemContext(next_gos);
    (*(next_gos->pllRealTime)) = _i8254_clock.value;

    rtl_switch_to(prev_gos, next_gos);
   
    if (kbd_guest_os == iCurrGuestOsIndex)
        *kbd_data_gos = spark_kbd_data;

    return 0;
}

/* Function called after every times tick 
 * This does the scheduling 
 */
int spark_schedule(void)
{
    struct thread_struct *next_GOS_to_sched;
    int i;
    int cur_gos_id;
    hrtime_t now;

    /* Schedule repeats after every cycle */
    if(gCurrEtaCount >= eta) 
    {
        //printf("gCurrEtaCount = %d \t eta = %d\n", gCurrEtaCount, eta);
        gCurrEtaCount = 0;
#ifdef PERF_ANALYSIS_ON
        /* Code inserted for demo START */
        now = gethrtime();
        current_schedule[c_s_run_no].schedule_over_eta[no_of_switches+1].end_time = now;

        c_s_run_no = (c_s_run_no +1)%2;
        current_schedule[c_s_run_no].index = 0;
        current_schedule[c_s_run_no].schedule_over_eta[0].start_time = now;
        current_schedule[c_s_run_no].schedule_over_eta[0].end_time = 0;
        for(i=1; i<MAX_NO_OF_SWITCHES; i++)
        {
            current_schedule[c_s_run_no].schedule_over_eta[i].start_time = 0;
            current_schedule[c_s_run_no].schedule_over_eta[i].end_time = 0;
        }
        //current_schedule[c_s_run_no].schedule_over_eta[0].start_time = now;
        /* Code inserted for demo END */
#endif
        next_GOS_to_sched = fGetNextGOSToSchedule();
    }
    /* Within a cycle */
    else
    {
        cur_gos_id =  iCurrGuestOsIndex;
        //      printf("gCurrEtaCount = %d \t\t\t\t\t\t %d\n", gCurrEtaCount, cur_gos_id);
        /* Find if the same GOS can continue execution */
        /* If NO then find out the the next GOS to be scheduled */
        if ((continue_same_context(cur_gos_id)) == 0 )
        {
            next_GOS_to_sched = fGetNextGOSToSchedule();
        }
        /* If YES then do nothing */
        else 
        {
#ifdef PERF_ANALYSIS_ON
            /* Code inserted for demo START */
            schedule_decision_performace.t2 = gethrtime();
            /* Code inserted for demo END */
#endif
            return 0;
        }
    }
    /* This is true when all the GOSs are getting their deserved share
     * and still there is some time left within the cycle
     * Right now we keep this time with SParK
     */
    if(next_GOS_to_sched == NULL)
    {
          if(iCurrGuestOsIndex ==  NUM_OF_GUESTOS)
          {
#ifdef PERF_ANALYSIS_ON
                  /* Code inserted for demo START */
                  schedule_decision_performace.t2 = gethrtime();
                  /* Code inserted for demo END */
#endif
                  return -1;
          }
          else
                  next_GOS_to_sched = &guestOS_thread[NUM_OF_GUESTOS];
    }
#ifdef PERF_ANALYSIS_ON
    /* Code inserted for demo START */
    schedule_decision_performace.t2 = gethrtime();         
    /* Code inserted for demo END */        
#endif

    /* Do the switch */
    spark_switch_to(next_GOS_to_sched);

#ifdef PERF_ANALYSIS_ON
    /* Code inserted for demo START */
    i = current_schedule[c_s_run_no].index;
    now = gethrtime(); 
    current_schedule[c_s_run_no].schedule_over_eta[i].end_time = now; 
    current_schedule[c_s_run_no].index++;
    current_schedule[c_s_run_no].schedule_over_eta[i+1].start_time = now;
    current_schedule[c_s_run_no].schedule_over_eta[i+1].gos_id = iCurrGuestOsIndex; 
    /* Code inserted for demo END */        
#endif

    return 0;
}

unsigned long kill_gos(unsigned long gos_id_to_kill)
{
    int i;
    
    if(gos_id_to_kill >= NUM_OF_GUESTOS)
        return 0xFFFF;
    for(i=0; i<no_of_switches; i++)
    {
        if(offline_schedule[i].gos_id == gos_id_to_kill) 
        {
             offline_schedule[i].gos_id = NUM_OF_GUESTOS; 
        }
    }
    return gos_id_to_kill;
}


unsigned int current_gos_schedule(unsigned char * psched)
{
    int i;
    int cs_no = (c_s_run_no+1)%2;
    int switches = (current_schedule[cs_no].no_of_switches) + 1; 
    memcpy(psched, (unsigned char *)(&current_schedule[cs_no].schedule_over_eta[0]), (switches*(8+8+4))) ;
    return (switches*(8+8+4));
}

/**
 * @file os.c
 *
 * @brief A Real Time Operating System
 *
 * Our implementation of the operating system described by Mantis Cheng in os.h.
 *
 * @author Scott Craig
 * @author Justin Tanner
 */

#include "kernel.h"
#include "os.h"
#include "error_code.h"
#include "../Arduino/Arduino.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Needed for memset */
/* #include <string.h> */

/** @brief main function provided by user application. The first task to run. */
extern int r_main(void);

/** The task descriptor of the currently RUNNING task. */
static task_descriptor_t* cur_task = NULL;

/** Since this is a "full-served" model, the kernel is executing using its own stack. */
static volatile uint16_t kernel_sp;

/** This table contains all task descriptors, regardless of state, plus idler. */
static task_descriptor_t task_desc[MAXPROCESS + 1];

/** The special "idle task" at the end of the descriptors array. */
static task_descriptor_t* idle_task = &task_desc[MAXPROCESS];

/** The current kernel request. */
static volatile kernel_request_t kernel_request;

/** Arguments for Task_Create() request. */
static volatile create_args_t kernel_request_create_args;

/** Return value for Task_Create() request. */
static volatile int kernel_request_retval;

/** Number of tasks created so far */
static queue_t dead_pool_queue;

/** Keeps track of how long the current periodic task has been delayed by a system task */
static uint16_t periodic_time_preempted = 0;

/** Keeps track of how many periodic tasks are currently ready **/
static uint8_t number_ready = 0;

/** The ready queue for RR tasks. Their scheduling is round-robin. */
static queue_t rr_queue;

/** The ready queue for SYSTEM tasks. Their scheduling is first come, first served. */
static queue_t system_queue;

/** The ready queue for PERIODIC tasks. Their scheduling is based on period and start time. */
static queue_t periodic_queue;

/** Error message used in OS_Abort() */
static uint8_t volatile error_msg = ERR_RUN_1_USER_CALLED_OS_ABORT;

/* Forward declarations */
/* kernel */
static void kernel_main_loop(void);
static void kernel_dispatch(void);
static void kernel_handle_request(void);
/* context switching */
static void exit_kernel(void) __attribute((noinline, naked));
static void enter_kernel(void) __attribute((noinline, naked));
extern "C" void TIMER1_COMPA_vect(void) __attribute__ ((signal, naked));

static int kernel_create_task();
static void kernel_terminate_task(void);
/* queues */

static void enqueue(queue_t* queue_ptr, task_descriptor_t* task_to_add);
static task_descriptor_t* dequeue(queue_t* queue_ptr);

static void idle (void);
static void _delay_25ms(void);

/** Keeps track of the current tick **/
uint16_t clock;

// Globals added for implementing services
static uint8_t service_count;
static SERVICE services[MAXSERVICE];
static service_args_t kernel_request_service_args;

/*
 * FUNCTIONS
 */
/**
 *  @brief The idle task does nothing but busy loop.
 */
static void idle (void)
{
    for(;;)
    {};
}


/**
 * @fn kernel_main_loop
 *
 * @brief The heart of the RTOS, the main loop where the kernel is entered and exited.
 *
 * The complete function is:
 *
 *  Loop
 *<ol><li>Select and dispatch a process to run</li>
 *<li>Exit the kernel (The loop is left and re-entered here.)</li>
 *<li>Handle the request from the process that was running.</li>
 *<li>End loop, go to 1.</li>
 *</ol>
 */
static void kernel_main_loop(void)
{
    for(;;)
    {
        kernel_dispatch();

        exit_kernel();

        /* if this task makes a system call, or is interrupted,
         * the thread of control will return to here. */

        kernel_handle_request();
    }
}


/**
 * @fn kernel_dispatch
 *
 *@brief The second part of the scheduler.
 *
 * Chooses the next task to run.
 *
 */
static void kernel_dispatch(void)
{
    /* If the current state is RUNNING, then select it to run again.
     * kernel_handle_request() has already determined it should be selected.
     * Do not select a task to run if it is in the WAITING state
     */
	uint8_t i;
	
	/* Check how many periodic tasks are ready */
	number_ready = 0;
	for(i = 0; i < periodic_queue.size; i++)
	{
		task_descriptor_t* next_task = dequeue(&periodic_queue);		
		if((next_task->state == READY)&&(next_task->nrt == 0))
		{
			number_ready ++;
		}
		enqueue(&periodic_queue, next_task);
	}
	
	/* If more than one task is ready, that is a scheduling error */
	if (number_ready > 1)
	{
		error_msg = ERR_RUN_7_PERIODIC_TASK_COLLISION;
		OS_Abort();
	}
	
    if(cur_task->state != RUNNING || cur_task == idle_task)
    {
		/* System tasks will preempt periodic and RR tasks */
        if(system_queue.head != NULL)
        {
            for(i = 0; i < system_queue.size; i++)
            {
			    task_descriptor_t* next_task = dequeue(&system_queue);
                if((next_task->state == READY))
                {
                    cur_task = next_task;
                    cur_task->state = RUNNING;
                    return;
                }
                else
                {
                    enqueue(&system_queue, next_task);
                }
            }
        }
		
		/* We can assume number_ready is 0 or 1 because >1 aborts the OS above */
		if(periodic_queue.head != NULL)
		{

			int temp_queue_size = periodic_queue.size;
			for(i = 0; i < temp_queue_size; i++)
			{
				task_descriptor_t* next_task = dequeue(&periodic_queue);
				
				if((next_task->state == READY)&&(next_task->nrt == 0))
				{
					if (next_task->elapsed > next_task->wcet) {
						error_msg = ERR_RUN_3_PERIODIC_TOOK_TOO_LONG;
						OS_Abort();
					}
					cur_task = next_task;
				}
				else
				{
					enqueue(&periodic_queue, next_task);
				}
			}
			
		}
		
		/* If one task is ready, set new task to run or non-finished task to continue running */
		if (number_ready == 1)
		{
			cur_task->state = RUNNING;
			return;
		}
		
        if(rr_queue.head != NULL)
        {
	        for(i = 0; i < rr_queue.size; i++)
	        {
		        task_descriptor_t* next_task = dequeue(&rr_queue);
		        if(next_task->state == READY)
		        {
			        cur_task = next_task;
			        cur_task->state = RUNNING;
			        return;
		        }
		        else
		        {
			        enqueue(&rr_queue, next_task);
		        }
	       }
       }
    
       cur_task = idle_task;
       cur_task->state = RUNNING;
    }
}


/**
 * @fn kernel_handle_request
 *
 *@brief The first part of the scheduler.
 *
 * Perform some action based on the system call or timer tick.
 * Perhaps place the current process in a ready or waitng queue.
 */
static void kernel_handle_request(void)
{
   switch(kernel_request)
    {
    case NONE:
        /* Should not happen. */
        break;

    case TIMER_EXPIRED:		
		/* Update the time to run for PERIODIC tasks */
		int i;
		for(i = 0; i < periodic_queue.size; i++)
		{
			task_descriptor_t* next_task = dequeue(&periodic_queue);
			if (next_task->nrt > 0)
			{
				next_task->nrt --;
			}			
			enqueue(&periodic_queue, next_task);
		}
		
		/* Periodic tasks must be checked first so we know if we are preempting */
		/* If preempting a periodic task we need to stretch the wcet */
		/* If a system task is preempting a periodic task, keep track of how long */
		if ((cur_task->level == SYSTEM)&&(number_ready == 1)) {
			periodic_time_preempted ++;
		}
		
		/* Periodic tasks can be pre-empted by system tasks. */
		if(cur_task->level == PERIODIC && cur_task->state == RUNNING)
		{
			cur_task->state = READY;
			cur_task->elapsed ++;
			enqueue(&periodic_queue, cur_task);
		}
		
        /* Round robin tasks get pre-empted on every tick. */
        if(cur_task->level == RR && cur_task->state == RUNNING)
        {
            cur_task->state = READY;
            enqueue(&rr_queue, cur_task);
        }
        break;

    case TASK_CREATE:
        kernel_request_retval = kernel_create_task();

        /* Check if new task has higher priority, and that it wasn't an ISR
         * making the request.
         */
        if(kernel_request_retval)
        {
            /* If new task is SYSTEM and cur is not, then don't run old one */
            if(kernel_request_create_args.level == SYSTEM && cur_task->level != SYSTEM)
            {
                cur_task->state = READY;
            }

            /* If cur is RR, it might be pre-empted by a new PERIODIC. */
			if(cur_task->level == RR && kernel_request_create_args.level == PERIODIC) 
			{
				cur_task->state = READY;
			}
			
			/* enqueue READY PERIODIC tasks. */
			if(cur_task->level == PERIODIC && cur_task->state == READY)
			{
				enqueue(&periodic_queue, cur_task);
			}
			
            /* enqueue READY RR tasks. */
            if(cur_task->level == RR && cur_task->state == READY)
            {
                enqueue(&rr_queue, cur_task);
            }
        }
        break;

    case TASK_TERMINATE:
		if(cur_task != idle_task)
		{
        	kernel_terminate_task();
		}
        break;

    case TASK_NEXT:
		switch(cur_task->level)
		{
	    case SYSTEM:
	        enqueue(&system_queue, cur_task);
			break;

	    case PERIODIC:
			
			/* if a periodic task is preempted for so long that it misses an entire period,
			 * that is an error! */
			if ((cur_task->elapsed + periodic_time_preempted) > cur_task->period) {
				error_msg = ERR_RUN_8_PERIODIC_PREEMPTED_TOO_LONG;
				OS_Abort();
			}
			
			/* Update next run time of completed task based on elapsed time and preempted time */
			cur_task->nrt = cur_task->period -  cur_task->elapsed - periodic_time_preempted;
			cur_task->elapsed = 0;
			periodic_time_preempted = 0;
			enqueue(&periodic_queue, cur_task);
	        break;

	    case RR:
	        enqueue(&rr_queue, cur_task);
	        break;

	    default: /* idle_task */
			break;
		}

		cur_task->state = READY;
        break;

    case TASK_GET_ARG:
        /* Should not happen. Handled in task itself. */
        break;
		
    case SUBSCRIBE:
    {
        SERVICE* serv = kernel_request_service_args.service;
			
        switch(cur_task->level)
        {
            case PERIODIC:
                error_msg = ERR_RUN_6_PERIODIC_IPC_ERROR;
                OS_Abort();
					
            case SYSTEM:
                enqueue(&system_queue, cur_task);
                break;

            case RR:
                enqueue(&rr_queue, cur_task);
                break;
				
            default:
                break;
         }
				
         serv->retvals[serv->subscribers_count] = kernel_request_service_args.retval;
         serv->subscribers[serv->subscribers_count] = cur_task;
         serv->subscribers_count++;
		   
         cur_task->state = WAITING;  
    }
		
        break;
	
      case PUBLISH:
	  {
           SERVICE* serv = kernel_request_service_args.service;
           int i;
           for(i = 0; i < serv->subscribers_count; i++)
           {
               *(serv->retvals[i]) = *(kernel_request_service_args.retval);
               serv->subscribers[i]->state = READY;
           }
		   
           serv->subscribers_count = 0;
		   
		   switch(cur_task->level)
		   {
		       case SYSTEM:
			       break;

	           case PERIODIC:
				   cur_task->state = READY;
			       enqueue(&periodic_queue, cur_task);
	               break;

	           case RR:
			       cur_task->state = READY;
	               enqueue(&rr_queue, cur_task);
	               break;

	          default: 
			       break;
		   }
		   
		   
		   break;
      }
	   
    default:
        /* Should never happen */
        error_msg = ERR_RUN_5_RTOS_INTERNAL_ERROR;
        OS_Abort();
        break;
    }

    kernel_request = NONE;
}


/*
 * Context switching
 */
/**
 * It is important to keep the order of context saving and restoring exactly
 * in reverse. Also, when a new task is created, it is important to
 * initialize its "initial" context in the same order as a saved context.
 *
 * Save r31 and SREG on stack, disable interrupts, then save
 * the rest of the registers on the stack. In the locations this macro
 * is used, the interrupts need to be disabled, or they already are disabled.
 */
#define    SAVE_CTX_TOP()       asm volatile (\
    "push   r31             \n\t"\
    "in     r31,__SREG__    \n\t"\
    "cli                    \n\t"::); /* Disable interrupt */

#define STACK_SREG_SET_I_BIT()    asm volatile (\
    "ori    r31, 0x80        \n\t"::);

#define    SAVE_CTX_BOTTOM()       asm volatile (\
    "push   r31             \n\t"\
    "in     r31, 0x3c       \n\t"\
    "push   r31             \n\t"\
    "push   r30             \n\t"\
    "push   r29             \n\t"\
    "push   r28             \n\t"\
    "push   r27             \n\t"\
    "push   r26             \n\t"\
    "push   r25             \n\t"\
    "push   r24             \n\t"\
    "push   r23             \n\t"\
    "push   r22             \n\t"\
    "push   r21             \n\t"\
    "push   r20             \n\t"\
    "push   r19             \n\t"\
    "push   r18             \n\t"\
    "push   r17             \n\t"\
    "push   r16             \n\t"\
    "push   r15             \n\t"\
    "push   r14             \n\t"\
    "push   r13             \n\t"\
    "push   r12             \n\t"\
    "push   r11             \n\t"\
    "push   r10             \n\t"\
    "push   r9              \n\t"\
    "push   r8              \n\t"\
    "push   r7              \n\t"\
    "push   r6              \n\t"\
    "push   r5              \n\t"\
    "push   r4              \n\t"\
    "push   r3              \n\t"\
    "push   r2              \n\t"\
    "push   r1              \n\t"\
    "push   r0              \n\t"::);

/**
 * @brief Push all the registers and SREG onto the stack.
 */
#define    SAVE_CTX()    SAVE_CTX_TOP();SAVE_CTX_BOTTOM();

/**
 * @brief Pop all registers and the status register.
 */
#define    RESTORE_CTX()    asm volatile (\
    "pop    r0                \n\t"\
    "pop    r1                \n\t"\
    "pop    r2                \n\t"\
    "pop    r3                \n\t"\
    "pop    r4                \n\t"\
    "pop    r5                \n\t"\
    "pop    r6                \n\t"\
    "pop    r7                \n\t"\
    "pop    r8                \n\t"\
    "pop    r9                \n\t"\
    "pop    r10             \n\t"\
    "pop    r11             \n\t"\
    "pop    r12             \n\t"\
    "pop    r13             \n\t"\
    "pop    r14             \n\t"\
    "pop    r15             \n\t"\
    "pop    r16             \n\t"\
    "pop    r17             \n\t"\
    "pop    r18             \n\t"\
    "pop    r19             \n\t"\
    "pop    r20             \n\t"\
    "pop    r21             \n\t"\
    "pop    r22             \n\t"\
    "pop    r23             \n\t"\
    "pop    r24             \n\t"\
    "pop    r25             \n\t"\
    "pop    r26             \n\t"\
    "pop    r27             \n\t"\
    "pop    r28             \n\t"\
    "pop    r29             \n\t"\
    "pop    r30             \n\t"\
    "pop    r31             \n\t"\
    "out    0x3c, r31       \n\t"\
	"pop    r31             \n\t"\
    "out    __SREG__, r31   \n\t"\
    "pop    r31             \n\t"::);


/**
 * @fn exit_kernel
 *
 * @brief The actual context switching code begins here.
 *
 * This function is called by the kernel. Upon entry, we are using
 * the kernel stack, on top of which is the address of the instruction
 * after the call to exit_kernel().
 *
 * Assumption: Our kernel is executed with interrupts already disabled.
 *
 * The "naked" attribute prevents the compiler from adding instructions
 * to save and restore register values. It also prevents an
 * automatic return instruction.
 */
static void exit_kernel(void)
{
    /*
     * The PC was pushed on the stack with the call to this function.
     * Now push on the I/O registers and the SREG as well.
     */
     SAVE_CTX();

    /*
     * The last piece of the context is the SP. Save it to a variable.
     */
    kernel_sp = SP;

    /*
     * Now restore the task's context, SP first.
     */
    SP = (uint16_t)(cur_task->sp);

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * return explicitly required as we are "naked".
     * Interrupts are enabled or disabled according to SREG
     * recovered from stack, so we don't want to explicitly
     * enable them here.
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}


/**
 * @fn enter_kernel
 *
 * @brief All system calls eventually enter here.
 *
 * Assumption: We are still executing on cur_task's stack.
 * The return address of the caller of enter_kernel() is on the
 * top of the stack.
 */
static void enter_kernel(void)
{
    /*
     * The PC was pushed on the stack with the call to this function.
     * Now push on the I/O registers and the SREG as well.
     */
    SAVE_CTX();

    /*
     * The last piece of the context is the SP. Save it to a variable.
     */
    cur_task->sp = (uint8_t*)SP;

    /*
     * Now restore the kernel's context, SP first.
     */
    SP = kernel_sp;

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * return explicitly required as we are "naked".
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}


/**
 * @fn TIMER1_COMPA_vect
 *
 * @brief The interrupt handler for output compare interrupts on Timer 1
 *
 * Used to enter the kernel when a tick expires.
 *
 * Assumption: We are still executing on the cur_task stack.
 * The return address inside the current task code is on the top of the stack.
 *
 * The "naked" attribute prevents the compiler from adding instructions
 * to save and restore register values. It also prevents an
 * automatic return instruction.
 */
void TIMER1_COMPA_vect(void)
{
		DDRB |= _BV(6);
		
		PORTB |= _BV(6);
		PORTB &= ~_BV(6);
    /*
     * Save the interrupted task's context on its stack,
     * and save the stack pointer.
     *
     * On the cur_task's stack, the registers and SREG are
     * saved in the right order, but we have to modify the stored value
     * of SREG. We know it should have interrupts enabled because this
     * ISR was able to execute, but it has interrupts disabled because
     * it was stored while this ISR was executing. So we set the bit (I = bit 7)
     * in the stored value.
     */
    SAVE_CTX_TOP();

    STACK_SREG_SET_I_BIT();

    SAVE_CTX_BOTTOM();
	
	clock += 1;

    cur_task->sp = (uint8_t*)SP;

    /*
     * Now that we already saved a copy of the stack pointer
     * for every context including the kernel, we can move to
     * the kernel stack and use it. We will restore it again later.
     */
    SP = kernel_sp;

    /*
     * Inform the kernel that this task was interrupted.
     */
    kernel_request = TIMER_EXPIRED;

    /*
     * Prepare for next tick interrupt.
     */
    OCR1A += TICK_CYCLES;

    /*
     * Restore the kernel context. (The stack pointer is restored again.)
     */
    SP = kernel_sp;

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * We use "ret" here, not "reti", because we do not want to
     * enable interrupts inside the kernel.
     * Explilictly required as we are "naked".
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}


/*
 * Tasks Functions
 */
/**
 *  @brief Kernel function to create a new task.
 *
 * When creating a new task, it is important to initialize its stack just like
 * it has called "enter_kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * @sa enter_kernel
 */
static int kernel_create_task()
{
    /* The new task. */
    task_descriptor_t *p;
    uint8_t* stack_bottom;


    if (dead_pool_queue.head == NULL)
    {
        error_msg = ERR_RUN_2_TOO_MANY_TASKS;
		OS_Abort();
        return 0;
    }

	/* idling "task" goes in last descriptor. */
	if(kernel_request_create_args.level == NULL)
	{
		p = &task_desc[MAXPROCESS];
	}
	/* Find an unused descriptor. */
	else
	{
	    p = dequeue(&dead_pool_queue);
	}

    stack_bottom = &(p->stack[MAXSTACK-1]);

    /* The stack grows down in memory, so the stack pointer is going to end up
     * pointing to the location 32 + 1 + 1 + 3 + 3 = 40 bytes above the bottom, to make
     * room for (from bottom to top):
     *   the address of Task_Terminate() to destroy the task if it ever returns,
     *   the address of the start of the task to "return" to the first time it runs,
     *   register 31,
     *   the stored SREG
	 *	 the stored EIND 
     *   registers 30 to 0.
     */
    uint8_t* stack_top = stack_bottom - (32 + 1 + 1 + 3 + 3);

    // stack_top[0] is the byte above the stack.
    // stack_top[1] is r0. 
    stack_top[2] = (uint8_t) 0; // r1 is the "zero" register. 
    // stack_top[31] is r30. 
    stack_top[32] = 0 ; // set the EIND register to initially be zero
    stack_top[33] = (uint8_t) _BV(SREG_I); // set SREG_I bit in stored SREG. 
    // stack_top[34] is r31.

    /* We are placing the address (24-bit) of the functions
     * onto the stack in reverse byte order (least significant first, followed
     * by most significant).  This is because the "return" assembly instructions
     * (ret and reti) pop addresses off in BIG ENDIAN (most sig. first, least sig.
     * second), even though the AT90 is LITTLE ENDIAN machine.
	 *
     * Each function pointer now consists of an extra 8 bits due to an extra byte 
	 * being needed to reference them. This is byte is initialized to zero.
	 *
	 */ 
    stack_top[35] = (uint8_t)(0);
    stack_top[36] = (uint8_t)((uint16_t)(kernel_request_create_args.f) >> 8);
    stack_top[37] = (uint8_t)(uint16_t)(kernel_request_create_args.f);
    stack_top[38] = (uint8_t)(0);
    stack_top[39] = (uint8_t)((uint16_t)Task_Terminate >> 8);
    stack_top[40] = (uint8_t)(uint16_t)Task_Terminate;

    /*
     * Make stack pointer point to cell above stack (the top).
     * Make room for 32 registers, SREG, EIND,  and two return addresses.
     */
    p->sp = stack_top;

    p->state = READY;
    p->arg = kernel_request_create_args.arg;
    p->level = kernel_request_create_args.level;
	
	/* Default values for periodic attributes = 0 */
	/* TODO: Maybe do this elsewhere */
	p->period = 0;
	p->wcet = 0;
	p->nrt = 0;
	p->elapsed = 0;

	switch(kernel_request_create_args.level)
	{
	case PERIODIC:
		p->period = kernel_request_create_args.period;
		p->wcet = kernel_request_create_args.wcet;
		p->nrt = kernel_request_create_args.start;
		enqueue(&periodic_queue, p);
		break;

    case SYSTEM:
        enqueue(&system_queue, p);
		break;

    case RR:
        enqueue(&rr_queue, p);
		break;

	default:
		/* idle task does not go in a queue */
		break;
	}

    return 1;
}


/**
 * @brief Kernel function to destroy the current task.
 */
static void kernel_terminate_task(void)
{
    /* deallocate all resources used by this task */
    cur_task->state = DEAD;
    enqueue(&dead_pool_queue, cur_task);
}

/*
 * Queue manipulation.
 */

/**
 * @brief Add a task the head of the queue
 *
 * @param queue_ptr the queue to insert in
 * @param task_to_add the task descriptor to add
 */
static void enqueue(queue_t* queue_ptr, task_descriptor_t* task_to_add)
{
    task_to_add->next = NULL;

    if(queue_ptr->head == NULL)
    {
        /* empty queue */
        queue_ptr->head = task_to_add;
        queue_ptr->tail = task_to_add;
    }
    else
    {
        /* put task at the back of the queue */
        queue_ptr->tail->next = task_to_add;
        queue_ptr->tail = task_to_add;
    }
	
    queue_ptr->size++;
}


/**
 * @brief Pops head of queue and returns it.
 *
 * @param queue_ptr the queue to pop
 * @return the popped task descriptor
 */
static task_descriptor_t* dequeue(queue_t* queue_ptr)
{
    task_descriptor_t* task_ptr = queue_ptr->head;

    if(queue_ptr->head != NULL)
    {
        queue_ptr->head = queue_ptr->head->next;
        task_ptr->next = NULL;
    }
	
    queue_ptr->size--;

    return task_ptr;
}

#undef SLOW_CLOCK

#ifdef SLOW_CLOCK
/**
 * @brief For DEBUGGING to make the clock run slower
 *
 * Divide CLKI/O by 64 on timer 1 to run at 125 kHz  CS3[210] = 011
 * 1 MHz CS3[210] = 010
 */
static void kernel_slow_clock(void)
{
    TCCR1B &= ~(_BV(CS12) | _BV(CS10));
    TCCR1B |= (_BV(CS11));
}
#endif

/**
 * @brief Setup the RTOS and create main() as the first SYSTEM level task.
 *
 * Point of entry from the C runtime crt0.S.
 */
void OS_Init()
{	
    int i;

    /* Set up the clocks */

	// Pre-scalar set to 8 
    TCCR1B |= (_BV(CS11));

#ifdef SLOW_CLOCK
    kernel_slow_clock();
#endif

    /*
     * Initialize dead pool to contain all but last task descriptor.
     *
     * DEAD == 0, already set in .init4
     */
    for (i = 0; i < MAXPROCESS - 1; i++)
    {
        task_desc[i].state = DEAD;
        task_desc[i].next = &task_desc[i + 1];
    }
    task_desc[MAXPROCESS - 1].next = NULL;
    dead_pool_queue.head = &task_desc[0];
    dead_pool_queue.tail = &task_desc[MAXPROCESS - 1];

	/* Create idle "task" */
    kernel_request_create_args.f = (voidfuncvoid_ptr)idle;
    kernel_request_create_args.level = NULL;
    kernel_create_task();

    /* Create "main" task as SYSTEM level. */
    kernel_request_create_args.f = (voidfuncvoid_ptr)r_main;
    kernel_request_create_args.level = SYSTEM;
    kernel_create_task();

    /* First time through. Select "main" task to run first. */
    cur_task = &task_desc[0];
    cur_task->state = RUNNING;
    dequeue(&system_queue);

    /* Set up Timer 1 Output Compare interrupt,the TICK clock. */
	// 
    TIMSK1 |= _BV(OCIE1A);
    OCR1A = TCNT1 + TICK_CYCLES;
    /* Clear flag. */
    TIFR1 = _BV(OCF1A);

    clock = 0;
    service_count = 0;
    /*
     * The main loop of the RTOS kernel.
     */
    kernel_main_loop();
}




/**
 *  @brief Delay function adapted from <util/delay.h>
 */
static void _delay_25ms(void)
{
    //uint16_t i;

    /* 4 * 50000 CPU cycles = 25 ms */
    //asm volatile ("1: sbiw %0,1" "\n\tbrne 1b" : "=w" (i) : "0" (50000));
    _delay_ms(25);
}


/** @brief Abort the execution of this RTOS due to an unrecoverable erorr.
 */
void OS_Abort(void)
{
    uint8_t i, j;
    uint8_t flashes, mask;

    Disable_Interrupt();

    if(error_msg < ERR_RUN_1_USER_CALLED_OS_ABORT)
    {
        flashes = error_msg + 1;
        mask = LED_GREEN_MASK;
    }
    else
    {
        flashes = error_msg + 1 - ERR_RUN_1_USER_CALLED_OS_ABORT;
        mask = LED_RED_MASK;
    }

	/* Initialize port for output */
	DDRL |= mask;

    for(;;)
    {
        PORTL |= mask;

        for(i = 0; i < 100; ++i)
        {
               _delay_25ms();
        }

        PORTL &= ~mask;

        for(i = 0; i < 40; ++i)
        {
               _delay_25ms();
        }


        for(j = 0; j < flashes; ++j)
        {
            PORTL |= mask;

            for(i = 0; i < 10; ++i)
            {
                _delay_25ms();
            }

            PORTL &= ~mask;

            for(i = 0; i < 10; ++i)
            {
                _delay_25ms();
            }
        }

        for(i = 0; i < 20; ++i)
        {
            _delay_25ms();
        }
    }
}

/** @brief Create a new SYSTEM task.
 */
int8_t   Task_Create_System(void (*f)(void), int16_t arg)
{
    int retval;
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request_create_args.f = (voidfuncvoid_ptr)f;
    kernel_request_create_args.arg = arg;
    kernel_request_create_args.level = SYSTEM;

    kernel_request = TASK_CREATE;
    enter_kernel();

    retval = kernel_request_retval;
    SREG = sreg;

    return retval;	
}

/** @brief Create a new RR task.
 */
int8_t   Task_Create_RR(    void (*f)(void), int16_t arg)
{
    int retval;
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request_create_args.f = (voidfuncvoid_ptr)f;
    kernel_request_create_args.arg = arg;
    kernel_request_create_args.level = RR;

    kernel_request = TASK_CREATE;
    enter_kernel();

    retval = kernel_request_retval;
    SREG = sreg;

    return retval;
}

/** @brief Create a new PERIODIC task.
 */
int8_t Task_Create_Periodic(void(*f)(void), int16_t arg, uint16_t period, uint16_t wcet, uint16_t start)
{	
	/* You can't have wcet greater than period or equal to zero */
	/* This also prevents period from being equal to zero */
	if (wcet > period) 
	{
		error_msg = ERR_1_WCET_GREATER_THAN_PERIOD;
		OS_Abort();
	}
	
	if (wcet == 0)
	{
		error_msg = ERR_2_WCET_EQUALS_ZERO;
		OS_Abort();
	}
	
	int retval;
	uint8_t sreg;

	sreg = SREG;
	Disable_Interrupt();

	kernel_request_create_args.f = (voidfuncvoid_ptr)f;
	kernel_request_create_args.arg = arg;
	kernel_request_create_args.level = PERIODIC;
	kernel_request_create_args.period = (uint16_t)period;
	kernel_request_create_args.wcet = (uint16_t)wcet;
	kernel_request_create_args.start = (uint16_t)start;

	kernel_request = TASK_CREATE;
	enter_kernel();

	retval = kernel_request_retval;
	SREG = sreg;

	return retval;
}


/**
  * @brief The calling task gives up its share of the processor voluntarily.
  */
void Task_Next()
{
    uint8_t volatile sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = TASK_NEXT;
    enter_kernel();

    SREG = sreg;
}


/**
  * @brief The calling task terminates itself.
  */
void Task_Terminate()
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = TASK_TERMINATE;
    enter_kernel();

    SREG = sreg;
}


/** @brief Retrieve the assigned parameter.
 */
int Task_GetArg(void)
{
    int arg;
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    arg = cur_task->arg;

    SREG = sreg;

    return arg;
}

/* 
 * @brief Retrieves the current time, down to the millisecond
 */
uint16_t Now()
{
	uint8_t sreg;

    sreg = SREG;
	Disable_Interrupt();
	uint16_t now = (clock * TICK) + (TICK - (uint16_t) ((OCR1A - TCNT1) / MS_CYCLES));
	SREG = sreg;
	return now;	
}

/* 
 * @brief Initializes a new service
 */
SERVICE *Service_Init()
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    if(service_count >= MAXSERVICE)
    {
        return NULL;
    }

    services[service_count].subscribers_count = 0;
    service_count ++;

    SREG = sreg;	
	
    return &services[service_count - 1];
}

/* 
 * @brief Subscribe to a service
 */
void Service_Subscribe(SERVICE *s, int16_t *v)
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request_service_args.service = s;
    kernel_request_service_args.retval = v;
      
    kernel_request = SUBSCRIBE;
    enter_kernel();

    SREG = sreg;
}

/*
 * @brief Publish a notification to a service
 */
void Service_Publish(SERVICE *s, int16_t v)
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();
	
    kernel_request_service_args.service = s;
    kernel_request_service_args.retval = &v;
	 
    kernel_request = PUBLISH;
    enter_kernel();

    SREG = sreg;
}

/**
 * Runtime entry point into the program; just start the RTOS.  The application layer must define r_main() for its entry point.
 */
int main()
{	
	init();
	Disable_Interrupt();
		
	OS_Init();
	return 0;
}


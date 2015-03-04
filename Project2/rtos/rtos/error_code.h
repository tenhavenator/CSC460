/**
 * @file   error_code.h
 *
 * @brief Error messages returned in OS_Abort().
 *        Green errors are initialization errors
 *        Red errors are runt time errors
 *
 * CSC 460/560 Real Time Operating Systems - Mantis Cheng
 *
 * @author Scott Craig
 * @author Justin Tanner
 */
#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

enum {

/** GREEN ERRORS -- Initialize time errors. */
    
/** PERIODIC worst-case execution time must be less than period */
ERR_1_WCET_GREATER_THAN_PERIOD,

/** PERIODIC wcet must be > 0 **/
ERR_2_WCET_EQUALS_ZERO,

/** PERIODIC period must be > 0 **/
ERR_3_PERIOD_EQUALS_ZERO,

/** RED ERRORS -- Run time errors. */

/** User called OS_Abort() */
ERR_RUN_1_USER_CALLED_OS_ABORT,

/** Too many tasks created. Only allowed MAXPROCESS at any time.*/
ERR_RUN_2_TOO_MANY_TASKS,

/** PERIODIC task still running at end of time slot. */
ERR_RUN_3_PERIODIC_TOOK_TOO_LONG,

/** ISR made a request that only tasks are allowed. */
ERR_RUN_4_ILLEGAL_ISR_KERNEL_REQUEST,

/** RTOS Internal error in handling request. */
ERR_RUN_5_RTOS_INTERNAL_ERROR,

// A periodic task tries to subscribe to a service
ERR_RUN_6_PERIODIC_IPC_ERROR, 

// Too many subscribers are already subscribed to a service
ERR_RUN_7_TOO_MANY_SUBSCRIBERS,


};


#endif

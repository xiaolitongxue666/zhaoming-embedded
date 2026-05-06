/**
  ******************************************************************************
  * @file    platform_sys.h
  * @brief   抽象一层 OS 原语（信号量 / 互斥锁 / 邮箱 / 线程 / 临界区），
  *          让平台层同时支持裸机 (PLATFORM_OS == 0) 和 RTOS (FreeRTOS /
  *          CMSIS-RTOS v2 等)。
  *
  *          架构跟 lwIP 的 sys_arch 一致：给出统一接口语义，各家 RTOS port
  *          自己实现 platform_sys_arch.c 把 platform_sys_xxx 接到底层 OS API。
  *          裸机 mode 下整套接口退化为 nop，上层代码一字不动。
  ******************************************************************************
  */

#ifndef PLATFORM_API_PLATFORM_SYS_H_
#define PLATFORM_API_PLATFORM_SYS_H_

#include "platform_config.h"
#include "platform_def.h"

#if (PLATFORM_OS == 0)

/* For a totally minimal and standalone system, we provide null
   definitions of the platform_sys_ functions. */
typedef uint8_t platform_sys_sem_t;
typedef uint8_t platform_sys_mutex_t;
typedef uint8_t platform_sys_mbox_t;
typedef int platform_sys_prot_t;


#define platform_sys_sem_new(s, c) PLATFORM_EOK
#define platform_sys_sem_signal(s)
#define platform_sys_sem_wait(s)
#define platform_sys_sem_wait_timeout(s,t)
#define platform_sys_sem_free(s)
#define platform_sys_sem_valid(s) 0
#define platform_sys_sem_valid_val(s) 0
#define platform_sys_sem_set_invalid(s)
#define platform_sys_sem_set_invalid_val(s)
#define platform_sys_mutex_new(mu) PLATFORM_EOK
#define platform_sys_mutex_lock(mu)
#define platform_sys_mutex_unlock(mu)
#define platform_sys_mutex_free(mu)
#define platform_sys_mutex_valid(mu) 0
#define platform_sys_mutex_set_invalid(mu)
#define platform_sys_mbox_new(m, s) PLATFORM_EOK
#define platform_sys_mbox_fetch(m,d)
#define platform_sys_mbox_tryfetch(m,d)
#define platform_sys_mbox_post(m,d)
#define platform_sys_mbox_trypost(m,d)
#define platform_sys_mbox_free(m)
#define platform_sys_mbox_valid(m)
#define platform_sys_mbox_valid_val(m)
#define platform_sys_mbox_set_invalid(m)
#define platform_sys_mbox_set_invalid_val(m)

#define platform_sys_thread_new(n,t,a,s,p)

#define platform_sys_msleep(t)
#define platform_sys_arch_protect(t) PLATFORM_EOK
#define platform_sys_arch_unprotect(t) (void)t
#else

#include PLATFORM_SYS_ARCH_H

/** Return code for timeouts from platform_sys_arch_mbox_fetch and platform_sys_sem_wait_timeout */
#define PLATFORM_SYS_ARCH_TIMEOUT 0xffffffffUL

/** platform_sys_mbox_tryfetch() returns PLATFORM_SYS_MBOX_EMPTY if appropriate.
 * For now we use the same magic value, but we allow this to change in future.
 */
#define PLATFORM_SYS_MBOX_EMPTY PLATFORM_SYS_ARCH_TIMEOUT


/** Function prototype for thread functions */
typedef void (*lwip_thread_fn)(void *arg);

/* Function prototypes for functions to be implemented by platform ports
   (in platform_sys_arch.c) */

/* Mutex functions: */

/**
 * @ingroup platform_sys_mutex
 * Create a new mutex.
 * Both implementation types (recursive or non-recursive) should work.
 * The mutex is allocated to the memory that 'mutex'
 * points to (which can be both a pointer or the actual OS structure).
 * If the mutex has been created, ERR_OK should be returned. Returning any
 * other error will provide a hint what went wrong, but except for assertions,
 * no real error handling is implemented.
 *
 * @param mutex pointer to the mutex to create
 * @return ERR_OK if successful, another platform_err_t otherwise
 */
platform_err_t platform_sys_mutex_new(platform_sys_mutex_t *mutex);
/**
 * @ingroup platform_sys_mutex
 * Blocks the thread until the mutex can be grabbed.
 * @param mutex the mutex to lock
 */
void platform_sys_mutex_lock(platform_sys_mutex_t *mutex);
/**
 * @ingroup platform_sys_mutex
 * Releases the mutex previously locked through 'platform_sys_mutex_lock()'.
 * @param mutex the mutex to unlock
 */
void platform_sys_mutex_unlock(platform_sys_mutex_t *mutex);
/**
 * @ingroup platform_sys_mutex
 * Deallocates a mutex.
 * @param mutex the mutex to delete
 */
void platform_sys_mutex_free(platform_sys_mutex_t *mutex);

/**
 * @ingroup platform_sys_mutex
 * Returns 1 if the mutes is valid, 0 if it is not valid.
 * When using pointers, a simple way is to check the pointer for != NULL.
 * When directly using OS structures, implementing this may be more complex.
 * This may also be a define, in which case the function is not prototyped.
 */
int platform_sys_mutex_valid(platform_sys_mutex_t *mutex);

/**
 * @ingroup platform_sys_mutex
 * Invalidate a mutex so that platform_sys_mutex_valid() returns 0.
 * ATTENTION: This does NOT mean that the mutex shall be deallocated:
 * platform_sys_mutex_free() is always called before calling this function!
 * This may also be a define, in which case the function is not prototyped.
 */
void platform_sys_mutex_set_invalid(platform_sys_mutex_t *mutex);


/* Semaphore functions: */

/**
 * @ingroup platform_sys_sem
 * Create a new semaphore
 * Creates a new semaphore. The semaphore is allocated to the memory that 'sem'
 * points to (which can be both a pointer or the actual OS structure).
 * The "count" argument specifies the initial state of the semaphore (which is
 * either 0 or 1).
 * If the semaphore has been created, ERR_OK should be returned. Returning any
 * other error will provide a hint what went wrong, but except for assertions,
 * no real error handling is implemented.
 *
 * @param sem pointer to the semaphore to create
 * @param count initial count of the semaphore
 * @return ERR_OK if successful, another platform_err_t otherwise
 */
platform_err_t platform_sys_sem_new(platform_sys_sem_t *sem, uint8_t count);
/**
 * @ingroup platform_sys_sem
 * Signals a semaphore
 * @param sem the semaphore to signal
 */
void platform_sys_sem_signal(platform_sys_sem_t *sem);
/**
 * @ingroup platform_sys_sem
 *  Blocks the thread while waiting for the semaphore to be signaled. If the
 * "timeout" argument is non-zero, the thread should only be blocked for the
 * specified time (measured in milliseconds). If the "timeout" argument is zero,
 * the thread should be blocked until the semaphore is signalled.
 *
 * The return value is PLATFORM_SYS_ARCH_TIMEOUT if the semaphore wasn't signaled within
 * the specified time or any other value if it was signaled (with or without
 * waiting).
 * platform_sys_sem_wait(), that uses the platform_sys_sem_wait_timeout() function.
 *
 * @param sem the semaphore to wait for
 * @param timeout timeout in milliseconds to wait (0 = wait forever)
 * @return PLATFORM_SYS_ARCH_TIMEOUT on timeout, any other value on success
 */
uint32_t platform_sys_sem_wait_timeout(platform_sys_sem_t *sem, uint32_t timeout);
/**
 * @ingroup platform_sys_sem
 * Deallocates a semaphore.
 * @param sem semaphore to delete
 */
void platform_sys_sem_free(platform_sys_sem_t *sem);
/** Wait for a semaphore - forever/no timeout */
#define platform_sys_sem_wait(sem)                  platform_sys_sem_wait_timeout(sem, 0)

/**
 * @ingroup platform_sys_sem
 * Returns 1 if the semaphore is valid, 0 if it is not valid.
 * When using pointers, a simple way is to check the pointer for != NULL.
 * When directly using OS structures, implementing this may be more complex.
 * This may also be a define, in which case the function is not prototyped.
 */
int platform_sys_sem_valid(platform_sys_sem_t *sem);

/**
 * @ingroup platform_sys_sem
 * Invalidate a semaphore so that platform_sys_sem_valid() returns 0.
 * ATTENTION: This does NOT mean that the semaphore shall be deallocated:
 * platform_sys_sem_free() is always called before calling this function!
 * This may also be a define, in which case the function is not prototyped.
 */
void platform_sys_sem_set_invalid(platform_sys_sem_t *sem);
/**
 * Same as platform_sys_sem_valid() but taking a value, not a pointer
 */
#define platform_sys_sem_valid_val(sem)       platform_sys_sem_valid(&(sem))

/**
 * Same as platform_sys_sem_set_invalid() but taking a value, not a pointer
 */
#define platform_sys_sem_set_invalid_val(sem) platform_sys_sem_set_invalid(&(sem))

/**
 * @ingroup platform_sys_misc
 * Sleep for specified number of ms
 */
void platform_sys_msleep(uint32_t ms); /* only has a (close to) 1 ms resolution. */

/* Mailbox functions. */

/**
 * @ingroup platform_sys_mbox
 * Creates an empty mailbox for maximum "size" elements. Elements stored
 * in mailboxes are pointers. You have to define macros "_MBOX_SIZE"
 * in your lwipopts.h, or ignore this parameter in your implementation
 * and use a default size.
 * If the mailbox has been created, ERR_OK should be returned. Returning any
 * other error will provide a hint what went wrong, but except for assertions,
 * no real error handling is implemented.
 *
 * @param mbox pointer to the mbox to create
 * @param size (minimum) number of messages in this mbox
 * @return ERR_OK if successful, another platform_err_t otherwise
 */
platform_err_t platform_sys_mbox_new(platform_sys_mbox_t *mbox, int size);
/**
 * @ingroup platform_sys_mbox
 * Post a message to an mbox - may not fail
 * -> blocks if full, only to be used from tasks NOT from ISR!
 *
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
void platform_sys_mbox_post(platform_sys_mbox_t *mbox, void *msg);
/**
 * @ingroup platform_sys_mbox
 * Try to post a message to an mbox - may fail if full.
 * Can be used from ISR (if the sys arch layer allows this).
 * Returns ERR_MEM if it is full, else, ERR_OK if the "msg" is posted.
 *
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
platform_err_t platform_sys_mbox_trypost(platform_sys_mbox_t *mbox, void *msg);
/**
 * @ingroup platform_sys_mbox
 * Try to post a message to an mbox - may fail if full.
 * To be be used from ISR.
 * Returns ERR_MEM if it is full, else, ERR_OK if the "msg" is posted.
 *
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
platform_err_t platform_sys_mbox_trypost_fromisr(platform_sys_mbox_t *mbox, void *msg);
/**
 * @ingroup platform_sys_mbox
 * Blocks the thread until a message arrives in the mailbox, but does
 * not block the thread longer than "timeout" milliseconds (similar to
 * the platform_sys_sem_wait_timeout() function). If "timeout" is 0, the thread should
 * be blocked until a message arrives. The "msg" argument is a result
 * parameter that is set by the function (i.e., by doing "*msg =
 * ptr"). The "msg" parameter maybe NULL to indicate that the message
 * should be dropped.
 * The return values are the same as for the platform_sys_sem_wait_timeout() function:
 * PLATFORM_SYS_ARCH_TIMEOUT if there was a timeout, any other value if a messages
 * is received.
 *
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message (0 = wait forever)
 * @return PLATFORM_SYS_ARCH_TIMEOUT on timeout, any other value if a message has been received
 */
uint32_t platform_sys_arch_mbox_fetch(platform_sys_mbox_t *mbox, void **msg, uint32_t timeout);
/* Allow port to override with a macro, e.g. special timeout for platform_sys_arch_mbox_fetch() */
/**
 * @ingroup platform_sys_mbox
 * This is similar to platform_sys_arch_mbox_fetch, however if a message is not
 * present in the mailbox, it immediately returns with the code
 * PLATFORM_SYS_MBOX_EMPTY. On success 0 is returned.
 * To allow for efficient implementations, this can be defined as a
 * function-like macro in platform_sys_arch.h instead of a normal function. For
 * example, a naive implementation could be:
 * \#define platform_sys_arch_mbox_tryfetch(mbox,msg) platform_sys_arch_mbox_fetch(mbox,msg,1)
 * although this would introduce unnecessary delays.
 *
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @return 0 (milliseconds) if a message has been received
 *         or PLATFORM_SYS_MBOX_EMPTY if the mailbox is empty
 */
uint32_t platform_sys_arch_mbox_tryfetch(platform_sys_mbox_t *mbox, void **msg);
/**
 * For now, we map straight to platform_sys_arch implementation.
 */
#define platform_sys_mbox_tryfetch(mbox, msg) platform_sys_arch_mbox_tryfetch(mbox, msg)
/**
 * @ingroup platform_sys_mbox
 * Deallocates a mailbox. If there are messages still present in the
 * mailbox when the mailbox is deallocated, it is an indication of a
 * programming error.
 *
 * @param mbox mbox to delete
 */
void platform_sys_mbox_free(platform_sys_mbox_t *mbox);
#define platform_sys_mbox_fetch(mbox, msg) platform_sys_arch_mbox_fetch(mbox, msg, 0)

/**
 * @ingroup platform_sys_mbox
 * Returns 1 if the mailbox is valid, 0 if it is not valid.
 * When using pointers, a simple way is to check the pointer for != NULL.
 * When directly using OS structures, implementing this may be more complex.
 * This may also be a define, in which case the function is not prototyped.
 */
int platform_sys_mbox_valid(platform_sys_mbox_t *mbox);


/**
 * @ingroup platform_sys_mbox
 * Invalidate a mailbox so that platform_sys_mbox_valid() returns 0.
 * ATTENTION: This does NOT mean that the mailbox shall be deallocated:
 * platform_sys_mbox_free() is always called before calling this function!
 * This may also be a define, in which case the function is not prototyped.
 */
void platform_sys_mbox_set_invalid(platform_sys_mbox_t *mbox);

/**
 * Same as platform_sys_mbox_valid() but taking a value, not a pointer
 */
#define platform_sys_mbox_valid_val(mbox)       platform_sys_mbox_valid(&(mbox))

/**
 * Same as platform_sys_mbox_set_invalid() but taking a value, not a pointer
 */
#define platform_sys_mbox_set_invalid_val(mbox) platform_sys_mbox_set_invalid(&(mbox))


/**
 * @ingroup platform_sys_misc
 * The only thread function:
 * Starts a new thread named "name" with priority "prio" that will begin its
 * execution in the function "thread()". The "arg" argument will be passed as an
 * argument to the thread() function. The stack size to used for this thread is
 * the "stacksize" parameter. The id of the new thread is returned. Both the id
 * and the priority are system dependent.
 * ATTENTION: although this function returns a value, it MUST NOT FAIL (ports have to assert this!)
 *
 * @param name human-readable name for the thread (used for debugging purposes)
 * @param thread thread-function
 * @param arg parameter passed to 'thread'
 * @param stacksize stack size in bytes for the new thread (may be ignored by ports)
 * @param prio priority of the new thread (may be ignored by ports) */
platform_sys_thread_t platform_sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio);

#endif 

/**
 * @ingroup platform_sys_misc
 * platform_sys_init() must be called before anything else.
 * Initialize the platform_sys_arch layer.
 */
void platform_sys_init(void);

/**
 * Ticks/jiffies since power up.
 */
uint32_t platform_sys_jiffies(void);

/**
 * @ingroup platform_sys_time
 * Returns the current time in milliseconds,
 * may be the same as platform_sys_jiffies or at least based on it.
 * Don't care for wraparound, this is only used for time diffs.
 * Not implementing this function means you cannot use some modules (e.g. TCP
 * timestamps, internal timeouts for NO_SYS==1).
 */
uint32_t platform_sys_now(void);

/* Critical Region Protection */
/* These functions must be implemented in the platform_sys_arch.c file.
   In some implementations they can provide a more light-weight protection
   mechanism than using semaphores. Otherwise semaphores can be used for
   implementation */

/**
 * @ingroup platform_sys_prot
 * PLATFORM_SYS_ARCH_DECL_PROTECT
 * declare a protection variable. This macro will default to defining a variable of
 * type platform_sys_prot_t. If a particular port needs a different implementation, then
 * this macro may be defined in platform_sys_arch.h.
 */
#define PLATFORM_SYS_ARCH_DECL_PROTECT(lev) platform_sys_prot_t lev
/**
 * @ingroup platform_sys_prot
 * PLATFORM_SYS_ARCH_PROTECT
 * Perform a "fast" protect. This could be implemented by
 * disabling interrupts for an embedded system or by using a semaphore or
 * mutex. The implementation should allow calling PLATFORM_SYS_ARCH_PROTECT when
 * already protected. The old protection level is returned in the variable
 * "lev". This macro will default to calling the platform_sys_arch_protect() function
 * which should be implemented in platform_sys_arch.c. If a particular port needs a
 * different implementation, then this macro may be defined in platform_sys_arch.h
 */
#define PLATFORM_SYS_ARCH_PROTECT(lev) lev = platform_sys_arch_protect()
/**
 * @ingroup platform_sys_prot
 * PLATFORM_SYS_ARCH_UNPROTECT
 * Perform a "fast" set of the protection level to "lev". This could be
 * implemented by setting the interrupt level to "lev" within the MACRO or by
 * using a semaphore or mutex.  This macro will default to calling the
 * platform_sys_arch_unprotect() function which should be implemented in
 * platform_sys_arch.c. If a particular port needs a different implementation, then
 * this macro may be defined in platform_sys_arch.h
 */
#define PLATFORM_SYS_ARCH_UNPROTECT(lev) platform_sys_arch_unprotect(lev)
platform_sys_prot_t platform_sys_arch_protect(void);
void platform_sys_arch_unprotect(platform_sys_prot_t pval);


/*
 * Macros to set/get and increase/decrease variables in a thread-safe way.
 * Use these for accessing variable that are used from more than one thread.
 */

#define PLATFORM_SYS_ARCH_INC(var, val) do { \
                                PLATFORM_SYS_ARCH_DECL_PROTECT(old_level); \
                                PLATFORM_SYS_ARCH_PROTECT(old_level); \
                                var += val; \
                                PLATFORM_SYS_ARCH_UNPROTECT(old_level); \
                              } while(0)


#define PLATFORM_SYS_ARCH_DEC(var, val) do { \
                                PLATFORM_SYS_ARCH_DECL_PROTECT(old_level); \
                                PLATFORM_SYS_ARCH_PROTECT(old_level); \
                                var -= val; \
                                PLATFORM_SYS_ARCH_UNPROTECT(old_level); \
                              } while(0)

#define PLATFORM_SYS_ARCH_GET(var, ret) do { \
                                PLATFORM_SYS_ARCH_DECL_PROTECT(old_level); \
                                PLATFORM_SYS_ARCH_PROTECT(old_level); \
                                ret = var; \
                                PLATFORM_SYS_ARCH_UNPROTECT(old_level); \
                              } while(0)

#define PLATFORM_SYS_ARCH_SET(var, val) do { \
                                PLATFORM_SYS_ARCH_DECL_PROTECT(old_level); \
                                PLATFORM_SYS_ARCH_PROTECT(old_level); \
                                var = val; \
                                PLATFORM_SYS_ARCH_UNPROTECT(old_level); \
                              } while(0)

#define PLATFORM_SYS_ARCH_LOCKED(code) do { \
                                PLATFORM_SYS_ARCH_DECL_PROTECT(old_level); \
                                PLATFORM_SYS_ARCH_PROTECT(old_level); \
                                code; \
                                PLATFORM_SYS_ARCH_UNPROTECT(old_level); \
                              } while(0)

#endif /* PLATFORM_API_PLATFORM_PLATFORM_SYS_H_ */

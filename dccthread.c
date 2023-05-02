/* --------------------------------------- Includes --------------------------------------------------------- */
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include "dccthread.h"

/* --------------------------------------- Macros ----------------------------------------------------------- */
#define REG_RIP 16
#define TIMER_INTERVAL 10 // ms

// #define PRINT_DEBUG

/* --------------------------------------- Type Definitions ------------------------------------------------- */

struct dccthread_t
{
	char name[DCCTHREAD_MAX_NAME_SIZE];
	ucontext_t context;
	void (*func)(int);
	int param;
};

typedef enum {
	false,
	true
} bool;

enum {
	LAST_OP_RUN,
	LAST_OP_YIELD,
	LAST_OP_EXIT,
	LAST_OP_WAIT,
};

typedef struct thread_node_t thread_node_t;

struct thread_node_t
{
	dccthread_t * thread;
	struct thread_node_t *next;
	union info {
		dccthread_t * waiting_for;
	} line_info;
};

typedef struct
{
	int len;
 	thread_node_t *head;
 	thread_node_t *tail;
} thread_line_t;


/* --------------------------------------- Local Variables -------------------------------------------------- */

static dccthread_t manager_thread;
static dccthread_t *running_thread;

static thread_line_t ready_line;
static thread_line_t wait_line;

static int last_op;

static struct sigaction sa = {0};
static sigset_t aux_sigset;

timer_t preemption_tmr_id = 0;
struct itimerspec preemption_tmr_specs = {
	.it_value.tv_sec = 0,
	.it_value.tv_nsec = TIMER_INTERVAL * 10E6,
	.it_interval.tv_sec = 0,
	.it_interval.tv_nsec = TIMER_INTERVAL * 10E6
};

/* --------------------------------------- Function Declaration --------------------------------------------- */
static void setup_timer();

static void scheduler();
static void check_waiting_line(dccthread_t *tid);
static void timer_handler(int sig, siginfo_t *si, void *uc);

void dccthread_init(void (*func)(int), int param);
dccthread_t *dccthread_create(const char *name, void (*func)(int), int param);
void dccthread_yield(void);
void dccthread_exit(void);
void dccthread_wait(dccthread_t *tid);
void dccthread_sleep(struct timespec ts);
dccthread_t *dccthread_self(void);
const char *dccthread_name(dccthread_t *tid);

/* --------------------------------------- External Function Implementation ---------------------------------- */

#ifdef PRINT_DEBUG
void print_list(int lid, thread_line_t * list){
	printf("DCCTHREAD: list() - %d ", lid);
	thread_node_t * it = list->head;
	while(it != NULL){
		printf("%p -> ", it->thread);
		it = it->next;
	}
	printf("\n");
}
#endif

/* Part 1 */
void dccthread_init(void (*func)(int), int param)
{

	#ifdef PRINT_DEBUG
		printf("DCCTHREAD: init() - manager %p\n", &manager_thread);
	#endif

	/* Initialize Structures */
	memset(&ready_line, 0, sizeof(thread_line_t));
	memset(&wait_line, 0, sizeof(thread_line_t));

	/* Acquire manager thread */
	getcontext(&manager_thread.context);

	/* Create main thread */
	dccthread_t *main_thread = dccthread_create("main", func, param);

	/* Setup the timer */
	setup_timer();

	/* Run main thread */
	running_thread = main_thread;
	last_op = LAST_OP_RUN;
	swapcontext(&manager_thread.context, &main_thread->context);

	while (1)
	{
		running_thread = &manager_thread;
		scheduler();
	}
	exit(0);
}

/* Part 1 */
dccthread_t *dccthread_create(const char *name, void (*func)(int), int param)
{
	/* Create the thread */
	thread_node_t * new_node = (thread_node_t *) malloc(sizeof(thread_node_t));
	new_node->thread = (dccthread_t *) malloc(sizeof(dccthread_t));
	strcpy(new_node->thread->name, name);

	/* Initialize thread context */
	getcontext(&new_node->thread->context);
	new_node->thread->context.uc_link = &manager_thread.context;
	new_node->thread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
	new_node->thread->context.uc_stack.ss_size = THREAD_STACK_SIZE;
	new_node->thread->context.uc_stack.ss_flags = 0;

	makecontext(&new_node->thread->context, (void *)func, 1, param);

	/* Add thread to the end of threads list */
	if (ready_line.len > 0)
	{
		ready_line.tail->next = new_node;
		ready_line.tail = new_node;
	}
	else
	{
		ready_line.head = new_node;
		ready_line.tail = new_node;
	}
	ready_line.len++;

	#ifdef PRINT_DEBUG
		printf("DCCTHREAD: create() - created %p (%s)\n", (new_node->thread), name);
	#endif

	return new_node->thread;
}

/* Part 1 */
void dccthread_yield(void)
{
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &aux_sigset);
	sa.sa_mask = aux_sigset;

	/* Send executing thread to the end of the list */
	thread_node_t *curr = ready_line.head;
	ready_line.tail->next = curr;
	ready_line.tail = curr;
	ready_line.head = (curr->next) ? (curr->next) : curr;
	curr->next = NULL;


	#ifdef PRINT_DEBUG
		printf("DCCTHREAD: yield() - %p yielded \n", curr->thread);
	#endif

	/* Signal yield and go back to manager */
	last_op = LAST_OP_YIELD;
	running_thread = &manager_thread;
	swapcontext(&(curr->thread->context), &manager_thread.context);
}

/* Part 1 */
dccthread_t *dccthread_self(void)
{
	return running_thread;
}

/* Part 1 */
const char *dccthread_name(dccthread_t *tid)
{
	return tid->name;
}

/* Part 2 */
void dccthread_exit(void)
{
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &aux_sigset);
	sa.sa_mask = aux_sigset;

	#ifdef PRINT_DEBUG
		printf("DCCTHREAD: exit() - thread %p exited\n", running_thread);
	#endif

	last_op = LAST_OP_EXIT;
	running_thread = &manager_thread;
	swapcontext(&(ready_line.head->thread->context), &manager_thread.context);
}

/* Part 2 */
void dccthread_wait(dccthread_t *tid)
{
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &aux_sigset);
	sa.sa_mask = aux_sigset;

	#ifdef PRINT_DEBUG
		printf("DCCTHREAD: wait() - thread %p will wait for %p\n", running_thread, tid);
	#endif

	/* check if tid exists */
	int exist = 0;
	thread_node_t * it;
	if(ready_line.len > 1){
		it = ready_line.head;
		while(it != NULL){
			if(it->thread == tid){
				exist = 1;
				break;
			}
			it = it->next;
		}
	}
	if(!exist && (wait_line.len > 0)){
		it = wait_line.head;
		while(it != NULL){
			if(it->thread == tid){
				exist = 1;
				break;
			}
		it = it->next;
		}
	}
	if(!exist){
		return;
	}


	/* Remove from ready line */
	thread_node_t * curr = ready_line.head;
	ready_line.head = ready_line.head->next;
	ready_line.len--;

	/* Add to wait line */
	curr->line_info.waiting_for = tid;
	if (wait_line.len > 0)
	{
		wait_line.tail->next = curr;
		wait_line.tail = curr;
		wait_line.tail->next = NULL;
	}
	else
	{
		wait_line.head = curr;
		wait_line.tail = curr;
		wait_line.tail->next = NULL;
	}
	wait_line.len++;

	/* Signal wait and go back to manager */
	last_op = LAST_OP_WAIT;
	running_thread = &manager_thread;
	swapcontext(&(curr->thread->context), &manager_thread.context);

	return;
}

/* TODO - Part 5*/
void dccthread_sleep(struct timespec ts)
{
	return;
}

/* --------------------------------------- Internal Function Implementation ---------------------------------- */

static void scheduler()
{

	#ifdef PRINT_DEBUG
		print_list(1, &ready_line);
		print_list(2, &wait_line);
	#endif

	/* Running thread ended or exited */
	if((last_op == LAST_OP_RUN) || (last_op == LAST_OP_EXIT)){

		check_waiting_line(ready_line.head->thread);

		#ifdef PRINT_DEBUG
			printf("DCCTHREAD: scheduler() - thread %p ended or exited, %d left\n", ready_line.head->thread, ready_line.len-1);
		#endif

		thread_node_t *prev_head = ready_line.head;
		ready_line.head = ready_line.head->next;
		free(prev_head);
		if(--ready_line.len == 0){
			exit(1);
		}
	}

	#ifdef PRINT_DEBUG
		printf("DCCTHREAD: scheduler() - changing from %p to %p\n", running_thread, ready_line.head->thread);
	#endif

	sigprocmask(SIG_SETMASK, &sa.sa_mask, NULL);
	timer_settime(preemption_tmr_id, 0, &preemption_tmr_specs, NULL);

	/* Change the context to the next thread on the queue */
	last_op = LAST_OP_RUN;
	running_thread = ready_line.head->thread;
	swapcontext(&manager_thread.context, &running_thread->context);
}

static void check_waiting_line(dccthread_t *tid){
	if(wait_line.len == 0){
		return;
	}

	thread_node_t * it = wait_line.head;
	thread_node_t * prev = NULL;
	while(it != NULL){
		if(it->line_info.waiting_for == tid){
			break;
		}
		prev = it;
		it = it->next;
	}

	if(it != NULL){

		#ifdef PRINT_DEBUG
			printf("DCCTHREAD: check_waiting_line() - %p was waiting for %p\n", it->thread, tid);
		#endif

		/* Remove from wait line */
		if(wait_line.len == 1){ /* Only one in wait line */
			wait_line.head = NULL;
			wait_line.tail = NULL;
		}
		else {
			if(prev == NULL){ /* It is the head of the list */
				wait_line.head = it->next;
			}
			else{
				prev->next = it->next;
			}
		}
		wait_line.len--;

		it->line_info.waiting_for = NULL;

		ready_line.tail->next = it;
		ready_line.tail = it;
		ready_line.tail->next = NULL;
		ready_line.len++;
	}
}

static void timer_handler(int sig, siginfo_t *si, void *uc){
	#ifdef PRINT_DEBUG
		printf("DCCTHREAD: timer_handler()\n");
	#endif
	if((running_thread != &manager_thread) && (ready_line.len>=1)){
		dccthread_yield();
	}
}

static void setup_timer(){
	int res = 0;

    struct sigevent sev = {0};

    sev.sigev_notify = SIGEV_SIGNAL; // Linux-specific
    sev.sigev_signo = SIGRTMIN;

    /* create timer */
    res = timer_create(CLOCK_PROCESS_CPUTIME_ID, &sev, &preemption_tmr_id);

    if (res != 0)
    {
        printf("Error timer_create\n");
        exit(-1);
    }

    /* specifz signal and handler */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;

    /* Initialize signal */
    sigemptyset(&sa.sa_mask);

    /* Register signal handler */
    if (sigaction(SIGRTMIN, &sa, NULL) == -1)
    {
        printf("Error sigaction\n");
        exit(-1);
    }

    /* start timer */
    res = timer_settime(preemption_tmr_id, 0, &preemption_tmr_specs, NULL);

    if (res != 0)
    {
        printf("Error timer_settime\n");
        exit(-1);
    }
}

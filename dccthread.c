/* --------------------------------------- Includes --------------------------------------------------------- */
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dccthread.h"

/* --------------------------------------- Macros ----------------------------------------------------------- */
#define REG_RIP 16

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
};

typedef struct thread_node_t thread_node_t;

struct thread_node_t
{
	dccthread_t * thread;
	struct thread_node_t *next;
};

/* --------------------------------------- Local Variables -------------------------------------------------- */

static dccthread_t manager_thread;
static dccthread_t *running_thread;

static int num_of_threads = 0;
static thread_node_t *head_thread;
static thread_node_t *tail_thread;

static int last_op;

/* --------------------------------------- Function Declaration --------------------------------------------- */

static void scheduler();

void dccthread_init(void (*func)(int), int param);
dccthread_t *dccthread_create(const char *name, void (*func)(int), int param);
void dccthread_yield(void);
void dccthread_exit(void);
void dccthread_wait(dccthread_t *tid);
void dccthread_sleep(struct timespec ts);
dccthread_t *dccthread_self(void);
const char *dccthread_name(dccthread_t *tid);

/* --------------------------------------- External Function Implementation ---------------------------------- */

// Allows the file to be compiled and linked standalone. Can be taken out for the final version
// int main(int argc, char** argv) {
// 	return 0;
// }

#ifdef PRINT_DEBUG
void print_list(){
	printf("DCCTHREAD: list() - ");
	thread_node_t * it = head_thread;
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
	printf("DCCTHREAD: init() - Init, manager %p\n", &manager_thread);
#endif

	/* Acquire manager thread */
	getcontext(&manager_thread.context);

	/* Create main thread */
	dccthread_t *main_thread = dccthread_create("main", func, param);

#ifdef PRINT_DEBUG
	printf("DCCTHREAD: init() - main_thread %p\n", main_thread);
#endif

	/* Run main thread */
	running_thread = main_thread;
	last_op = LAST_OP_RUN;
	swapcontext(&manager_thread.context, &main_thread->context);

	while (1)
	{

#ifdef PRINT_DEBUG
		printf("DCCTHREAD: init() - back at while(1) (running %p), call scheduler\n", running_thread);
#endif

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
	if (num_of_threads > 0)
	{
		tail_thread->next = new_node;
		tail_thread = new_node;
	}
	else
	{
		head_thread = new_node;
		tail_thread = new_node;
	}
	num_of_threads++;

#ifdef PRINT_DEBUG
	printf("DCCTHREAD: create() - created %s (%p), running %p \n", name, (new_node->thread), running_thread);
#endif

	return new_node->thread;
}

/* TODO - Part 1 */
void dccthread_yield(void)
{
	/* Send executing thread to the end of the list */
	thread_node_t *curr = head_thread;
	tail_thread->next = curr;
	tail_thread = curr;
	head_thread = (curr->next) ? (curr->next) : curr;
	curr->next = NULL;


#ifdef PRINT_DEBUG
	printf("DCCTHREAD: yield() - %p yielded, moved to %p \n", curr->thread, tail_thread->thread);
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

/* TODO - Part 2 */
void dccthread_exit(void)
{
	return;
}

/* TODO - Part 2 */
void dccthread_wait(dccthread_t *tid)
{
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
	print_list();
#endif

	/* Running thread ended */
	if(last_op == LAST_OP_RUN){
#ifdef PRINT_DEBUG
	printf("DCCTHREAD: scheduler() - thread %p ended, %d left\n", running_thread, num_of_threads-1);
#endif
		thread_node_t *prev_head = head_thread;
		head_thread = head_thread->next;
		free(prev_head);
		if(--num_of_threads == 0){
			exit(1);
		}
	}

#ifdef PRINT_DEBUG
	printf("DCCTHREAD: scheduler() - changing from %p to %p\n", running_thread, head_thread->thread);
#endif

	/* Change the context to the next thread on the queue */
	last_op = LAST_OP_RUN;
	running_thread = head_thread->thread;
	swapcontext(&manager_thread.context, &running_thread->context);
}



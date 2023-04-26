/* --------------------------------------- Includes --------------------------------------------------------- */
#include <ucontext.h>
#include <stdlib.h>
#include <string.h>
#include "dccthread.h"

/* --------------------------------------- Macros ----------------------------------------------------------- */
#define REG_RIP 16

/* --------------------------------------- Type Definitions ------------------------------------------------- */

struct dccthread_t
{
	char name[DCCTHREAD_MAX_NAME_SIZE];
	ucontext_t context;
	void (*func)(int);
	int param;
};

typedef struct thread_node_t thread_node_t;

struct thread_node_t
{
	dccthread_t data;
	struct thread_node_t *next;
};

/* --------------------------------------- Local Variables -------------------------------------------------- */

static dccthread_t manager_thread;
static dccthread_t *running_thread;

static int num_of_threads = 0;
static int thread_ended = 1;
static thread_node_t *head_thread;
static thread_node_t *tail_thread;

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

/* Part 1 */
void dccthread_init(void (*func)(int), int param)
{
	/* Acquire manager thread */
	getcontext(&manager_thread.context);

	/* Create main thread */
	dccthread_t *main_thread = dccthread_create("main", func, param);

	/* Run main thread */
	running_thread = main_thread;
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
	thread_node_t *new_node = (thread_node_t *)malloc(sizeof(thread_node_t));
	dccthread_t *new_thread = &new_node->data;
	strcpy(new_thread->name, name);

	/* Initialize thread context */
	getcontext(&new_thread->context);
	new_thread->context.uc_link = &manager_thread.context;
	new_thread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
	new_thread->context.uc_stack.ss_size = THREAD_STACK_SIZE;
	new_thread->context.uc_stack.ss_flags = 0;

	makecontext(&new_thread->context, (void *)func, 1, param);

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
	return new_thread;
}

/* TODO - Part 1 */
void dccthread_yield(void)
{
	/* Send executing thread to the end of the list */
	thread_node_t *new_node = (thread_node_t *)malloc(sizeof(thread_node_t));
	tail_thread->next = new_node;
	new_node->data = *running_thread;
	new_node->next = NULL;
	tail_thread = new_node;

	/* Give control to the scheduler thread, indicating that the thread should be requeued */
	thread_ended = 0;
	swapcontext(&tail_thread->data.context, &manager_thread.context);
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
	/* Check if there are any threads left to execute */
	if (thread_ended == 1)
		num_of_threads--;
	if (num_of_threads == 0)
		exit(1);

	if (thread_ended == 1) {
		/* If a thread has ended, pop it */
		thread_node_t *prev_head = head_thread;
		head_thread = head_thread->next;
		free(prev_head);
	} else {
		/* If no thread ended, requeue the head thread */
		tail_thread->next = head_thread;
		head_thread = head_thread->next;
		tail_thread->next = NULL;
		thread_ended = 1;
	}

	/* Change the context to the next thread on the queue */
	running_thread = &head_thread->data;
	swapcontext(&manager_thread.context, &running_thread->context);
}

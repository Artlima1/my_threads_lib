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
	LAST_OP_EXIT,
};

typedef struct thread_node_t thread_node_t;

struct thread_node_t
{
	dccthread_t * thread;
	struct thread_node_t *next;
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

static thread_line_t ready_line = {0, NULL, NULL};

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
	thread_node_t * it = ready_line.head;
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
	printf("DCCTHREAD: create() - created %s (%p), running %p \n", name, (new_node->thread), running_thread);
#endif

	return new_node->thread;
}

/* TODO - Part 1 */
void dccthread_yield(void)
{
	/* Send executing thread to the end of the list */
	thread_node_t *curr = ready_line.head;
	ready_line.tail->next = curr;
	ready_line.tail = curr;
	ready_line.head = (curr->next) ? (curr->next) : curr;
	curr->next = NULL;


#ifdef PRINT_DEBUG
	printf("DCCTHREAD: yield() - %p yielded, moved to %p \n", curr->thread, ready_line.tail->thread);
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
	last_op = LAST_OP_EXIT;
	running_thread = &manager_thread;
	swapcontext(&(ready_line.head->thread->context), &manager_thread.context);
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

	/* Running thread ended or exited */
	if((last_op == LAST_OP_RUN) || (last_op == LAST_OP_EXIT)){
#ifdef PRINT_DEBUG
	printf("DCCTHREAD: scheduler() - thread %p ended, %d left\n", running_thread, ready_line.len-1);
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

	/* Change the context to the next thread on the queue */
	last_op = LAST_OP_RUN;
	running_thread = ready_line.head->thread;
	swapcontext(&manager_thread.context, &running_thread->context);
}



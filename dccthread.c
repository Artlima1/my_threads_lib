/* --------------------------------------- Includes --------------------------------------------------------- */
#include <ucontext.h>
#include <stdlib.h>
#include <string.h>
#include "dccthread.h"

/* --------------------------------------- Macros ----------------------------------------------------------- */

/* --------------------------------------- Type Definitions ------------------------------------------------- */

struct dccthread_t{
	char name[DCCTHREAD_MAX_NAME_SIZE];
	ucontext_t context;
	void (*func)(int);
	int param;
};

typedef struct thread_node_t thread_node_t;

struct thread_node_t {
	dccthread_t data;
	struct thread_node_t * next;
};

/* --------------------------------------- Local Variables -------------------------------------------------- */

static dccthread_t manager_thread;
static dccthread_t * running_thread;

static int num_of_threads = 0;
static thread_node_t * head_thread;
static thread_node_t * tail_thread;

/* --------------------------------------- Function Declaration --------------------------------------------- */

void scheduler(int param);

void dccthread_init(void (*func)(int), int param);
dccthread_t * dccthread_create(const char *name, void (*func)(int ), int param);
void dccthread_yield(void);
void dccthread_exit(void);
void dccthread_wait(dccthread_t *tid);
void dccthread_sleep(struct timespec ts);
dccthread_t * dccthread_self(void);
const char * dccthread_name(dccthread_t *tid);

/* --------------------------------------- External Function Implementation ---------------------------------- */

// Allows the file to be compiled and linked standalone. Can be taken out for the final version
// int main(int argc, char** argv) {
// 	return 0;
// }

/* TODO - Part 1 */
void dccthread_init(void (*func)(int), int param) {
	/* Acquire manager thread */
	getcontext(&manager_thread.context);
	
	/* Create main thread */
	dccthread_t * main_thread = dccthread_create("main", (void*)func, param);
	
	/* Run main thread */
	swapcontext(&manager_thread.context, &main_thread->context);
	exit(0);
}

/* TODO - Part 1 */
dccthread_t * dccthread_create(const char *name, void (*func)(int ), int param) {
	/* Create the thread */
	thread_node_t * new_node = (thread_node_t *) malloc(sizeof(thread_node_t));
	dccthread_t * new_thread = &new_node->data;
	strcpy(new_thread->name, name);

	/* Initialize thread context */
	getcontext(&new_thread->context);
	new_thread->context.uc_link = &manager_thread.context;
	new_thread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
	new_thread->context.uc_stack.ss_size = THREAD_STACK_SIZE;
	new_thread->context.uc_stack.ss_flags = 0;

	makecontext(&new_thread->context, (void*)func, 1, param);

	/* Add thread to the end of threads list */
	if(num_of_threads > 0) {
		tail_thread->next = new_node;
		tail_thread = new_node;
	} else {
		head_thread = new_node;
		tail_thread = new_node;
	}
	num_of_threads++;
	return new_thread;
}

/* TODO - Part 1 */
void dccthread_yield(void) {
	/* TODO - End current thread context */

	/* Remove current thread from threads list */
	thread_node_t * temp = head_thread;
	head_thread = head_thread->next;
	free(temp);
	num_of_threads--;

	/* Swap context with manager_thread */
	/* Set running_thread to manager */
}

/* Part 1 */
dccthread_t * dccthread_self(void) {
	return running_thread;
}

/* Part 1 */
const char * dccthread_name(dccthread_t *tid) {
	return tid->name;
}

/* TODO - Part 2 */
void dccthread_exit(void) {
	return;
}

/* TODO - Part 2 */
void dccthread_wait(dccthread_t *tid) {
	return;
}

/* TODO - Part 5*/
void dccthread_sleep(struct timespec ts) {
	return;
}

/* --------------------------------------- Internal Function Implementation ---------------------------------- */

void scheduler(int param){
	if(num_of_threads==0){
		exit(1);
	}

	/* swap context with head_thread (for now) */
	/* Set running thread to head_thread */
}

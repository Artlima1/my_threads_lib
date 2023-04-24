/* --------------------------------------- Includes --------------------------------------------------------- */
#include "dccthread.h"
#include <ucontext.h>

/* --------------------------------------- Macros ----------------------------------------------------------- */

/* --------------------------------------- Type Definitions ------------------------------------------------- */

typedef struct {
	char name[DCCTHREAD_MAX_NAME_SIZE];
	ucontext_t context;
	void (*func)(int);
	int param;
} dccthread_t;

typedef struct {
	dccthread_t data;
	struct thread_node_t * next;
} thread_node_t;

/* --------------------------------------- Local Variables -------------------------------------------------- */

static dccthread_t manager_thread;
static dccthread_t * running_thread;

static int num_of_threads = 0;
static thread_node_t * head_thread;
static thread_node_t * tail_thread;

/* --------------------------------------- Function Declaration --------------------------------------------- */

static void scheduler(int param);

void dccthread_init(void (*func)(int), int param);
dccthread_t * dccthread_create(const char *name, void (*func)(int ), int param);
void dccthread_yield(void);
void dccthread_exit(void);
void dccthread_wait(dccthread_t *tid);
void dccthread_sleep(struct timespec ts);
dccthread_t * dccthread_self(void);
const char * dccthread_name(dccthread_t *tid);

/* --------------------------------------- External Function Implementation ---------------------------------- */

/* TODO - Part 1 */
void dccthread_init(void (*func)(int), int param) {
	/* Create manager thread */
	/* Create main thread */
	/* Run main thread */
}

/* TODO - Part 1 */
dccthread_t * dccthread_create(const char *name, void (*func)(int ), int param) {
	/* Create the thread */
	thread_node_t * new_node = (thread_node_t *) malloc(sizeof(thread_node_t));
	dccthread_t * new_thread = &new_node->data;

	/* TODO - Initialize thread context */

	/* Add thread to the end of threads list */
	tail_thread->next = new_node;
	tail_thread = new_node;
	num_of_threads++;
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

static void scheduler(int param){
	if(num_of_threads==0){
		exit(1);
	}

	/* swap context with head_thread (for now) */
	/* Set running thread to head_thread */
}

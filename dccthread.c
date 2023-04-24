/* --------------------------------------- Includes --------------------------------------------------------- */
#include "dccthread.h"
#include <ucontext.h>

/* --------------------------------------- Macros ----------------------------------------------------------- */

#define MAX_THREADS 10
// #define USE_THREADS_LINKED_LIST
#define USE_THREADS_ARRAY

/* --------------------------------------- Type Definitions ------------------------------------------------- */

typedef struct {
	char name[DCCTHREAD_MAX_NAME_SIZE];
	ucontext_t context;
	void (*func)(int);
	int param;
} dccthread_t;

/* --------------------------------------- Local Variables -------------------------------------------------- */

static dccthread_t manager_thread;

#ifdef USE_THREADS_ARRAY
static dccthread_t threads_list[MAX_THREADS];
static int num_of_threads = 0;
/* I'm assuming the manager selects the next thread from the list every time, instead of sorting the list and picking the first */
static int current_thread_index;
#endif

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
	/* Add thread to the end of threads list */
	/* num_of_threads++ */
}

/* TODO - Part 1 */
void dccthread_yield(void) {
	/* End current thread context */
	/* Remove current thread from threads list, shifting left the rest of the list (if array) */
	/* num_of_threads-- */
	/* Swap context with manager_thread */
}

/* Part 1 */
dccthread_t * dccthread_self(void) {
	/* Return current_thread */
	return &threads_list[current_thread_index];
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

	/* swap context with threads_list[0] (for now) */
}

/* --------------------------------------- Includes --------------------------------------------------------- */
#include "dccthread.h"
#include <ucontext.h>

/* --------------------------------------- Macros ----------------------------------------------------------- */

/* --------------------------------------- Type Definitions ------------------------------------------------- */

typedef struct {
	char name[DCCTHREAD_MAX_NAME_SIZE];
	ucontext_t * context;
	void (*func)(int);
} dccthread_t;

/* --------------------------------------- Local Variables -------------------------------------------------- */

static dccthread_t manager;

/* --------------------------------------- Function Declaration --------------------------------------------- */

void dccthread_init(void (*func)(int), int param);
dccthread_t * dccthread_create(const char *name, void (*func)(int ), int param);
void dccthread_yield(void);
void dccthread_exit(void);
void dccthread_wait(dccthread_t *tid);
void dccthread_sleep(struct timespec ts);
dccthread_t * dccthread_self(void);
const char * dccthread_name(dccthread_t *tid);

/* --------------------------------------- Function Implementation ------------------------------------------ */

/* TODO - Part 1 */
void dccthread_init(void (*func)(int), int param) {

}

/* TODO - Part 1 */
dccthread_t * dccthread_create(const char *name, void (*func)(int ), int param) {

}

/* TODO - Part 1 */
void dccthread_yield(void) {
	return;
}

/* TODO - Part 1 */
dccthread_t * dccthread_self(void) {
	return;
}

/* TODO - Part 1 */
const char * dccthread_name(dccthread_t *tid) {
	return;
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

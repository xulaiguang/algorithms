#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

/* non thread-safe */

// 64KB, align 16byte
#define STACK_SIZE 64 * 1024 + 0x10

// 128 maximum co
#define CO_MAX 128

enum co_status {
	CO_NEW = 1,
	CO_RUNNING,
	CO_WAITING,
	CO_DEAD,
};

struct co {
	const char *name;
	void (*func)(void *);
	void *arg;

	enum co_status status;
	struct co *waiter;
	jmp_buf context;
	uint8_t stack[STACK_SIZE];

	int list_idx;
};

// special co for main()
struct co main_co = {
	.name = "main",
	.func = (void *)-1,
	.arg = NULL,
	.status = CO_RUNNING,
	.waiter = NULL,
	.list_idx = 0,
};

// current co
static struct co *current = &main_co;
// all co
static struct co *co_list[CO_MAX] = { &main_co, NULL };

struct co *co_start(const char *name, void (*func)(void *), void *arg)
{
	static int idx = 0;
	int i = (idx + 1) % CO_MAX;

	for (; i != idx; i = (i + 1) % CO_MAX) {
		if (co_list[i] == NULL) {
			break;
		}
	}

	if (i == idx && co_list[idx] != NULL)
		return NULL;

	idx = i;

	struct co *new = malloc(sizeof(struct co));
	if (!new) {
		return NULL;
	}

	new->name = name;
	new->func = func;
	new->arg = arg;

	new->status = CO_NEW;
	new->waiter = NULL;
	memset(new->stack, 'A', STACK_SIZE);
	new->list_idx = i;

	co_list[i] = new;

	return new;
}

void co_wait(struct co *co)
{
	assert(co->waiter == NULL);

	// waiting for co exiting
	if (co->status != CO_DEAD) {
		current->status = CO_WAITING;
		co->waiter = current;

		co_yield();
	}

	// schedule in
	// co is dead
	assert(co->status == CO_DEAD);

	co_list[co->list_idx] = NULL;
	free(co);
}

void co_wrapper(void *arg)
{
	struct co *run = arg;

	run->status = CO_RUNNING;
	run->func(run->arg);
	run->status = CO_DEAD;

	if (run->waiter) {
		run->waiter->status = CO_RUNNING;
	}

	// schedule out & never return
	co_yield();

	assert(1);
}

// pick next co
// status == CO_NEW or CO_RUNNING
struct co *co_next()
{
	static int idx = 0;
	for (int i = (idx + 1) % CO_MAX; i != idx; i = (i + 1) % CO_MAX) {
		if (!co_list[i])
			continue;
		if (co_list[i]->status == CO_NEW || co_list[i]->status == CO_RUNNING) {
			idx = i;
			return co_list[i];
		}
	}

	return current;
}

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg)
{
	asm volatile (
#if __x86_64__
			//"movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
			"movq %0, %%rsp; movq %2, %%rdi; call *%1"
			: : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
#else
			"movl %0, %%esp; movl %2, 4(%0); jmp *%1"
			: : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg) : "memory"
#endif
			);
}

void co_yield()
{
	int ret = setjmp(current->context);
	if (ret == 0) {
		// schedule out
		struct co *next = co_next();

		assert(next->status == CO_NEW || next->status == CO_RUNNING);
		current = next;
		if (next->status == CO_RUNNING) {
			longjmp(current->context, 1);
		} else {
			// align 16bytes
			void *stack = (void *)((long)(current->stack + STACK_SIZE) & (~0xf));
			assert(stack <= (void *)current->stack + STACK_SIZE);
			stack_switch_call(stack, co_wrapper, (uintptr_t)current);

			// should never return
			assert(1);
		}
	} else {
		// schedule in
		return;
	}
}

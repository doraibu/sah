#ifndef SAH_H
#define SAH_H

#ifdef __unix__

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define MEGABYTE (1024 * 1024)
#define STACK_TINY MEGABYTE
#define STACK_SMALL (2 * MEGABYTE)
#define STACK_MEDIUM (4 * MEGABYTE)
#define STACK_LARGE (8 * MEGABYTE)
#define STACK_HUGE (16 * MEGABYTE)

/* API */
struct _stack_header {
	size_t size;
};

/* STACK CONTROLLER STRUCTURE, ALLOCATED ON THE HEAP AND TRACKS THE SIZE OF THE STACK */
struct sah_stack {
	size_t payload_size;
	uint8_t* bp;
	uint8_t* sp;
}; __attribute__((aligned(64)));

/* BEGIN PUBLIC API FUNCTIONS SIGNATURE */
struct sah_stack* screate(size_t);
void sdestroy(struct sah_stack*);
void* spush(struct sah_stack*, size_t);
void spop(struct sah_stack*);
/* END PUBLIC API FUNCTIONS SIGNATURE */

/* BASIC MODULE STATIC FUNCTIONS */
/* FUNCTION push(), PUSHES INTO THE STACK, MANUAL CONTROL OF SIZE TO PUSH */
static inline void* push(struct sah_stack* s, size_t n)
{
	s->sp -= n;
	return s->sp;
}

/* FUNCTION pop(), POP THE STACK, MANUAL CONTROL OF SIZE TO POP */
static inline void pop(struct sah_stack* s, size_t n)
{
	s->sp += n;
}

/* FUNCTION sreset(), RESET THE STACK MOVING THE SP TO BP, ALLOWING REUSE */
static inline void sreset(struct sah_stack* s)
{
	s->sp = s->bp;
}

#ifdef SAH_IMPLEMENTATION

#define ALIGN(n) (((n) + 15) & ~15)

#ifdef BASIC

/* FUNCTION screate(), CREATES THE STACK AND RETURNS THE STACK CONTROLLER/HANDLER FOR USE */
struct sah_stack* screate(size_t psize)
{
	size_t guard = sysconf(_SC_PAGESIZE);
	size_t total = guard + psize;
	uint8_t* mem = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem == MAP_FAILED)
		return NULL;

	mprotect(mem, guard, PROT_NONE);

	struct sah_stack* s = malloc(sizeof(struct sah_stack));
	if (!s) {
		munmap(mem, total);
		return NULL;
	}

	s->payload_size = psize;
	s->bp = mem + total;
	s->sp = s->bp;

	return s;
}

/* FUNCTION sdestroy(), DESTROY THE STACK GIVEN A STACK CONTROLLER/HANDLER, FREE ALL MEMORY INCLUDING THE STACK CONTROLLER */
void sdestroy(struct sah_stack* s)
{
	if (s == NULL) return;

	size_t payload = s->payload_size;
	size_t guard = sysconf(_SC_PAGESIZE);
	size_t total = guard + payload;

	uint8_t* mem = s->bp - total;
	
	munmap(mem, total);
	free(s);
}

/* FUNCTION spush(), PUSH INTO THE STACK WITH HEADERS TO TRACK SIZE FOR SPOP */
void* spush(struct sah_stack* s, size_t n)
{
	size_t rtotal = sizeof(struct _stack_header) + n;
	size_t total = ALIGN(rtotal);
	
	s->sp -= total;

	struct _stack_header* hdr = (struct _stack_header*)s->sp;
	hdr->size = n;

	return (void*)(hdr + 1);
}

/* FUNCTION spop(), POP THE STACK, AUTO TRACKS THE AMOUNT TO POP WITH spush() HEADER */
void spop(struct sah_stack* s)
{
	struct _stack_header* hdr = (struct _stack_header*)s->sp;
	s->sp += ALIGN(sizeof(struct _stack_header) + hdr->size);
}

#endif /* BASIC MODULE */
#endif /* IMPLEMENTATION */

#endif /* unix */
#endif /* SAH_H */

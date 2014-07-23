
//
// GUMemory.h
//

// define C++ memory handling functions and macros to simplify the creation and validation of heap allocated memory.  GUMemory defines the gu_memAssert(v) macro to validate allocated memory.  If v is NULL then a message is logged to stdout and the host process is aborted.  The gu_memAlloc(v, n) and gu_memDeclare(t, v, n) macros encapsulate calls to malloc and gu_memAssert.  These macros should only be used if failure to allocate memory constitutes a critical / abortive application error.  gu_memDispose(v) validates, frees and sets v to NULL.  Memory tracking versions of malloc, calloc and free are also defined along with memory tracking overrides of new, new[], delete and delete[].  To override the standard malloc, calloc and free calls with the memory tracking versions __GU_DEBUG_MEMORY__ must be defined in the host application or framework.  GUMemory does not provide automatic garbage collection functionality, so memory must still be freed explicitly

#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>

#ifdef __GU_DEBUG_MEMORY__

// malloc, calloc and free overriding defines

#define	malloc		gu_malloc
#define	calloc		gu_calloc
#define	free		gu_free 


// override new and delete operators to track memory allocation

void *operator new(std::size_t size);
void operator delete(void *ptr);

void *operator new[](std::size_t size);
void operator delete[](void *ptr);

#endif


// Memory creation and validation macros

// gu_memAssert determines if v==NULL and if so logs an error message to the console (stdout) and aborts the host process
#define gu_memAssert(v)				if (!(v)) gu_memAssertFail(#v, __FUNCTION__)


// gu_memAlloc allocates n bytes from the heap (using malloc) which is assigned to the (already declared) pointer v.  v is then validated using gu_memAssert
#define gu_memAlloc(v, n)			(v) = (__typeof__(v))malloc((n)); gu_memAssert(v)


// gu_memDeclare declares a new pointer v of type t and allocates n bytes from the heap (using malloc) which is assigned to v.  v is then validated using gu_memAssert.  The type (t) should declare the required type and not a pointer to the required type since the pointer syntax is declared as part of the gu_memDeclare macro.  For example, a float pointer would be declared as gu_memDeclare(float, ptr, <size>) and not gu_memDeclare(float*, ptr, <size>) 
#define gu_memDeclare(t, v, n)		t* (v) = (t*)malloc((n)); gu_memAssert(v)


// gu_memDispose calls free on pointer v and sets v to NULL
#define gu_memDispose(v)			gu_memAssert(v); free((v)); (v) = NULL


// memory handling functions

void gu_memAssertFail(const char *ptrString, const char *fnString);


// memory tracking functions

void *gu_malloc(std::size_t memreq);
void *gu_calloc(std::size_t num, std::size_t size);
void gu_free(void *ptr);


// memory reporting functions

unsigned long gu_memory_allocations();
unsigned long gu_memory_deallocations();
unsigned long gu_memory_error();
void gu_memory_report();


// memory counter adjustment functions

void compensate_malloc_count(unsigned long c);
void compensate_free_count(unsigned long c);



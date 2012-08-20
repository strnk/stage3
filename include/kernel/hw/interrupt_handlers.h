#ifndef _HW_INTERRUPT_HANDLERS_H_
#define _HW_INTERRUPT_HANDLERS_H_

#include <kernel/hw/cpu.h>

typedef void 
(*exception_handler_t)(uint64_t exception, const struct cpu_context *context);
	
	
typedef void 
(*interrupt_handler_t)(int interrupt);

void
interrupt_init(void);

void
exception_init(void);

void
exception_set_handler(int exception, exception_handler_t handler);


void
default_exception_handler(uint64_t exception, const struct cpu_context *context);

#endif // _HW_INTERRUPT_HANDLERS_H_

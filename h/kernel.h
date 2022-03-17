#ifndef _kernel_setup_h_
#define _kernel_setup_h_

// makro za pripremu ulaza u IVT; (formira objekat klase IVTEntry)
#define PREPAREENTRY(entryNo, old_routine) \
void interrupt interrupt_routine##entryNo(...); \
IVTEntry new_entry##entryNo(entryNo, interrupt_routine##entryNo);\
void interrupt interrupt_routine##entryNo(...) { \
	new_entry##entryNo.signal(); \
	if (old_routine == 1) new_entry##entryNo.call_old_routine(); \
}

#include <iostream.h>
#include <dos.h>
#include "SharedF.h"
#include "SCHEDULE.h"
#include "pcb.h"
#include "../h/IVTEntry.h"

// deklaracija nove prekidne rutine
void interrupt timer();

// postavlja novu prekidnu rutinu
void init();

// vraca staru prekidnu rutinu
void restore();

/*volatile int cntr = 20;
volatile int context_switch_on_demand = 0;*/

// sinhrona promena konteksta
void dispatch();

void lock_flag();
void unlock_flag();

void childs_body();

#endif

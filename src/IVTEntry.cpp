#include "../h/IVTEntry.h"
#include <dos.h>
#include "../h/KernelEv.h"

#include <stdio.h>

//const unsigned char IVTEntry::TIMER_INTERRUPT_NEW_ROUTINE = 0x8;	// ako ce se ovo koristiti, ne sme biti static
//const unsigned char IVTEntry::TIMER_INTERRUPT_OLD_ROUTINE = 0x60;	// ako ce se ovo koristiti, ne sme biti static

HashMapIVTEntry *IVTEntry::allIVTEntries = new HashMapIVTEntry();
//HashMapIVTEntry *IVTEntry::allIVTEntries = 0;

// proveriti ovo
IVTEntry *IVTEntry::get_ptr_to_IVTEntry(unsigned char entryNo)
{
    return IVTEntry::allIVTEntries->get(entryNo);
}

IVTEntry::IVTEntry(unsigned char entryNo, pInterrupt newInterruptRoutine) : ENTRY_LIMIT(255)
{
	lock
    kernelEvent = 0;
    entryNumber = entryNo;

    if( entryNumber <= ENTRY_LIMIT)
    {
        oldRoutine = getvect(entryNo); // zapamtimo staru prekidnu rutinu
        setvect(entryNumber, newInterruptRoutine);
        if(oldRoutine == newInterruptRoutine) printf("FESTA!\n");
        // dodajemo u hash mapu svih IVTEntry : allIVTEntries
        if(IVTEntry::allIVTEntries == 0) IVTEntry::allIVTEntries = new HashMapIVTEntry(); // izbrisi
        IVTEntry::allIVTEntries->add(this, this->entryNumber);
    }
    else
    {
        oldRoutine = 0;
    }
   unlock
}

IVTEntry::~IVTEntry()
{
    if(oldRoutine != 0)
    {
        setvect(entryNumber, oldRoutine);
        IVTEntry::allIVTEntries->remove(this->entryNumber);
    }
}

void IVTEntry::call_old_routine() { if(oldRoutine != 0) oldRoutine(); }

void IVTEntry::signal()
{
	if(kernelEvent != 0)  kernelEvent->signal();
	else
	{
		lock
		if(oldRoutine != 0)
		{
		     setvect(entryNumber, oldRoutine);
		     IVTEntry::allIVTEntries->remove(this->entryNumber);
		}
		unlock
	}
}

void IVTEntry::set_kernel_event(KernelEv *event) { kernelEvent = event; }

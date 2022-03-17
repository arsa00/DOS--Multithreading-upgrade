#include "../h/KernelEv.h"
#include "../h/kernel.h"

#include <stdio.h>

KernelEv::KernelEv(unsigned char ivtNo)
{
    MyIVTEntry = IVTEntry::get_ptr_to_IVTEntry(ivtNo);
    owner = 0;
    val = 0;
    owner = (PCB*)SHARED_FILES::runningThread;
    isOwnerBlocked = 0;
    if(MyIVTEntry != 0)
    {
        // ako je ulaz u IVT-u podesen, onda pravimo event, u suprotnom GRESKA!
        MyIVTEntry->set_kernel_event(this);
    }
}

void KernelEv::wait()
{
	lock_flag();
    if(MyIVTEntry == 0 || owner != (PCB*)SHARED_FILES::runningThread)
    {
    	unlock_flag();
        return;
    }
    
    if(val == 0)
    {
        owner->isBlocked = 1;
        isOwnerBlocked = 1;
        dispatch();
    }
    else val = 0;
    unlock_flag();
}

void KernelEv::signal()
{
    lock_flag();
    if(isOwnerBlocked == 0) val = 1;
    else
    {
        isOwnerBlocked = 0;
        owner->isBlocked = 0;
        Scheduler::put(owner);
        dispatch();
    }

 /*  if(val == 0)
   {
	   val = 1;
	   if(isOwnerBlocked)
	   {
		   isOwnerBlocked = 0;
		   owner->isBlocked = 0;
		   Scheduler::put(owner);
		   dispatch();
	   }
   }*/
    unlock_flag();
}

KernelEv::~KernelEv()
{
	signal();
	lock
	if(MyIVTEntry != 0)
	{
	    // ako je ulaz u IVT-u podesen, onda pravimo event, u suprotnom GRESKA!
	    MyIVTEntry->set_kernel_event(0);
	}
   // delete MyIVTEntry;
	unlock
}

#include "../h/KernSem.h"
#include "../h/SharedF.h"
#include "../h/kernel.h"
#include "../h/ListSEM.h"

#include <stdio.h>

int KernelSem::ID = 0;

KernelSem::KernelSem(int init_val) : val(init_val)
{
	id = ID++;
    blocked_all = new List<SemElement>();
    blocked_limited = new List<SemElement>();

    // dodaj u globalnu promeljiu allSemaphores
	if(SHARED_FILES::allSemaphores == 0) SHARED_FILES::allSemaphores = new ListSem();

    SHARED_FILES::allSemaphores->add(this);
}

int KernelSem::wait(unsigned int TimeToWait)
{
    /* wait(unsigned int TimeToWait) vraca: 
            ->  1  : u slucaju da je nit deblokirana metodom signal semafora
            ->  0  : u slucaju da je nit deblokirana zbog isteka vremena cekanja ili brisanja semafora
            -> -1  : u slucaju da se nit uopste ne blokira
    */
	// lock_
    lock_flag();
    int ret;
    if(--val < 0) ret = block(TimeToWait);
    else
    {
    	ret = -1;
    	unlock_flag();
    }
    // unlock_
    return ret;
}

void KernelSem::signal()
{
    lock_flag();
    if(++val <= 0) unblock();
    unlock_flag();
}

int KernelSem::block(unsigned int TimeSlice)
{
    SHARED_FILES::runningThread->isBlocked = 1;
    SemElement *new_el = new SemElement();
    new_el->pcb = (PCB *)SHARED_FILES::runningThread;
    new_el->TimeSlice = TimeSlice;

    blocked_all->add(new_el);
    if(TimeSlice > 0) blocked_limited->add(new_el);

    dispatch();
    unlock_flag();
    return new_el->unblocked_by_signal;
}

void KernelSem::unblock()
{
    if(blocked_all->getSize() > 0)
    {
        List<SemElement>::Iterator iter = blocked_all->getIter();
        SemElement* to_unblock = iter.next();   // uzmemo prvu nit iz reda cekanja (FIFO)
        to_unblock->pcb->isBlocked = 0;     // postavimo da vise nije blokirana
        to_unblock->unblocked_by_signal = 1;    // napomenemo da je ova nit odblokirana pomocu signal metode semafora
        to_unblock->PassedTimeSlices = 0;

        // i vratimo tu nit u red spremnih (u Scheduler)
        if(to_unblock->pcb != SHARED_FILES::idleThread && !to_unblock->pcb->isFinished)
            Scheduler::put(to_unblock->pcb);

        // na kraju ostaje da se azuriraje dve liste blokiranih niti
        if(to_unblock->TimeSlice > 0) blocked_limited->remove(to_unblock);  // ako ima tog elementa u listi blokiranih niti sa vremenom cekanja, izbacimo ga
        blocked_all->remove(to_unblock);
    }
}

void KernelSem::tickSem()
{
		if(blocked_limited->getSize() > 0)
		{
			// ukoliko ima blokiranih niti sa vremenom cekanja, inkrementiramo ih sve i deblokiramo ako je neka zavrsila vreme cekanja
			List<SemElement>::Iterator iter = blocked_limited->getIter();
			while(iter.hasNext())
			{
				SemElement *ptr = iter.next();
				ptr->PassedTimeSlices++;
				if(ptr->PassedTimeSlices == ptr->TimeSlice)
				{
					// deblokiramo nit koja je zavrsila svoje vreme cekanja
					ptr->pcb->isBlocked = 0;
					ptr->PassedTimeSlices = 0;
					ptr->unblocked_by_signal = 0;   // napomenemo da ova nit nije odblokirana pomocu signal metode semafora

					// i vratimo tu nit u red spremnih (u Scheduler)
					if(ptr->pcb != SHARED_FILES::idleThread && !ptr->pcb->isFinished)
						Scheduler::put(ptr->pcb);

					// azuriramo liste blokiranih niti
					blocked_limited->remove(ptr);
					blocked_all->remove(ptr);
				}
			}
		}
}

KernelSem::~KernelSem()
{
	//lock
    // ukoliko su neke niti cekale na semafor koji se brise, oslobodimo ih
	if(blocked_all->getSize() > 0)
	{
		List<SemElement>::Iterator iter = blocked_all->getIter();
		while(iter.hasNext())
		{
			SemElement *ptr = iter.next();
			ptr->pcb->isBlocked = 0;
			ptr->PassedTimeSlices = 0;
			ptr->unblocked_by_signal = 0;   // napomenemo da ova nit nije odblokirana pomocu signal metode semafora

			// i vratimo tu nit u red spremnih (u Scheduler)
		    if(ptr->pcb != SHARED_FILES::idleThread && !ptr->pcb->isFinished)
				Scheduler::put(ptr->pcb);

		    if(ptr->TimeSlice > 0) blocked_limited->remove(ptr);  // ako ima tog elementa u listi blokiranih niti sa vremenom cekanja, izbacimo ga
		    blocked_all->remove(ptr);
		}
	}

    // nakon toga izbacimo semafor iz globalne promeljive allSemaphores
    SHARED_FILES::allSemaphores->remove(this);

    // i na kraju dealociramo sve dinamicke promenljive

    delete blocked_all;
    delete blocked_limited;
   // unlock
    blocked_all = 0;
    blocked_limited = 0;
}

#include "../h/kernel.h"
#include "../h/thread.h"
#include "../h/KernSem.h"
#include "../h/ListSEM.h"
#include <stdio.h>

extern void tick();

// stara prekidna rutina
unsigned oldTimerOFF, oldTimerSEG;

void init()
{
	asm{
		cli
		push es
		push ax
		
		mov ax,0
		mov es,ax // es = 0
		
		// pamti staru rutinu
		mov ax, word ptr es:0022h
		mov word ptr oldTimerSEG, ax
		mov ax, word ptr es:0020h
		mov word ptr oldTimerOFF, ax
		
		// postavlja novu rutinu
		mov word ptr es:0022h, seg timer
		mov word ptr es:0020h, offset timer
		
		// postavlja staru rutinu na int 60h
		mov ax, oldTimerSEG
		mov word ptr es:0182h, ax
		mov ax, oldTimerOFF
		mov word ptr es:0180h, ax

		pop ax
		pop es
		sti
	}
}

// vraca staru prekidnu rutinu
void restore()
{
	asm {
		cli
		push es
		push ax

		mov ax,0
		mov es,ax

		mov ax, word ptr oldTimerSEG
		mov word ptr es:0022h, ax
		mov ax, word ptr oldTimerOFF
		mov word ptr es:0020h, ax

		pop ax
		pop es
		sti
	}
}

//pomocne promenljive za prekid tajmera
unsigned tsp;
unsigned tss;
unsigned tbp;

// nova prekidna rutina tajmera
void interrupt timer()
{
	if(!SHARED_FILES::context_switch_on_demand)
	{
		ListSem::Iterator sem_iter = SHARED_FILES::allSemaphores->getIter();
		while(sem_iter.hasNext())
		{
			KernelSem *cur = sem_iter.next();
			cur->tickSem();
		}
	}

	if (SHARED_FILES::runningThread && !SHARED_FILES::context_switch_on_demand && !SHARED_FILES::runningThread->isUnlimited) SHARED_FILES::timer_counter--;
	if (SHARED_FILES::runningThread && ((SHARED_FILES::timer_counter == 0 && !SHARED_FILES::runningThread->isUnlimited) || SHARED_FILES::context_switch_on_demand))
    {
		if(SHARED_FILES::lockFlag <= 0)
		{
			SHARED_FILES::context_switch_on_demand = 0;
			// treba dodati lock flag-ove
			asm {
				// cuva sp
				mov tsp, sp
				mov tss, ss
				mov tbp, bp
			}

			SHARED_FILES::runningThread->sp = tsp;
			SHARED_FILES::runningThread->ss = tss;
			SHARED_FILES::runningThread->bp = tbp; //izmena

			if(!SHARED_FILES::runningThread->isFinished && !SHARED_FILES::runningThread->isBlocked && SHARED_FILES::runningThread != SHARED_FILES::idleThread && !SHARED_FILES::runningThread->Error)
				Scheduler::put((PCB *) SHARED_FILES::runningThread);


	/*		if(SHARED_FILES::runningThread != 0 && !SHARED_FILES::runningThread->isFinished && !SHARED_FILES::runningThread->isBlocked && SHARED_FILES::runningThread != SHARED_FILES::idleThread && !SHARED_FILES::runningThread->Error)
				Scheduler::put((PCB *) SHARED_FILES::runningThread);
*/
			if(SHARED_FILES::runningThread == 0 || SHARED_FILES::runningThread->Error == 1)
				SHARED_FILES::error_num--;

			//printf("\nPROMENA SA %d\n", SHARED_FILES::runningThread->id);

			SHARED_FILES::runningThread = Scheduler::get();
			if(SHARED_FILES::runningThread == 0)
				SHARED_FILES::runningThread = SHARED_FILES::idleThread;

			//printf("\nPROMENA NA %d\n", SHARED_FILES::runningThread->id);

			tsp = SHARED_FILES::runningThread->sp;
			tss = SHARED_FILES::runningThread->ss;
			tbp = SHARED_FILES::runningThread->bp;

			SHARED_FILES::timer_counter = SHARED_FILES::runningThread->timeSlice;
			asm {
				// restaurira sp
				mov sp, tsp
				mov ss, tss
				mov bp, tbp  //izmena
			}
		}
		else
			SHARED_FILES::context_switch_on_demand = 1;
	} 

	// poziv stare prekidne rutine 
	// koja se nalazila na 08h, a sad je na 60h;
	// poziva se samo kada nije zahtevana promena konteksta
	// tako da se stara rutina poziva 
	// samo kada je stvarno doslo do prekida
	if(!SHARED_FILES::context_switch_on_demand)
	{
		tick();
		asm int 60h;
	}
}

// sinhrona promena konteksta
void dispatch()
{ 
	lock
	SHARED_FILES::context_switch_on_demand = 1;
	timer();
	unlock
}

void lock_flag()
{
    SHARED_FILES::lockFlag++;
}

void unlock_flag()
{
	SHARED_FILES::lockFlag--;
    if(SHARED_FILES::context_switch_on_demand && !SHARED_FILES::lockFlag)
    {
        dispatch();
    }
}

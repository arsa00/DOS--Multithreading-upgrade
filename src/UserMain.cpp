#include "../h/kernel.h"
#include "../h/semaphor.h"
#include <stdio.h>
#include <stdlib.h>
#include "../h/ListSEM.h"
#include "../h/event.h"


#include <DOS.H>
#include <STDIO.H>
#include <STDARG.H>

#define false 0
#define true 1
#define nullptr 0

int syncPrint(const char *format, ...)
{
	int res;
	va_list args;
	lock_flag();
		va_start(args, format);
	res = vprintf(format, args);
	va_end(args);
	unlock_flag();
		return res;
}


void dumbSleep(int delay) {
      for (int i = 0; i < 1000; ++i) {
          for (int j = 0; j < delay; ++j);
      }
  }
volatile int failedFork = false;









/*
class TickTock
{
public:
	void tick();
	void tock();
	TickTock() : tickSem(2), tockSem(0) {}
private:
	Semaphore tickSem, tockSem;
};

void TickTock::tick()
{
	if(tickSem.val() == 0) tockSem.signal();
	tickSem.wait(0);
	syncPrint("tick ");
}

void TickTock::tock()
{
	tockSem.wait(0);
	syncPrint("tock ");
	tickSem.signal();
	tickSem.signal();
}

TickTock tickTock;

class Tick : public Thread
{
public:
	void run()
	{
		for(int j = 0; j < 100; j++) tickTock.tick();
	}
};

class Tock : public Thread
{
public:
	void run()
	{
		for(int i = 0; i < 100; i++) tickTock.tock();
	}
};


int userMain(int argc, char *argv[])
{
	Tick ti;
	Tock to;
	to.start();
	ti.start();

	ti.waitToComplete();
	return 0;
}

void tick() { }
*/

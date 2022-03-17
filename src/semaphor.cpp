#include "../h/semaphor.h"
#include "../h/KernSem.h"
#include "../h/SharedF.h"
#include "../h/kernel.h"
#include <stdio.h>

Semaphore::Semaphore(int init)
{
	lock_flag();
	myImpl = new KernelSem(init);
	unlock_flag();
}

Semaphore::~Semaphore()
{
    lock_flag();
    delete myImpl;
    unlock_flag();
}

int Semaphore::wait(Time maxTimeToWait)
{
    return myImpl->wait(maxTimeToWait);
}

void Semaphore::signal()
{
    myImpl->signal();
}

int Semaphore::val() const
{
    return myImpl->getValue();
}


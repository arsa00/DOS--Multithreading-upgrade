#include "../h/kernel.h"
#include "../h/semaphor.h"
#include <stdio.h>
#include <stdlib.h>
#include "../h/ListSEM.h"
#include "../h/event.h"



extern int userMain(int argc, char* argv[]);
// mozda safe guard-ove dodati
class userMainThread : public Thread
{
public:
	userMainThread(int argc, char **argv) : Thread(), argc(argc), argv(argv), ret(-1) {}
    ~userMainThread() { waitToComplete(); }
    void run();
    Thread* clone() const;
    int returnValue() const { return ret; }
private:
    int argc;
    char **argv;
    int ret;
};

void userMainThread::run()
{
    ret = userMain(argc, argv);
}

Thread* userMainThread::clone() const
{
	userMainThread *copy = new userMainThread(this->argc, this->argv);
	copy->ret = this->ret;
	return copy;
}


int main(int argc, char** argv)
{
    init();

    PCB::PCB_main_init();

    userMainThread user_main(argc, argv);
    user_main.start();
    user_main.waitToComplete();
    int ret = user_main.returnValue();

    //userMain(argc, argv);

    restore();

    return ret;
}


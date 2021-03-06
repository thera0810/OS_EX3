// threadtest.cc 
//  Simple test case for the threads assignment.
//
//  Create two threads, and have them context switch
//  back and forth between themselves by calling Thread::Yield, 
//  to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include <stdlib.h>
#include <time.h>
#include "copyright.h"
#include "system.h"
#include "dllist.h"
#include <stdio.h>
#include "synch.h"
#include "Table.h"
#include "BoundedBuffer.h"
#include "EventBarrier.h"
#include "Elevator.h"
#include <ctime>


#include "sysdep.h"
#include "Alarm.h"


// testnum is set in main.cc
int testnum = 1;
int threadnum=2;
int N=10;
int floors=30;
int elevs=1;
int capacity=7;
char threadname[10][20]={{0}};
char threadname_[10][20]={{0}};
DLList *ls=new DLList();

int tablesize = 3;
Table* tb = new Table(tablesize);
BoundedBuffer * bf=new BoundedBuffer(10);

//EventBarrier
EventBarrier* eb=new EventBarrier();
Building *building;
Elevator *elevators;
//Alarm* alarms=new Alarm();
//---------------------------ThreadTest1---------------------------

//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread 
//  each iteration.
//
//  "which" is simply a number identifying the thread, for debugging
//  purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
   }
}

//----------------------------------------------------------------------
// ThreadTest1
//  Set up a ping-pong between two threads, by forking a thread 
//  to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);

}

//---------------------------ThreadTest2 dllist---------------------------

void SimpleThread20(int which)
{
    ls->SortedInsert(NULL,7);
    ls->Show();
}

void SimpleThread21(int which)
{
    ls->SortedRemove(7);
    ls->Show();
}

void ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");

    dllFunc1(ls, N);
    for (int i=0;i<threadnum;++i)
    {
        sprintf(threadname[i],"%d",i);
        Thread *t = new Thread(threadname[i]);
        if (i==0)
        {
            t->Fork(SimpleThread20, 1);
        }
        else if (i==1)
        {
            t->Fork(SimpleThread21, 1);
        }
    }
}


//---------------------------ThreadTest3 dllist---------------------------

void SimpleThreadFunc3(int n)
{
    ls->Append(NULL);
    ls->Show();
}
void ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest3");
    dllFunc1(ls, 3);
    for (int i = 0; i < threadnum; ++i)
    {
        sprintf(threadname[i], "%d", i);
        Thread* t = new Thread(threadname[i]);
        t->Fork(SimpleThreadFunc3, i);
    }
}

//---------------------------ThreadTest4 dllist---------------------------

void SimpleThreadFunc4(int n)
{
    ls->SortedInsert(NULL, n);
    ls->Show();
}

void ThreadTest4()
{
    DEBUG('t', "Entering ThreadTest4");
    ls->SortedInsert(NULL, 5);
    ls->SortedInsert(NULL, 15);
    ls->Show();
    for (int i = 0; i < threadnum; ++i)
    {
        sprintf(threadname[i], "%d", i);
        Thread* t = new Thread(threadname[i]);
        t->Fork(SimpleThreadFunc4, i+10);
    }
}

//---------------------------ThreadTest 5 Table---------------------------

void TableThreadFunc0(int n) // allocate 5 item continueously
{
    int* object = new int();
    *object = n;
    for(int i=0;i<5;++i)
       tb->Alloc(object);
}

void TableThreadFunc1(int n) // release from 0 to tablesize one by one
{
    int i;
    for (i = 0; i < tablesize; i++) {
        int j = i%3;
        tb->Release(j);
        currentThread->Yield();
    }
}

void TableThreadTest5()
{
    DEBUG('t', "Entering TableThreadTest5");
    for (int i = 0; i < threadnum; ++i)
    {
        sprintf(threadname[i], "%d", i);
        Thread* t = new Thread(threadname[i]);
        if (i == 1) t->Fork(TableThreadFunc1, i);
        else t->Fork(TableThreadFunc0, i);
    }
}

//---------------------------ThreadTest 6 Table---------------------------

void TableThreadTest6()
{
    DEBUG('t', "Entering TableThreadTest6");
    for (int i = 0; i < threadnum; ++i)
    {
        sprintf(threadname[i], "%d", i);
        Thread* t = new Thread(threadname[i]);
        if(i==0) t->Fork(TableThreadFunc0, i);
        else t->Fork(TableThreadFunc1, i);
    }
}


//---------------------------ThreadTest7 Bounded Buffer---------------------------

void Consumer(int n)
{
    //int a[3]={2,0,0};
    void *data=new char[10];
    for(int i=0;i<n;++i){
        int t=rand()%10;
        //int t=a[i];
        bf->Read(data,t);
        printf("%s:%d\n",currentThread->getName(),t);
        bf->ShowState();
    }
}
void Producer(int n)
{
    //int a[3]={3,3,5};
    void *data=new char[10];
    for(int i=0;i<n;++i){
        int t=rand()%10;
        //int t=a[i];
        bf->Write(data,t);
        printf("%s:%d\n",currentThread->getName(),t);
        bf->ShowState();
    }
}
void ThreadTest7()
{
    srand(time(NULL));
    DEBUG('t', "Entering ThreadTest7\n");
    bf->ShowState();
    for (int i = 0; i < threadnum; ++i)
    {
        if(i%2==0){
            sprintf(threadname[i], "Producer%d", i/2);
            Thread* t = new Thread(threadname[i]);
            t->Fork(Producer, 3);
        }
        else{
            sprintf(threadname[i], "Consumer%d", i/2);
            Thread* t = new Thread(threadname[i]);
            t->Fork(Consumer, 3);
        }
    }
}


//--------------------------- ThreadTest 8 EventBarrier---------------------------

void SimpleThreadFunc8_wait(int n)
{
    eb->Wait();
    eb->Complete();
}
void SimpleThreadFunc8_signal(int n)
{
    while(eb->Waiters()<(n<threadnum?n:threadnum))
        currentThread->Yield();
    eb->Signal();
}
void ThreadTest8()
{
    DEBUG('t', "Entering ThreadTest8");
    int i=0;
    for (i = 0; i < threadnum; ++i)
    {
        sprintf(threadname[i], "%d", i);
        Thread* t = new Thread(threadname[i]);
        t->Fork(SimpleThreadFunc8_wait,i);
    }
    SimpleThreadFunc8_signal(i);
}




//--------------------------- ThreadTest 9 Alarm ---------------------------

void AlarmThreadFunc1(int n)
{
        int when = (n*500)%149;
        alarms->Pause(when);
        printf("Thread%s has been awakened.\n\n", currentThread->getName());
        //DEBUG('a',"Thread%s has been awakened.\n\n", currentThread->getName());
}

void AlarmCheckThreadFunc(int n) // a SNEAKY check thread
{
    int i;
    for (i = 0; i < 500; i++) 
        currentThread->Yield();
}

void AlarmThreadTest9()
{
    DEBUG('t', "Entering AlarmThreadTest9");
    for (int i = 0; i < threadnum; ++i)
    {
        sprintf(threadname[i], "%d", i);
        Thread* t = new Thread(threadname[i]);
        t->Fork(AlarmThreadFunc1, i);
    }
    AlarmCheckThreadFunc(0);
}


//--------------------------- ThreadTest 10 Elevator ---------------------------
//单电梯测试 1
int f1[5]={3,1,3,5,3};
int f2[5]={5,5,4,2,1};

// //单电梯超载 2
// int f1[5]={3,2,4,6};
// int f2[5]={5,6,6,2};

// //多电梯 3
// int f1[5]={12,3,7,12};
// int f2[5]={5,5,4,6};

void ElevatorThreadFunc(int id)  //Elevator Thread
{
    elevators[id].Operating();
}

void RiderThreadFunc(int id)                //Rider Threads
{
    Elevator *e;

    int srcFloor,dstFloor,tmp=0;

    tmp=1;

    if(tmp==1){
        srcFloor =f1[id];
        dstFloor =f2[id];
    }
    else
    {
        srcFloor = (rand()%floors)+1;
        do{
            dstFloor = (rand()%floors)+1;
        }while(dstFloor==srcFloor);
    }

    DEBUG('E',"\033[1;33;40mRider %d travelling from %d to %d\033[m\n",id,srcFloor,dstFloor);
    if (srcFloor == dstFloor){
        DEBUG('E',"\033[1;33;40mWorn rider %d do not push elevator buttom for fun\033[m\n\n",id);
        return;
    }
    do {
        if (srcFloor < dstFloor) {
            DEBUG('E',"\033[1;33;40mRider %d CallUp(%d)\033[m\n",id, srcFloor);
            building->CallUp(srcFloor);
            DEBUG('E',"\033[1;33;40mRider %d AwaitUp(%d)\033[m\n\n",id, srcFloor);
            e = building->AwaitUp(srcFloor);
        } else {
            DEBUG('E',"\033[1;33;40mRider %d CallDown(%d)\033[m\n",id, srcFloor);
            building->CallDown(srcFloor);
            DEBUG('E',"\033[1;33;40mRider %d AwaitDown(%d)\033[m\n\n",id, srcFloor);
            e = building->AwaitDown(srcFloor);
        }
    } while (!e->Enter()); // elevator might be full!
    e->RequestFloor(dstFloor); // doesn't return until arrival
    e->Exit();
}


void ElevatorTest10()
{
    DEBUG('t', "Entering ElevatorTest10\n");

    srand(time(NULL));
    int ele=threadnum-1;
    //ele=0;
    ele=ele<threadnum-1?ele:threadnum-1;//change elevator's order here
    
    building = new Building("Building",floors,elevs);
    elevators = building->GetElevator();

    for (int i = 0; i < threadnum; ++i)
    {
        sprintf(threadname[i], "%d", i);
        Thread* t = new Thread(threadname[i]);
        t->Fork(RiderThreadFunc, i);
    }

    for (int i = 0; i < elevs; ++i)
    {
        sprintf(threadname_[i], "elevator%d", i);
        Thread* t = new Thread(threadname_[i]);
        t->Fork(ElevatorThreadFunc, i);
    }
}

//--------------------------- ThreadTest 10 Elevator ---------------------------


//----------------------------------------------------------------------
// ThreadTest
//  Invoke a test routine.
//----------------------------------------------------------------------
void
ThreadTest()
{
    printf("|%*s|\n",75," ");
    printf("|%*s\033[1;32;40mtestnum    = %-2d\033[m%*s|\n",30," ",testnum,30," ");
    printf("|%*s\033[1;32;40mthreadnum  = %-2d\033[m%*s|\n",30," ",threadnum,30," ");
    printf("|%*s\033[1;32;40melementnum = %-2d\033[m%*s|\n",30," ",N,30," ");
    printf("+---------------------------------------------------------------------------+\n");

    switch (testnum) {
    case 1:
    {
        ThreadTest1();
        break;
    }

    case 2://xht: link break 70
    {
        ThreadTest2();
        break;
    }

    case 3://overwrite(pointer error) lxh 81
    {
        // ./nachos -q 81 -T 2  
        ThreadTest3();
        break;
    }

    case 4:// insert disorder lxh 60
    {
        // ./nachos -q 60 -T 2  
        ThreadTest4();
        break;
    }

    case 5:// Table Threadtest
    {
        // ./nachos -d b -q 5 -T 3  
        TableThreadTest5();
        break;
    }

    case 6:// Table Threadtest
    {
        // ./nachos -d b -q 6 -T 2  
        TableThreadTest6();
        break;
    }

    case 7://test boundedbuffer
    {
        ThreadTest7();
        break;
    }

    case 8://test eventbarrier
    {
        ThreadTest8();
        break;
    }

    case 9://test alarm -d a
    {
        AlarmThreadTest9();
        break;
    }

    case 10://test elevator
    {
        //./nachos -d E -q 10 -F 9 -T 5 -V 3
        ElevatorTest10();//-T rider number; -V elevator number.
        break;
    }

    default:
    {
        printf("No test specified.\n");
        break;
    }

    }
}
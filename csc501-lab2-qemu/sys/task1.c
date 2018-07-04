#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

extern unsigned long ctr1000;

long semin, semout, lockin, lockout;

void semreader (char msg, int sem){
	kprintf ("  %c: to acquired semaphore\n", msg);
	wait(sem);
	kprintf ("  %c: acquired semaphore\n", msg);
	int init = 0;
	sleep(10);
	signal(sem);
	kprintf("  %c: released semaphore\n", msg);
}

void semwriter (char msg, int sem){
	kprintf ("  %c: to acquired semaphore\n", msg);
	semin = ctr1000;
	wait(sem);
	kprintf ("  %c: acquired semaphore\n", msg);
	int init = 0;
	semout = ctr1000;
	sleep(10);
	signal(sem);
	long x = semout - semin;
	kprintf("  %c: released semaphore : Time A  get the semaphore %ld \n", msg, x);
}

void lockwriter	(char msg, int ldes){
	kprintf ("  %c: to acquired lock\n", msg);
	lockin = ctr1000;
	lock(ldes, WRITE, DEFAULT_LOCK_PRIO);
	lockout = ctr1000;
	kprintf ("  %c: acquired lock\n", msg);
	int init = 0;
	sleep(10);
	releaseall(1, ldes);
	long x = lockout - lockin;
	kprintf("  %c: released lock : Time A get the lock %ld  \n", msg, x);
}

void lockreader	(char msg, int ldes){
	lock(ldes, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %c: to acquired lock\n", msg);
	kprintf ("  %c: acquired lock\n", msg);
	int init = 0;
	sleep(10);
	releaseall(1, ldes);
	kprintf("  %c: released lock\n", msg);
}

void pro(){
	sleep(10);
}


void semPrioInver(){
	int semaphore, reader1, writer1, writer2, reader2, reader3, reader4, procc;
	kprintf("Inside Semaphore Implementation\n");
	semaphore = screate(4);
        writer1 = create(semwriter, 2000, 25, "Writer", 2,'A', semaphore);
        reader1 = create(semreader, 2000, 20, "Reader", 2,'B', semaphore);
		reader2 = create(semreader, 2000, 18, "Reader", 2,'C', semaphore);
		reader3 = create(semreader, 2000, 15, "Reader", 2,'D', semaphore);
		reader4 = create(semreader, 2000, 12, "Reader", 2,'E', semaphore);
		procc = create(pro, 2000, 23, "Pro", 1,1);
		resume(reader1);
		sleep(1);
		
		resume(reader2);
		sleep(1);
		
		resume(reader3);
		sleep(1);
		
		resume(reader4);
		sleep(1);
		
		resume(procc);
        resume(writer1);
		
		kprintf ("Priority of B doesn't increases to A, B's Priority : %d\n", getprio(reader1));
		kprintf ("Priority of C doesn't increases to A, C's Priority : %d\n", getprio(reader2));
		kprintf ("Priority of D doesn't increases to A, D's Priority : %d\n", getprio(reader3));
		kprintf ("Priority of E doesn't increases to A, E's Priority : %d\n", getprio(reader4));
		
		sleep(1);

        sleep (20);
}

void locPrioInver(){
	int lock, reader1, writer1, writer2, reader2, reader3, reader4, procc;
	kprintf("Inside Lock Implementation\n");
	lock = lcreate();
        writer1 = create(lockwriter, 2000, 25, "Writer", 2,'A', lock);
        reader1 = create(lockreader, 2000, 20, "Reader", 2,'B', lock);
		reader2 = create(lockreader, 2000, 18, "Reader", 2,'C', lock);
		reader3 = create(lockreader, 2000, 15, "Reader", 2,'D', lock);
		reader4 = create(lockreader, 2000, 12, "Reader", 2,'E', lock);
		procc = create(pro, 2000, 23, "Pro", 1,1);
		
		resume(reader1);
		sleep(1);
		
		resume(reader2);
		sleep(1);
		
		resume(reader3);
		sleep(1);
		
		resume(reader4);
		sleep(1);

		resume(pro);
        resume(writer1);
		
		kprintf ("Priority of B increases to A, B's Priority : %d\n", getprio(reader1));
		kprintf ("Priority of C increases to A, C's Priority : %d\n", getprio(reader2));
		kprintf ("Priority of D increases to A, D's Priority : %d\n", getprio(reader3));
		kprintf ("Priority of E increases to A, E's Priority : %d\n", getprio(reader4));
		
		sleep(1);

        sleep (20);
}

int task1()
{
	kprintf("\nWith Semaphores Priority Inversion Implementation\n");
	semPrioInver();	
	kprintf("\nWith Lock Priority Inversion Implementation\n");
	locPrioInver();
	kprintf("\nCompleted\n");
	sleep(30);
	
}
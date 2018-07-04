#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newloc(); 
extern unsigned long ctr1000;

int lcreate()
{
	STATWORD ps;    
	int loc;
	
	disable(ps);
	if ((loc=newloc())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	return(loc);
}

LOCAL int newloc()
{
	int	loc;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		loc=nextloc--;
		if (nextloc < 0)
			nextloc = NLOCKS-1;
			//kprintf("dsads %d\n",locadd[loc].lstate);
		if (locadd[loc].lstate==LFREE) {
			locadd[loc].lstate = LUSED;
			locadd[loc].ltype = LNONE;
			locadd[loc].created = ctr1000;
			globallock = loc;
			return(loc);
		}
	}
	return(SYSERR);
}
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

int ldelete(int loc)
{
	STATWORD ps;    
	int	i, pid;
	struct	lentry	*lptr;

	disable(ps);
	if (isbadloc(loc) || locadd[loc].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &locadd[loc];
	lptr->lstate = LFREE;
	lptr->ltype = LNONE;
	lptr->noofreaders = 0;
	if (nonempty(lptr->lhead)) {
		while( (pid=getfirst(lptr->lhead)) != EMPTY)
		  {
		    proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO);
		  }
		resched();
	}
	for(i=1; i<NPROC; i++){
		if(locadd[loc].allprocess[i] == READ || locadd[loc].allprocess[i] == WRITE){
		//kprintf("inside for");
		locadd[loc].allprocess[i] = DELETED;
		}
	}
	restore(ps);
	return(OK);
}
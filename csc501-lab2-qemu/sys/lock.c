#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

extern unsigned long ctr1000;

int prio_writer(int prio, int time, int id, int tail, int ldes){
	int	prev;
	prev = q[tail].qprev;
	while (q[prev].qkey > prio && prev != locadd[ldes].lhead){
		if(proctab[prev].lstatus == WRITE)	return TRUE;
		prev = q[prev].qprev;
	}
	/*if(prev != locadd[ldes].lhead){
		if(proctab[prev].lstatus == READ)	return TRUE;
		else if(proctab[prev].lstatus == WRITE){
			int t =proctab[prev].lreqtim - time;
			if(t > 500){
				return TRUE;
			}			
			else{
				return FALSE;
			}			
		}
	}*/
	return FALSE;
}

int max_locker(int ldes){
	int prev, max = -9999;
	prev = q[locadd[ldes].ltail].qprev;
	while(prev != locadd[ldes].lhead){
		if(proctab[prev].pprio >= max)	max = proctab[prev].pprio;
		prev = q[prev].qprev;
	}
	return max;
}

void priorityIn(int ldes){
	struct pentry *pptr, *npptr;
	pptr = &proctab[currpid];
	struct lentry *lptr;
	lptr = &locadd[ldes];
	if(pptr->pstate = PRWAIT){
		int i = 0;
		for(i = 0; i<NPROC; i++){
			if(lptr->allprocess[i] == READ || lptr->allprocess[i] == WRITE){
				if(pptr->pprio > proctab[i].pprio){
					if(proctab[i].pinh == -1)	proctab[i].pinh = proctab[i].pprio;	
				chprios(i,pptr->pprio);
				}
			}
		}
	}
}

void prioritt(int ldes, int pid){
	struct lentry *lptr;
	lptr = &locadd[ldes];
	int max = max_locker(ldes);
	if(proctab[pid].pprio < max)	chprios(pid, max);
}


int lock (int ldes1, int type, int priority){
	STATWORD ps;
	struct	pentry	*pptr;
	
	disable(ps);
	if(((proctab[currpid].lacquire[ldes1] - locadd[ldes1].created) < 0) && (proctab[currpid].lacquire[ldes1] != -1)){
		restore(ps);
		return(SYSERR);
	}	
	else{	
	if (isbadloc(ldes1) || locadd[ldes1].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	else if(locadd[ldes1].allprocess[currpid] == DELETED){
		locadd[ldes1].allprocess[currpid] = LNONE;
		return(DELETED);
	}
	else{
		if(locadd[ldes1].ltype == LNONE){
			proctab[currpid].lstatus = type;
			proctab[currpid].lreqtim = ctr1000;
			proctab[currpid].ltype[ldes1] = type;
			locadd[ldes1].ltype = type;
			locadd[ldes1].allprocess[currpid] = type;
			prioritt(ldes1, currpid);
			if(type == READ)	locadd[ldes1].noofreaders++;
		}
		else if(locadd[ldes1].ltype == WRITE){
			proctab[currpid].lstatus = type;
			proctab[currpid].lreqtim = ctr1000;
			proctab[currpid].pstate = PRWAIT;
			proctab[currpid].ltype[ldes1] = type;
			proctab[currpid].waitlock = ldes1;
			insert(currpid, locadd[ldes1].lhead, priority);
			priorityIn(ldes1);
			resched();
		}
		else if(locadd[ldes1].ltype == READ){
			if(type == WRITE){
				proctab[currpid].lstatus = type;
				proctab[currpid].lreqtim = ctr1000;
				proctab[currpid].ltype[ldes1] = type;
				proctab[currpid].pstate = PRWAIT;
				proctab[currpid].waitlock = ldes1;
				insert(currpid, locadd[ldes1].lhead, priority);
				priorityIn(ldes1);
				resched();
			}
			else if(type == READ){
				if(prio_writer(priority, ctr1000, currpid, locadd[ldes1].ltail, ldes1) == TRUE){
					proctab[currpid].lstatus = type;
					proctab[currpid].lreqtim = ctr1000;
					proctab[currpid].ltype[ldes1] = type;
					proctab[currpid].pstate = PRWAIT;
					proctab[currpid].waitlock = ldes1;
					insert(currpid, locadd[ldes1].lhead, priority);
					priorityIn(ldes1);
					resched();
				}
				else{
					proctab[currpid].lstatus = type;
					proctab[currpid].lreqtim = ctr1000;
					proctab[currpid].ltype[ldes1] = type;
					locadd[ldes1].allprocess[currpid] = type;
					locadd[ldes1].noofreaders++;
					prioritt(ldes1, currpid);
					locadd[ldes1].ltype = type;
				}
			}
		}
	}
	restore(ps);
	return(OK);
	}	
}
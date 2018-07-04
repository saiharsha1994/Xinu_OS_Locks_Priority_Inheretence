/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
int pidd, newprioo, changed = 0;

int max_lock(int ldes){
	int prev, max = -9999;
	prev = q[locadd[ldes].ltail].qprev;
	while(prev != locadd[ldes].lhead){
		if(proctab[prev].pprio >= max)	max = proctab[prev].pprio;
		prev = q[prev].qprev;
	}
	return max;
}

int max_count(int max_there, int ldes){
	int prev, count = 0;
	prev = q[locadd[ldes].ltail].qprev;
	while(prev != locadd[ldes].lhead){
		if(proctab[prev].pprio == max_there )	count++;
		prev = q[prev].qprev;
	}
	return count;
}

int max_second(int max_there, int ldes){
	int prev, count = 9999, id=-1;
	prev = q[locadd[ldes].ltail].qprev;
	while(prev != locadd[ldes].lhead){
		if(max_there - proctab[prev].pprio < count && max_there != proctab[prev].pprio){
			count = max_there - proctab[prev].pprio;
			id = proctab[prev].pprio;
		}
		prev = q[prev].qprev;
	}
	return id;
}

int max_all(int pid, int wait_ldes){
	int g, all = 0;
	int al[100], count = 0;
	for(g = 0; g<NLOCKS; g++){
		if(proctab[pid].ltype[g] != -1 && g != wait_ldes){
			al[count] = g;
			count++;
		}
	}
	int full = 0, max = -9999;
		for(full = 0; full < count; full++){
			if(max < max_lock(al[full]))	max = max_lock(al[full]);	
		}
	return max;		
}

int all_all(struct pentry *ptr, int oldprio, int newprio){
				struct lentry *lptr = &locadd[ptr->waitlock];
				int unchanged = newprio;
				int max_there = max_lock(ptr->waitlock);
				int maxall = max_all(pidd, ptr->waitlock);
				if(maxall > newprio)	newprio = maxall;
				if(max_there > oldprio){}
				else{
					int max_counts = max_count(max_there, ptr->waitlock);				
					if(max_counts > 1){}
					else{
						int check = max_second(max_there, ptr->waitlock);
						int togive = 0;
						if(check == -1)	togive = newprio;
						else if(check > newprio)	togive = check;
						else togive = newprio;
						int in_lock = 0;
						ptr->pprio = newprio;
						ptr->pinh = unchanged;
						for(in_lock = 0; in_lock < NPROC; in_lock++){
							if(lptr->allprocess[in_lock] == READ || lptr->allprocess[in_lock] == WRITE){
							if(proctab[in_lock].pinh < togive)	chprios(in_lock, togive);
							else	chprios(in_lock, proctab[in_lock].pinh);
							}
						}
					}						
				}
				return newprio;
}

void newprocChange(int pid, int newprio){
	int g, all = 0;
	int al[100], count = 0; 
	for(g = 0; g<NLOCKS; g++){
			if(proctab[pid].ltype[g] != -1){
			al[count] = g;
			count++;
			all = g;
			}
		}
	if(count > 1){
		int full = 0, max = -9999;
		for(full = 0; full < count; full++){
			if(max < max_lock(al[full]))	max = max_lock(al[full]);	
		}
		if(max <= newprio){
			proctab[pid].pinh = newprio;
			proctab[pid].pprio = newprio;
		}
		else{
			changed = 1;
			proctab[pid].pinh = newprio;
		}
	}
	else{
		int a = max_lock(all);
		if(a <= newprio){
			proctab[pid].pinh = newprio;
			proctab[pid].pprio = newprio;
		}
		else{
			changed = 1;
			proctab[pid].pinh = newprio; 
		}
	}
}

void procChange(int pid, int newprio){
	struct pentry *ptr;
	ptr = &proctab[pidd];
	if(ptr->pstate == PRWAIT){
		int max = -9999;
		int prev = q[locadd[ptr->waitlock].ltail].qprev;
		while(prev != locadd[ptr->waitlock].lhead){
			if(proctab[prev].pprio >= max)	max = proctab[prev].pprio;
			prev = q[prev].qprev;
		}
		if(max < newprio){
			int oldprio = ptr->pprio;
			if(max >= oldprio){
				ptr->pprio = newprio;
				ptr->pinh = newprio;
				struct lentry *lptr = &locadd[ptr->waitlock];
				int in_locks;
				for(in_locks = 0; in_locks < NPROC; in_locks++){
					if(lptr->allprocess[in_locks] == READ || lptr->allprocess[in_locks] == WRITE){
						chprios(in_locks, newprio);
					}
				}
			}
			else if(max < oldprio){
				if(ptr->pinh > max){
					int newp = all_all(ptr, oldprio, newprio);
				}
				else{
					int newp = all_all(ptr, oldprio, newprio);
				}
			}	
		}
		else if(max == newprio){
			int oldprio = ptr->pprio;
			if(max <= oldprio){
				int newp = all_all(ptr, oldprio, newprio);
			}
			else if(max > oldprio){
				ptr->pprio = newprio;
				ptr->pprio = newprio;
			}
		}
		else if(max > newprio){
			int oldprio = ptr->pprio;
			if(max > oldprio){
				ptr->pprio = newprio;
				ptr->pinh = newprio;
			}
			else if(max <= oldprio){
				int newp = all_all(ptr, oldprio, newprio);
			}
		}
	}
	else{
		newprocChange(pid, newprio);
	}
}
 
SYSCALL chprio(int pid, int newprio)
{
	pidd = pid;
	newprioo = newprio;
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	procChange(pid, newprio);
	if(changed != 1)	pptr->pprio = newprio;
	restore(ps);
	return(newprio);
}





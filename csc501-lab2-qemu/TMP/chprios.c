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
int pidds, newprioos, changeds = 0;

int max_locks(int ldes){
	int prev, max = -9999;
	prev = q[locadd[ldes].ltail].qprev;
	while(prev != locadd[ldes].lhead){
		if(proctab[prev].pprio >= max)	max = proctab[prev].pprio;
		prev = q[prev].qprev;
	}
	return max;
}

int max_counts(int max_there, int ldes){
	int prev, count = 0;
	prev = q[locadd[ldes].ltail].qprev;
	while(prev != locadd[ldes].lhead){
		if(proctab[prev].pprio == max_there )	count++;
		prev = q[prev].qprev;
	}
	return count;
}

int max_seconds(int max_there, int ldes){
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

int max_alls(int pid, int wait_ldes){
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
			if(max < max_locks(al[full]))	max = max_locks(al[full]);	
		}
	return max;		
}

int all_alls(struct pentry *ptr, int oldprio, int newprio){
				struct lentry *lptr = &locadd[ptr->waitlock];
				int unchanged = newprio;
				int max_there = max_locks(ptr->waitlock);
				int maxall = max_alls(pidds, ptr->waitlock);
				if(maxall > newprio)	newprio = maxall;
				if(max_there > oldprio){}
				else{
					int max_countss = max_counts(max_there, ptr->waitlock);					
					if(max_countss > 1){}
					else{
						int check = max_seconds(max_there, ptr->waitlock);
						int togive = 0;
						if(check == -1)	togive = newprio;
						else if(check > newprio)	togive = check;
						else togive = newprio;
						int in_lock = 0;
						ptr->pprio = newprio;
						ptr->pinh = ptr->pinh;
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

void newprocChanges(int pid, int newprio){
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
			if(max < max_locks(al[full]))	max = max_locks(al[full]);	
		}
		if(max <= newprio){
			proctab[pid].pinh = proctab[pid].pinh;
			proctab[pid].pprio = newprio;
		}
		else{
			changeds = 1;
			proctab[pid].pinh = proctab[pid].pinh;
		}
	}
	else{
		int a = max_locks(all);
		if(a <= newprio){
			proctab[pid].pinh = proctab[pid].pinh;
			proctab[pid].pprio = newprio;
		}
		else{
			changeds = 1;
			proctab[pid].pinh = proctab[pid].pinh; 
		}
	}
}

void procChanges(int pid, int newprio){
	struct pentry *ptr;
	ptr = &proctab[pidds];
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
				ptr->pinh = ptr->pinh;
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
					int newp = all_alls(ptr, oldprio, newprio);
				}
				else{
					int newp = all_alls(ptr, oldprio, newprio);
				}
			}	
		}
		else if(max == newprio){
			int oldprio = ptr->pprio;
			if(max <= oldprio){
				int newp = all_alls(ptr, oldprio, newprio);
			}
			else if(max > oldprio){
				ptr->pprio = newprio;
				ptr->pinh = ptr->pinh;
			}
		}
		else if(max > newprio){
			int oldprio = ptr->pprio;
			if(max > oldprio){
				ptr->pprio = newprio;
				ptr->pinh = ptr->pinh;
			}
			else if(max <= oldprio){
				int newp = all_alls(ptr, oldprio, newprio);
			}
		}
	}
	else{
		newprocChanges(pid, newprio);
	}
}
 
SYSCALL chprios(int pid, int newprio)
{
	pidds = pid;
	newprioos = newprio;
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	procChanges(pid, newprio);
	if(changeds != 1)	pptr->pprio = newprio;
	restore(ps);
	return(newprio);
}





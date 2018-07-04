#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

int max_lockss(int ldes){
	int prev, max = -9999;
	prev = q[locadd[ldes].ltail].qprev;
	while(prev != locadd[ldes].lhead){
		if(proctab[prev].pprio >= max)	max = proctab[prev].pprio;
		prev = q[prev].qprev;
	}
	return max;
}

int max_allss(int pid	){
	int g, all = 0;
	int al[100], count = 0;
	for(g = 0; g<NLOCKS; g++){
		if(proctab[pid].ltype[g] != -1){
			al[count] = g;
			count++;
		}
	}
	int full = 0, max = -9999;
		for(full = 0; full < count; full++){
			if(max < max_lockss(al[full]))	max = max_lockss(al[full]);	
		}
	return max;		
}

void changepriority(int ldes){
	int prev, max = -9999, in_lock;
	struct lentry *lptr = &locadd[ldes];
	prev = q[locadd[ldes].ltail].qprev;
	while(prev != locadd[ldes].lhead){
		if(proctab[prev].pprio >= max)	max = proctab[prev].pprio;
		prev = q[prev].qprev;
	}
	for(in_lock = 0; in_lock < NPROC; in_lock++){
		if(lptr->allprocess[in_lock] == READ || lptr->allprocess[in_lock] == WRITE){
				if(proctab[in_lock].pinh < max)	chprios(in_lock, max);
				else	chprios(in_lock, proctab[in_lock].pinh);
		}
	}
}

int changeotherlocks(){
	int a = max_allss(currpid);
}

int nextprocess(int ldes){
	int back, all[50], allcount = 0;
	if(q[locadd[ldes].ltail].qprev == locadd[ldes].lhead){
		locadd[ldes].lstate = LNONE;
		return OK;
	}
	else{
		int id = q[locadd[ldes].ltail].qprev;
	int idd = q[id].qprev;
	if(proctab[id].ltype[ldes] == WRITE){
		if(q[id].qkey > q[idd].qkey){
			proctab[id].waitlock = -1;
			dequeue(id);
			changepriority(ldes);
			ready(id, RESCHNO);
			locadd[ldes].allprocess[id] = WRITE;
			locadd[ldes].ltype = WRITE;
			return OK;
		}
		else{
			int count = 0;
			while(q[id].qkey == q[idd].qkey && idd != locadd[ldes].lhead){
				if(proctab[idd].ltype[ldes] == READ){
					int tim = proctab[idd].lreqtim-proctab[id].lreqtim;
					if(tim < 500){
						count++;
						all[allcount] = idd;
						allcount++;
						int iddd = q[idd].qprev;
						int timm = proctab[iddd].lreqtim-proctab[id].lreqtim;
						while(timm < 500 && iddd != locadd[ldes].lhead){
							all[allcount] = iddd;
							allcount++;
							iddd = q[iddd].qprev;
							if(iddd != locadd[ldes].lhead)	timm = proctab[iddd].lreqtim-proctab[id].lreqtim;
						}
						break;
					}
				}
				idd = q[idd].qprev;
			}
			if(count == 0){
				proctab[id].waitlock = -1;
				dequeue(id);
				changepriority(ldes);
				ready(id, RESCHNO);
				locadd[ldes].allprocess[id] = WRITE;
				locadd[ldes].ltype = WRITE;
			}
			else{
				int i = 0;
				for(i = 0; i < allcount; i++){
					proctab[all[i]].waitlock = -1;
					dequeue(all[i]);
					changepriority(ldes);
					ready(all[i], RESCHNO);
					locadd[ldes].noofreaders++;
					locadd[ldes].ltype = READ;
					locadd[ldes].allprocess[all[i]] = READ;
				}
			}
			return OK;
		}
	}	
	else{
			all[allcount] = id;
			allcount++;
			while(q[id].qkey >= q[idd].qkey && idd != locadd[ldes].lhead){
				if(proctab[idd].ltype[ldes] == READ){
					all[allcount] = idd;
					allcount++;
				}
				else{
					int write_lock_prio = q[idd].qkey;
					int iddd = q[idd].qprev;
					int tim = proctab[iddd].lreqtim-proctab[idd].lreqtim;
					while(q[idd].qkey < q[iddd].qkey && tim < 500 && iddd != locadd[ldes].lhead){
						if(proctab[iddd].ltype[ldes] == READ){
							all[allcount] = iddd;
							allcount++;
						}
						iddd = q[iddd].qprev;
						if(iddd != locadd[ldes].lhead)	tim = proctab[iddd].lreqtim-proctab[idd].lreqtim;
					}
					break;
				}
				idd = q[idd].qprev;
			}
			int i = 0;
			for(i = 0; i < allcount; i++){
				proctab[all[i]].waitlock = -1;
				dequeue(all[i]);
				changepriority(ldes);
				ready(all[i], RESCHNO);
				locadd[ldes].noofreaders++;
				locadd[ldes].ltype = READ;
				locadd[ldes].allprocess[all[i]] = READ;
			}
			return OK;
	}
	}
}


int releaseall(int numlocks, int ldes1, ...){
	STATWORD ps;    
	struct	lentry	*lptr;

	disable(ps);
	
	int i = 0, ldes, back = OK, condition = 0, nextid;
	
	for(i = 0; i < numlocks; i++){
		ldes = *((int*)&ldes1 + i);
		lptr = &locadd[ldes];
		if (isbadloc(ldes) || lptr->lstate == LFREE || lptr->allprocess[currpid] == LNONE) {
			restore(ps);
			back = SYSERR;
		}
		else{
			lptr->allprocess[currpid] = LNONE;
			if(proctab[currpid].pinh != -1){
				int aa = proctab[currpid].pinh;
				proctab[currpid].pinh = proctab[currpid].pprio;
				proctab[currpid].pprio = aa;
			}
			int a = changeotherlocks();
			if(a != -9999) chprio(currpid, a);
			if(lptr->ltype == WRITE)	condition = 1;
			else if(lptr->ltype == READ){
				lptr->noofreaders--;
				if(lptr->noofreaders > 0)	condition = 0;
				else	condition = 1;
			}
			if(condition == 0){
			}
			else if(condition == 1){
				int backs = nextprocess(ldes);
				if(back != -1)	back = backs;
			}
		}
	}
	resched();
	restore(ps);
	return(back);
}
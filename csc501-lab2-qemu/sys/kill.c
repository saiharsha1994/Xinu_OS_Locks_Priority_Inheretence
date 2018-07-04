/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
 
 int pidd;
 void proc(){
	struct pentry *ptr;
	ptr = &proctab[pidd];
	if(ptr->pstate = PRWAIT){
		int max = -9999, count=0;
		int prev = q[locadd[ptr->waitlock].ltail].qprev;
		while(prev != locadd[ptr->waitlock].lhead){
			if(proctab[prev].pprio >= max){
				max = proctab[prev].pprio;
			}
			prev = q[prev].qprev;
		}
		int i;
		struct lentry *lptr;
		lptr = &locadd[ptr->waitlock];
		for(i = 0; i<NPROC; i++){
				if(lptr->allprocess[i] == READ || lptr->allprocess[i] == WRITE){
					if(max <= proctab[i].pinh){
						chprios(i, proctab[i].pinh);
					}
					else{
						chprios(i, max);
					}
				}
			}
	}
}
 
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	pidd = pid;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:
	dequeue(pid);
	proc();
	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}


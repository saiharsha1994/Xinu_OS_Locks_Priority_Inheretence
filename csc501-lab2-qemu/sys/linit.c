#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <stdio.h>
#include <lock.h>

int	nextloc;
int globallock;
struct	lentry	locadd[NLOCKS];

void linit(){
	nextloc = NLOCKS-1;
	globallock = -1;
	int i = 0;
	struct	lentry	*lptr;
	for (i=0 ; i<NLOCKS ; i++) {	/* initialize locks */
		int j;
		(lptr = &locadd[i])->lstate = LFREE;
		lptr->ltype = LNONE;
		lptr->noofreaders = 0;
		lptr->ltail = 1 + (lptr->lhead = newqueue());
		for(j =0; j<NPROC; j++){
			lptr->allprocess[j] = LNONE;
		}
	}
}
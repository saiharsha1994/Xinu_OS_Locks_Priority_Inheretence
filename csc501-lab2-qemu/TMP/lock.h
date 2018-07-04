				
#define	NLOCKS		50		

#define	READ		'\00'
#define	WRITE		'\01'		
#define	LNONE		'\03'
#define	LFREE		'\04'
#define	LUSED		'\05'		

struct	lentry	{			
	int	lhead;			
	int	ltail;
	int noofreaders;
	int allprocess[NPROC];
	int created;
	char	lstate;			
	char	ltype;					
};

extern	struct	lentry	locadd[];
extern	int	nextloc;
extern int globallock;

#define	isbadloc(s)	(s<0 || s>=NLOCKS)

void linit();
int lcreate();
int ldelete(int);
int lock(int, int, int);
int releaseall(int, int, ...);
#include <signal.h>
#include <string.h>

#include "unp_misc/signal.h"

namespace UNP {

SigFunc signal_r(int signo, SigFunc func) {
	struct sigaction act, oact;
	
	bzero(&act, sizeof(act));
	
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	if(signo == SIGALRM) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	}
	else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if(sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
}

}

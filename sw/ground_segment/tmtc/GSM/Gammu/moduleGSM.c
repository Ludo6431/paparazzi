#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <Ivy/ivy.h>
#include <Ivy/ivyglibloop.h>
#include <gammu.h>

#include "gammutools.h"

#define TIMEOUT_PERIOD 10000

int main() {
	GSM_StateMachine *s;
	GMainLoop *ml;

	s = gsm_setup();
	if(!s)
		return EXIT_FAILURE;

	if(gsm_connect(s))
		return EXIT_FAILURE;

	ml =  g_main_loop_new(NULL, FALSE);
	IvyInit("IvyReceptionGSM", "IvyReceptionGSM READY", NULL, NULL, NULL, NULL);
	IvyStart("127.255.255.255");
	g_timeout_add(TIMEOUT_PERIOD, (GSourceFunc)gsm_receive, s);
	g_main_loop_run(ml);

	return EXIT_SUCCESS;
}


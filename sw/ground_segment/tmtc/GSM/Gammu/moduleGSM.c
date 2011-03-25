#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <Ivy/ivy.h>
#include <Ivy/ivyglibloop.h>
#include <gammu.h>

#include "gammutools.h"
#include "smsprotocol.h"

#define TIMEOUT_PERIOD 10000

int main() {
	GMainLoop *ml;

	if(gsm_setup())
		return EXIT_FAILURE;
	if(gsm_connect())
		return EXIT_FAILURE;

	ml = g_main_loop_new(NULL, FALSE);
	IvyInit("IvyReceptionGSM", "IvyReceptionGSM READY", NULL, NULL, NULL, NULL);
	IvyStart("127.255.255.255");
	g_timeout_add(TIMEOUT_PERIOD, (GSourceFunc)gsm_receive, NULL /* TODO callback */);
	g_main_loop_run(ml);

	return EXIT_SUCCESS;
}


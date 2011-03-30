#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <Ivy/ivy.h>
#include <Ivy/ivyglibloop.h>

#include "gammutools.h"
#include "smsprotocol.h"

#define TIMEOUT_PERIOD 10000
#define IVY_SENDER_ID 16

void on_smsrx(char *number, time_t date /* UTC */, char *rxdata) {
    fprintf(stderr, "INFO: Received message sent on %s UTC by %s: \"%s\".\n", asctime(gmtime(&date)), number, rxdata);

    if(sms_handle(rxdata, IVY_SENDER_ID))
        printf("ERR: Failed parsing data\n");
}

int main(int argc, char *argv[]) {
    GMainLoop *ml;

    if(gsm_setup() || gsm_connect())
        return EXIT_FAILURE;

    ml = g_main_loop_new(NULL, FALSE);
    IvyInit("IvyReceptionGSM", "IvyReceptionGSM READY", NULL, NULL, NULL, NULL);
    IvyStart("127.255.255.255");
    g_timeout_add(TIMEOUT_PERIOD, (GSourceFunc)gsm_receive, on_smsrx);
    g_main_loop_run(ml);

    return EXIT_SUCCESS;
}


#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <Ivy/ivy.h>
#include <Ivy/ivyglibloop.h>
#include <gammu.h>

void mexit(int r, char *format, ...) {
    if(format) {
        va_list ap;
        va_start(ap, format);
        if(r) fprintf(stderr, "ERR:");
        vfprintf(stderr, format, ap);
        fprintf(stderr, "\n");
        va_end(ap);
    }

    exit(r);
}

void on_ivyrx(IvyClientPtr app, void *user_data, int argc, char *argv[]) {
    printf("IVYRX(from %s@%s): ", IvyGetApplicationName(app), IvyGetApplicationHost(app));
    int first=1;
    for(;argc--;) {
        printf("%s\"%s\"", first?"":", ", *argv++);
        first=0;
    }
    printf("\n");
}

gboolean smsrxcheck(gpointer data) {
    GSM_StateMachine *sm = (GSM_StateMachine *)data;
    GSM_Error error;
    GSM_MultiSMSMessage sms;
    bzero(&sms, sizeof(GSM_MultiSMSMessage));
    int i;

    // get all messages, reemit new ones on ivy
    error = GSM_GetNextSMS(sm, &sms, 1);
    while(1) {
        switch(error) {
        case ERR_EMPTY:
            // ok, nothing to do
            return TRUE;
        case ERR_NONE:
            // read message

            for (i = 0; i < sms.Number; i++)
                printf("SMSRX(from %s stored in %d/%d): \"%s\"\n",
                    DecodeUnicodeConsole(sms.SMS[i].Number), sms.SMS[i].Folder, sms.SMS[i].Location,
                    (sms.SMS[i].Coding == SMS_Coding_8bit)?"N/A":DecodeUnicodeConsole(sms.SMS[i].Text)
                );

            break;
        default:
            // unexpected error
            mexit(EXIT_FAILURE, "Can't get SMS (%s)", GSM_ErrorString(error));
        }

        error = GSM_GetNextSMS(sm, &sms, 0);
    }
}

int main(int argc, char** argv) {
    // init gsm
    GSM_Error error;
        // init locale for charset conversion
    GSM_InitLocales(NULL);
        // init state machine
    GSM_StateMachine *sm = GSM_AllocStateMachine();
        // init config
    INI_Section *config;
    error = GSM_FindGammuRC(&config, NULL);
    if(error != ERR_NONE) mexit(EXIT_FAILURE, "Can't get gammu's config file");
    error = GSM_ReadConfig(config, GSM_GetConfig(sm, 0), 0);
    if(error != ERR_NONE) mexit(EXIT_FAILURE, "Can't read gammu's config file");
    INI_Free(config);
    GSM_SetConfigNum(sm, 1);
        // connect to phone
    error = GSM_InitConnection(sm, 1);
    if(error != ERR_NONE) mexit(EXIT_FAILURE, "Can't connect to phone");

    // init ivy
        // init main loop
    GMainLoop *ml =  g_main_loop_new(NULL, FALSE);
        // init broadcasting
    IvyInit("GSM_Example", "GSM_Example READY", NULL, NULL, NULL, NULL);
    IvyBindMsg(on_ivyrx, NULL, "^(.*)");    // intercept all messages
    IvyStart("127.255.255.255");

    // add periodic check for incoming messages
    g_timeout_add(5*1000, smsrxcheck, (gpointer)sm);   // each 5 seconds

    // let's go !
    g_main_loop_run(ml);

    return EXIT_SUCCESS;
}


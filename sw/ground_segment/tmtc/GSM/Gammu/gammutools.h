#ifndef _GAMMUTOOLS_H
#define _GAMMUTOOLS_H

#include <time.h>

typedef void (*SMSRXCallBack)(char *number, time_t date /* UTC */, char *data);

int gsm_setup();
int gsm_connect();
int gsm_close_connection();
int gsm_send(char* recipient_number, char* message_text);
gboolean gsm_receive(SMSRXCallBack rxcb);

#endif


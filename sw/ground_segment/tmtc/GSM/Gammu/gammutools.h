#ifndef _GAMMUTOOLS_H
#define _GAMMUTOOLS_H

int gsm_setup();
int gsm_connect();
int gsm_close_connection();
int gsm_send(char* numero, char* message_text);
gboolean gsm_receive(/* TODO callback */);

#endif


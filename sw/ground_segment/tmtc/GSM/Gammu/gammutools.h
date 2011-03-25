#ifndef _GAMMUTOOLS_H
#define _GAMMUTOOLS_H

GSM_StateMachine *gsm_setup();
int gsm_connect(GSM_StateMachine *s);
int gsm_close_connection(GSM_StateMachine *s);
int gsm_send(GSM_StateMachine *s, char* message_text, char* numero);
gboolean gsm_receive(GSM_StateMachine *s);

#endif


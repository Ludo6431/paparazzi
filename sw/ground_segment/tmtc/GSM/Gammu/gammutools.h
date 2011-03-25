#ifndef _GAMMUTOOLS_H
#define _GAMMUTOOLS_H

GSM_StateMachine *setup();
int connection(GSM_StateMachine *s);
int close_connection(GSM_StateMachine *s);
int envoyer_sms(GSM_StateMachine *s, char* message_text, char* numero);
gboolean reception(GSM_StateMachine *s);

#endif


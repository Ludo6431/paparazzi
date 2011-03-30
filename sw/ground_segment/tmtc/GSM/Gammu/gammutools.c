#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gammu.h>

#include "gammutools.h"

#define DEVICE "/dev/ttyUSB0"
#define CONNECTION "at"
#define MODEL ""

GSM_Error error;
GSM_StateMachine *s = NULL;

/* Function to handle errors */
void error_handler() {
	if(error != ERR_NONE) {
		printf("ERR: %s\n", GSM_ErrorString(error));
		if(GSM_IsConnected(s))
			GSM_TerminateConnection(s);
		exit(error);
	}
}

int gsm_setup() {
	GSM_Config *cfg;

	/*
	 * We don't need gettext, but need to set locales so that
	 * charset conversion works.
	 */
	GSM_InitLocales(NULL);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if(!s)
		return 1;

	/*
	 * Get pointer to config structure.
	 */
	cfg = GSM_GetConfig(s, 0);

	/*
 	 * Set configuration, first freeing old values.
 	 */
	free(cfg->Device);
	cfg->Device = DEVICE;
	free(cfg->Connection);
	cfg->Connection = CONNECTION;
	/* For historical reasons this is not a pointer */
	strcpy(cfg->Model,MODEL);

	/* We have one valid configuration */
	GSM_SetConfigNum(s, 1);

	return 0;
}

int gsm_connect() {
	/* Connect to phone */
	/* 1 means number of replies you want to wait for */
	error = GSM_InitConnection(s,1);
	error_handler(s);

	return 0;
}

int gsm_close_connection() {
	/* Terminate connection */
	error = GSM_TerminateConnection(s);
	error_handler(s);

	/* Free up used memory */
	//GSM_FreeStateMachine(s); Actuellement pose problème
	printf("Attention la place mémoire de la StateMachine n'est pas libéré\n");

	return 0;
}

/* Handler for SMS send reply */
void send_sms_callback(GSM_StateMachine *sm, int status, int MessageReference, void *user_data) {
	GSM_Error *sms_send_status = user_data;

	printf("Sent SMS on device: \"%s\"\n", GSM_GetConfig(sm, -1)->Device);
	if(!status) {
		printf("..OK");
		*sms_send_status = ERR_NONE;
	}
	else {
		printf("..error %i", status);
		*sms_send_status = ERR_UNKNOWN;
	}
	printf(", message reference=%d\n", MessageReference);
}

int gsm_send(char* recipient_number, char* message_text) {
	// On essaie de reconnecter le téléphone
	if(!GSM_IsConnected(s))
		gsm_connect(s);

	GSM_SMSMessage sms;
	GSM_SMSC PhoneSMSC;
	volatile GSM_Error sms_send_status;

	/*
	 * we don't need gettext, but need to set locales so that
	 * charset conversion works.
	 */
	GSM_InitLocales(NULL);

	/* prepare message */
	/* cleanup the structure */
	memset(&sms, 0, sizeof(sms));
	/* encode message text */
	EncodeUnicode(sms.Text, message_text, strlen(message_text));
	/* encode recipient number */
	EncodeUnicode(sms.Number, recipient_number, strlen(recipient_number));
	/* we want to submit message */
	sms.PDU = SMS_Submit;
	/* no udh, just a plain message */
	sms.UDH.Type = UDH_NoUDH;
	/* we used default coding for text */
	sms.Coding = SMS_Coding_Default_No_Compression;
	/* class 1 message (normal) */
	sms.Class = 1;

	/* set callback for message sending */
	/* this needs to be done after initiating connection */
	GSM_SetSendSMSStatusCallback(s, send_sms_callback, (void *)&sms_send_status);

	/* we need to know smsc number */
	PhoneSMSC.Location = 1;
	error = GSM_GetSMSC(s, &PhoneSMSC);
	error_handler(s);

	/* set smsc number in message */
	CopyUnicodeString(sms.SMSC.Number, PhoneSMSC.Number);

	/*
	 * set flag before callind sendsms, some phones might give
	 * instant response
	 */
	sms_send_status = ERR_TIMEOUT;

	/* send message */
	error = GSM_SendSMS(s, &sms);
	error_handler(s);

	/* wait for network reply */
	while(sms_send_status == ERR_TIMEOUT)
		GSM_ReadDevice(s, TRUE);

	return sms_send_status==ERR_NONE?0:1;
}

gboolean gsm_receive(SMSRXCallBack rxcb) {
	if (!GSM_IsConnected(s))
		gsm_connect(s);

	gboolean start;
	GSM_MultiSMSMessage sms;
	int i;

	/* read all messages */
	error = ERR_NONE;
	start = TRUE;
	sms.Number = 0;
	sms.SMS[0].Location = 0;
	sms.SMS[0].Folder = 0;
	while(error == ERR_NONE) {
		error = GSM_GetNextSMS(s, &sms, start);
		if (error == ERR_EMPTY) break;
		error_handler(s);
		start = FALSE;

		/* now we can do something with the message */
		for(i = 0; i < sms.Number; i++) {
			if(rxcb) {
				GSM_DateTime date = sms.SMS[i].DateTime;
				struct tm ts;
				ts.tm_year = date.Year - 1900;
				ts.tm_mon = date.Month - 1;
				ts.tm_mday = date.Day;
				ts.tm_hour = date.Hour;
				ts.tm_min = date.Minute;
				ts.tm_sec = date.Second;

				rxcb(DecodeUnicodeConsole(sms.SMS[i].Number), timegm(&ts)-date.Timezone, DecodeUnicodeConsole(sms.SMS[i].Text));
			}

			GSM_DeleteSMS(s, &sms.SMS[i]);
		}
	}

	return TRUE;
}


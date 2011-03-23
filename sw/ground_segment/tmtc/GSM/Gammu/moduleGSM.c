#define DEVICE "/dev/ttyUSB0"
#define CONNECTION "at"
#define MODEL ""
#define TIMEOUT_PERIOD 10000

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <Ivy/ivy.h>
#include <Ivy/ivyglibloop.h>
#include <gammu.h>


GMainLoop *ml;
GSM_StateMachine *s;
GSM_Error error;
char buffer[100];
volatile GSM_Error sms_send_status;
volatile gboolean gshutdown = FALSE;

/* Function to handle errors */
void error_handler(void)
{
	if (error != ERR_NONE) {
		printf("ERREUR: %s\n", GSM_ErrorString(error));
		if (GSM_IsConnected(s))
			GSM_TerminateConnection(s);
		exit(error);
	}
}

int setup()
{
	GSM_Config *cfg;

	/*
	 *  * We don't need gettext, but need to set locales so that
	 *   * charset conversion works.
	 *    */
	GSM_InitLocales(NULL);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL)
		return 3;
	
	/*
	 *  * Get pointer to config structure.
	 *   */
	cfg = GSM_GetConfig(s, 0);
	
	/*
 	*  * Set configuration, first freeing old values.
 	*   */
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

int connection(){
	/* Connect to phone */
	/* 1 means number of replies you want to wait for */
	error = GSM_InitConnection(s,1);
	error_handler();
	return 0;
}

int close_connection(){
	/* Terminate connection */
	error = GSM_TerminateConnection(s);
	error_handler();
	/* Free up used memory */
	//GSM_FreeStateMachine(s); Actuellement pose problème
	printf("Attention la place mémoire de la StateMachine n'est pas libéré\n");
	return 0;
}


/* Handler for SMS send reply */
void send_sms_callback (GSM_StateMachine *sm, int status, int MessageReference, void * user_data)
{
	printf("Sent SMS on device: \"%s\"\n", GSM_GetConfig(sm, -1)->Device);
	if (status==0) {
		printf("..OK");
		sms_send_status = ERR_NONE;
	} else {
		printf("..error %i", status);
		sms_send_status = ERR_UNKNOWN;
	}
	printf(", message reference=%d\n", MessageReference);
}

int envoyer_sms(char* message_text, char* numero)
{
	// On essie de reconnecter le téléphone
	if (!GSM_IsConnected(s)){
		connection();
	}

	GSM_SMSMessage sms;
	GSM_SMSC PhoneSMSC;
	char* recipient_number = numero;
	int return_value = 0;

	/*
	 *  * we don't need gettext, but need to set locales so that
	 *   * charset conversion works.
	 *    */
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
	GSM_SetSendSMSStatusCallback(s, send_sms_callback, NULL);

	/* we need to know smsc number */
	PhoneSMSC.Location = 1;
	error = GSM_GetSMSC(s, &PhoneSMSC);
	error_handler();

	/* set smsc number in message */
	CopyUnicodeString(sms.SMSC.Number, PhoneSMSC.Number);

	/*
	 *  * set flag before callind sendsms, some phones might give
	 *   * instant response
	 *    */
	sms_send_status = ERR_TIMEOUT;

	/* send message */
	error = GSM_SendSMS(s, &sms);
	error_handler();

	/* wait for network reply */
	while (!gshutdown) {
		GSM_ReadDevice(s, TRUE);
		if (sms_send_status == ERR_NONE) {
			/* message sent ok */
			return_value = 0;
			break;
		}
		if (sms_send_status != ERR_TIMEOUT) {
		/* message sending failed */
		return_value = 100;
		break;
		}
	}

	return return_value;
}

gboolean reception()
{	
	if (!GSM_IsConnected(s)){
		connection();
	}
	gboolean start;
	GSM_MultiSMSMessage sms;
	int i;

	/* read all messages */
	error = ERR_NONE;
	start = TRUE;
	sms.Number = 0;
	sms.SMS[0].Location = 0;
	sms.SMS[0].Folder = 0;
	while (error == ERR_NONE && !gshutdown) {
		error = GSM_GetNextSMS(s, &sms, start);
		if (error == ERR_EMPTY) break;
		error_handler();
		start = FALSE;

		/* now we can do something with the message */
		for (i = 0; i < sms.Number; i++) {
			GSM_DateTime date = sms.SMS[i].DateTime;
			IvySendMsg("%d:%d:%d %s",date.Hour,date.Minute,date.Second,DecodeUnicodeConsole(sms.SMS[i].Text));
			GSM_DeleteSMS(s, &(sms.SMS[i]));
		}
	}

	return TRUE;
}



int main()
{
	if(setup()==0){
		if(connection()==0){
			ml =  g_main_loop_new(NULL, FALSE);
			IvyInit ("IvyReceptionGSM", "IvyReceptionGSM READY", NULL, NULL, NULL, NULL);
			IvyStart("127.255.255.255");
			g_timeout_add(TIMEOUT_PERIOD, reception, NULL);
      			g_main_loop_run(ml);
		}
	}
	return 0;
}

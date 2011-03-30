#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <Ivy/ivy.h>

#include "messages.h"

#include "smsprotocol.h"

int sms_handle(char *data, int senderid) {
    assert(data);

    int len;
    unsigned int msgid;
    if(sscanf(data, "%d%n", &msgid, &len)!=1)
        return 1;
    data+=len+1;    // go to the next number

    switch(msgid) {
    case MSG01_SMS:{
        printf("INFO: Received MSG01_SMS ");

        int lat, lon, alt;
        short course, climb;
        unsigned short speed;
        unsigned char vsupply;
        if(sscanf(data, "%d %d %d %hd %hu %hd %hu%n", &lat, &lon, &alt, &course, &speed, &climb, (unsigned short *)&vsupply, &len)!=7) {
            printf("(BAD FORMAT)\n");
            return 1;
        }
        printf("(%dBytes OK):\n", len);

        printf("INFO:   position LLA: %d %d %d\n", lat, lon, alt);
        printf("INFO:   course:       %.1f°\n", (float)course/10.0);
        printf("INFO:   speed:        %.2fm/s\n", (float)speed/100.0);
        printf("INFO:   climb:        %.2fm/s\n", (float)climb/100.0);
        printf("INFO:   vsupply:      %.1fV\n", (float)vsupply/10.0);

        IvySendMsg("%d GPS_LLA %d %d %d %d %d %d 0 335297960 31 0", senderid, lat, lon, alt, course, speed, climb);
        IvySendMsg("%d FBW_STATUS 0 1 %d 0", senderid, vsupply);

/*Extraction(data_to_cut, ' ', 1, 1, ' ', 1, 0, extr_estimator_flight_time);*/
/*Extraction(data_to_cut, ' ', 1, 1, '\r', 1, 0, extr_qualite_signal_GSM);*/

        break;
    }
#if 0
    case MSG02_SMS:{
        printf("INFO: Received MSG02_SMS\n");

        IvySendMsg("%d %d %f %f %f", senderid, blabla...);
        break;
    }
#endif
    default:
        printf("WARN: Received unknown message with id %d\n", msgid);
        return 1;
    }

    return 0;
}


#ifndef libortc_h__connection
#define libortc_h__connection

#include "libortc.h"
#include <stdio.h>
#include <stdlib.h>


int  _ortc_prepare_websocket(ortc_context* context);
void _ortc_send(ortc_context* context, char* channel, char* message);
void _ortc_disconnect(ortc_context* context);
void *_ortc_reconnecting_loop(void *);
void _ortc_send_command(ortc_context *context, char *message);

#endif  // libortc_h__connection

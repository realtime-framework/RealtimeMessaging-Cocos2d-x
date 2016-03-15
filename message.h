#ifndef libortc_h__message
#define libortc_h__message

#include "libortc.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../external//websockets/include/ios/libwebsockets.h"


void _ortc_parse_message(ortc_context *context, char *message);

#endif  // libortc_h__message

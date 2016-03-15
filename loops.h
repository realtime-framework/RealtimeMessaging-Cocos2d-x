#ifndef libortc_h__loops
#define libortc_h__loops

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _ortc_init_main_thread(ortc_context *context);
int _ortc_quit_main_thread(ortc_context *context);

void _ortc_init_connection(ortc_context* context);
void _ortc_on_socket_closed(ortc_context* context);
void _ortc_change_state(ortc_context* context, enum _ortc_state newState);

int _ortc_run_connecting_loop(ortc_context* context);
void _ortc_cancel_connecting(ortc_context* context);

const char* _ortc_decodeState(enum _ortc_state state);
#endif  // libortc_h__loops
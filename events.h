#ifndef libortc_h__events
#define libortc_h__events

void _ortc_fireOnConnected(ortc_context *context);
void _ortc_fireOnDisconnected(ortc_context *context);
void _ortc_fireOnReconnected(ortc_context *context);
void _ortc_fireOnReconnecting(ortc_context *context);

#endif  // libortc_h__events
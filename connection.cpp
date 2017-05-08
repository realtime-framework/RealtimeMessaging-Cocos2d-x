#include "libortc.h"
#include "connection.h"
#include "balancer.h"
#include "channel.h"
#include "message.h"
#include "common.h"
#include "dlist.h"
#include "loops.h"
#include <sys/time.h>

#include "libwebsockets.h"

#include <pthread.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <Winsock2.h>
#else
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif


#define MAX_ORTC_PAYLOAD 1800

static int ortc_callback(
			 struct lws *wsi,
			 enum lws_callback_reasons reason,
			 void *user, 
 			 void *in, 
			 size_t len)
{
  ortc_context *context = (ortc_context *) user;
  ortc_dnode *t;
  char *messageToSend;
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + MAX_ORTC_PAYLOAD + LWS_SEND_BUFFER_POST_PADDING];
  switch (reason) {
  case LWS_CALLBACK_CLIENT_WRITEABLE:{
    if(context->throttleCounter>=2000){
      return 0;
    }
    if(context->ortcCommands->count>0){
      pthread_mutex_lock(&context->mutex_cmd);
      t = (ortc_dnode*)_ortc_dlist_take_first(context->ortcCommands);
      if(t!=NULL){
		messageToSend = t->id;
		sprintf((char *)&buf[LWS_SEND_BUFFER_PRE_PADDING], "%s", messageToSend);
		lws_write(context->wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], strlen(messageToSend), LWS_WRITE_TEXT);
		_ortc_dlist_free_dnode(t);
		context->throttleCounter++;
      }
	  pthread_mutex_unlock(&context->mutex_cmd);
    } else if(context->state == CONNECTED && context->throttleCounter<2000){
	  pthread_mutex_lock(&context->mutex_msg);
      t = (ortc_dnode*)_ortc_dlist_take_first(context->messagesToSend);
      if(t!=NULL){
		messageToSend = t->id;
		sprintf((char *)&buf[LWS_SEND_BUFFER_PRE_PADDING], "%s", messageToSend);
		lws_write(context->wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], strlen(messageToSend), LWS_WRITE_TEXT);
		_ortc_dlist_free_dnode(t);
		context->throttleCounter++;
      } 
	  pthread_mutex_unlock(&context->mutex_msg);
    }
    if( (context->messagesToSend->count>0 && context->throttleCounter<2000) || context->ortcCommands->count>0) {	
      lws_callback_on_writable(context->wsi);
    }
    break;
  }
  case LWS_CALLBACK_CLIENT_RECEIVE:{
    char * message = (char *) in;
    context->heartbeat_counter = 0;
    _ortc_parse_message(context, message);
    break;
  }
  case LWS_CALLBACK_CLOSED:{
	_ortc_on_socket_closed(context);
    break;
  }
	default : {
          break;
      }

  }
  return 0;
}

static struct lws_protocols ortc_protocols[] = {
  {"ortc-protocol", ortc_callback, 0, 4096},
  { NULL, NULL, 0, 0}
};



int _ortc_prepare_websocket(ortc_context* context){
  struct lws_context_creation_info info;

  if(context->lws_context)
    lws_context_destroy(context->lws_context);
  context->lws_context = NULL;
  if(context->host)
    free(context->host);
  context->host = NULL;
  if(context->server)
    free(context->server);
  context->server = NULL;

  memset(&info, 0, sizeof info);
  info.port = CONTEXT_PORT_NO_LISTEN;
  info.gid = -1;
  info.uid = -1;
  info.protocols = ortc_protocols;
  info.ssl_cipher_list = "RC4-MD5:RC4-SHA:AES128-SHA:AES256-SHA:HIGH:!DSS:!aNULL";

  info.ka_time = 0;
  info.ka_interval = 0;
  info.ka_probes = 0;
  
  context->lws_context = lws_create_context(&info);
  if (context->lws_context == NULL) {
    _ortc_exception(context,  (char*)"Creating libwebsocket context failed!");
    return -1;
  }
  return 0;
}

void _ortc_send_command(ortc_context *context, char *message){
  pthread_mutex_lock(&context->mutex_cmd);
  _ortc_dlist_insert(context->ortcCommands, message, NULL, NULL, 0, NULL);
  pthread_mutex_unlock(&context->mutex_cmd);
  free(message);
  lws_callback_on_writable(context->wsi);
}

void _ortc_send_message(ortc_context *context, char *message){
  pthread_mutex_lock(&context->mutex_msg);
  _ortc_dlist_insert(context->messagesToSend, message, NULL, NULL, 0, NULL);
  pthread_mutex_unlock(&context->mutex_msg);
  free(message);
  lws_callback_on_writable(context->wsi);
}

void _ortc_send(ortc_context* context, char* channel, char* message){
  int i;
  size_t len;
  char *hash = _ortc_get_channel_permission(context, channel);
  char messageId[9], sParts[15], sMessageCount[15];
    int messageCount = 0;
  char* messagePart, *m;
  size_t parts = strlen(message) / ORTC_MAX_MESSAGE_SIZE;

  _ortc_random_string(messageId, 9);
  if(strlen(message) % ORTC_MAX_MESSAGE_SIZE > 0)
    parts++;
  sprintf(sParts, "%d", (int)parts);

  for(i=0; i<parts; i++){
    size_t messageSize;
    char *messageR;
    messageSize = strlen(message) - i * ORTC_MAX_MESSAGE_SIZE;
    if(messageSize > ORTC_MAX_MESSAGE_SIZE)
      messageSize = ORTC_MAX_MESSAGE_SIZE;
    
    messageCount = i + 1;    
    
    sprintf(sMessageCount, "%d", messageCount);

    messagePart = (char*)malloc(messageSize+1);
    if(messagePart==NULL){
      _ortc_exception(context, (char*)"malloc() failed in ortc send!");
      return;
    }
    memcpy(messagePart, message + i * ORTC_MAX_MESSAGE_SIZE, messageSize);
    messagePart[messageSize] = '\0';

    messageR = _ortc_escape_sequences_before(messagePart);
  
    len = 15 + strlen(context->appKey) + strlen(context->authToken) + strlen(channel) + strlen(hash) + strlen(messageId) + strlen(sParts) + strlen(sMessageCount) + strlen(messageR); 
    m = (char*)malloc(len + 1);
    if(m == NULL){
      _ortc_exception(context, (char*)"malloc() failed in ortc send!");
      free(messagePart);
      free(messageR);
      return;
    }
    snprintf(m, len, "\"send;%s;%s;%s;%s;%s_%d-%d_%s\"", context->appKey, context->authToken, channel, hash, messageId, messageCount, (int)parts, messageR);
    free(messagePart);
    free(messageR);
    _ortc_send_message(context, m);
  }
}

void _ortc_disconnect(ortc_context *context){
	_ortc_change_state(context, DISCONNECTING);
}

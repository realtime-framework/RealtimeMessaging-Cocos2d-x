#include "message.h"
#include "common.h"
#include "channel.h"
#include "connection.h"
#include "loops.h"

#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <Winsock2.h>
#else
#include <netinet/tcp.h>
#endif

#if __APPLE__ || ANDROID
#include <sys/socket.h>
#endif

void _ortc_fire_onMessage(ortc_context *context, char *channel, char *message){
  char *messageR;
  ortc_dnode* t = _ortc_dlist_search(context->channels, channel);
  messageR =  _ortc_escape_sequences_after(message);
  if(t != NULL){
    if(t->callback != NULL)
      t->callback(context, channel, messageR);
  }
  free(messageR);
}

void _ortc_check_if_got_all_parts(ortc_context *context, char* messageId, int iMessageTotal){
  int i;
  size_t size, pos;
  char *ret, *channel;
  ortc_dnode **aNodes = (ortc_dnode**)malloc(sizeof(ortc_dnode*) * (iMessageTotal + 1));

  if(aNodes==NULL){
    fprintf(stderr, "malloc() failed in ortc check if got all parts\n");
    exit(EXIT_FAILURE);
  }

  for(i = 1; i<= iMessageTotal; i++){
    ortc_dnode *t = _ortc_dlist_searchEx(context->multiparts, messageId, i);
    if(t != NULL) {
      aNodes[(i-1)] = t;
    } else {
      free(aNodes);
      return;    
    }
  }
  size = 0;
  for(i = 0; i< iMessageTotal; i++){
    size += strlen(aNodes[i]->val2);
  }
  ret = (char*)malloc(size+1);
  pos = 0;
  channel = (char*)strdup(aNodes[0]->val1);
  for(i = 0; i< iMessageTotal; i++){
    size_t len = strlen(aNodes[i]->val2);
    memcpy(ret + pos, aNodes[i]->val2, len);
    pos += len;
    _ortc_dlist_deleteEx(context->multiparts, messageId, i+1);
  }
  ret[size] = '\0';
  _ortc_fire_onMessage(context, channel, ret);
  free(channel);
  free(ret);
  free(aNodes);
}

void _ortc_parse_message(ortc_context *context, char *message){
  char *messageId, *messageCount, *messageTotal, *messagePart, *channelNameStr, *messageStr, *params, *permissionsStr, *exceptionStr, *validateString, *operationType;
  struct cap pmatch[3], pmatch2[5];
  int iMessageTotal, wsSock, opt;
  size_t len, hbLen;
  ortc_dnode *ch;
  char hbStr[24];
  
  if(message[0] == 'a') {
    if (slre_match(&context->reMessage, message, (int)strlen(message), pmatch)) {         //is message   
      channelNameStr = _ortc_get_from_slre(1, pmatch);
      messageStr = _ortc_get_from_slre(2, pmatch);
      if(slre_match(&context->reMultipart, messageStr, (int)strlen(messageStr), pmatch2)){
		messageId =    _ortc_get_from_slre(1, pmatch2);
		messageCount = _ortc_get_from_slre(2, pmatch2);
		messageTotal = _ortc_get_from_slre(3, pmatch2);
		messagePart =  _ortc_get_from_slre(4, pmatch2);
		iMessageTotal = atoi(messageTotal);
		if(iMessageTotal > 1){ //multipart message
			_ortc_dlist_insert(context->multiparts, messageId, channelNameStr, messagePart, atoi(messageCount), NULL);
		_ortc_check_if_got_all_parts(context, messageId, iMessageTotal);
		} else {
			_ortc_fire_onMessage(context, channelNameStr, messagePart);
		}
		free(messageId);
		free(messageCount);
		free(messageTotal);
		free(messagePart);
      } else {
        _ortc_fire_onMessage(context, channelNameStr, messageStr);
      }
      free(channelNameStr);
      free(messageStr);
    } else if (slre_match(&context->reOperation, message, (int)strlen(message), pmatch)) {
      params = _ortc_get_from_slre(2, pmatch);
      operationType = _ortc_get_from_slre(1, pmatch);
      if(strncmp(operationType, "ortc-validated", 14)==0){
		if(slre_match(&context->rePermissions, params, (int)strlen(params), pmatch2)){
			permissionsStr = _ortc_get_from_slre(1, pmatch2);
			_ortc_save_permissions(context, permissionsStr);
			free(permissionsStr);
		}
		_ortc_change_state(context, CONNECTED);
		
      } else if(strncmp(operationType, "ortc-subscribed", 15)==0){
	if(slre_match(&context->reChannel, params, (int)strlen(params), pmatch2)){
	  channelNameStr = _ortc_get_from_slre(1, pmatch2);
	  ch = _ortc_dlist_search(context->channels, channelNameStr);
	  if(ch != NULL)
	    ch->num += 2; //isSubscribed
	  if(context->onSubscribed != NULL)
	    context->onSubscribed(context, channelNameStr);	  
	  free(channelNameStr);
	}
      } else if(strncmp(operationType, "ortc-unsubscribed", 17)==0){
	if(slre_match(&context->reChannel, params, (int)strlen(params), pmatch2)){
	  channelNameStr = _ortc_get_from_slre(1, pmatch2);
	  _ortc_dlist_delete(context->channels, channelNameStr);
	  if(context->onUnsubscribed != NULL)
	    context->onUnsubscribed(context, channelNameStr);
	  free(channelNameStr);
	}
      } else if(strncmp(operationType, "ortc-error", 10)==0){
	if(slre_match(&context->reException, params, (int)strlen(params), pmatch2)){
		_ortc_cancel_connecting(context);
		exceptionStr = _ortc_get_from_slre(1, pmatch2);
		_ortc_exception(context, exceptionStr);
		free(exceptionStr);
	}
      }
      free(params);
      free(operationType);
    }

  } else if(message[0] == 'o' && strlen(message)==1){
    wsSock = libwebsocket_get_socket_fd(context->wsi);
    opt = ORTC_SNDBUF_SIZE;
    setsockopt(wsSock, SOL_SOCKET, SO_SNDBUF, (const char*)&opt, sizeof(opt));
	
	
	if(context->heartbeatActive){
		snprintf(hbStr, sizeof(hbStr), "%d;%d;", context->heartbeatTime, context->heartbeatFails);
		hbLen = strlen(hbStr);
	} else {
		hbLen = 0;
	}
    len = 17 + strlen(context->appKey) +  strlen(context->authToken) + strlen(context->announcementSubChannel) + strlen(context->sessionId) + strlen(context->metadata) + hbLen;
	
    validateString = (char*)malloc(len+1);
    if(validateString == NULL){
      _ortc_exception(context, (char*)"malloc() failed in ortc parese message");
      return;
    }
	if(context->heartbeatActive)
		snprintf(validateString, len, "\"validate;%s;%s;%s;%s;%s;%s\"", context->appKey, context->authToken, context->announcementSubChannel, context->sessionId, context->metadata, hbStr);
	else
		snprintf(validateString, len, "\"validate;%s;%s;%s;%s;%s;\"", context->appKey, context->authToken, context->announcementSubChannel, context->sessionId, context->metadata);
    _ortc_send_command(context, validateString);
	
  } else if(strncmp(message, "c[1000,\"Normal closure\"]", 20)==0){
	_ortc_exception(context, (char*)"Server is about to close the websocket!");
  }
}

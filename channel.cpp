#include "channel.h"
#include <string.h>
#include "common.h"
#include "connection.h"

void _ortc_save_permissions(ortc_context *context, char *permissions){
  int offset = 0;
  struct slre rePerm; 
  //size_t     nmatch = 3;
  struct cap pmatch[3];
  char *channel, *hash;

  _ortc_dlist_clear(context->permissions);
  if(strcmp(permissions, "null")!=0){

    if (0 == slre_compile(&rePerm, "\\\\\"(.[^\\\\\"]*)\\\\\":\\\\\"(.[^\\\\\"]*)\\\\\"")) {
      _ortc_exception(context, (char*)"regcomp() failed, returning nonzero in ortc save permissions");
      return;
    }

    while(1){
      if(slre_match(&rePerm, permissions + offset, (int)strlen(permissions) - offset, pmatch)){
        channel = _ortc_get_from_slre(1, pmatch);
        hash = _ortc_get_from_slre(2, pmatch);
        _ortc_dlist_insert(context->permissions, channel, hash, NULL, 0, NULL);
        offset += pmatch[0].len;
        free(channel);
        free(hash);
      } else {
        break;
      }
    }
  }
}

void _ortc_subscribeOnReconnected(ortc_context *context){
  ortc_dnode *ptr = (ortc_dnode*)context->channels->first;
  ortc_dnode *t;

  while(ptr != NULL){
    if(ptr->num < 3){
      t = ptr;
      ptr = (ortc_dnode*)ptr->next;
      _ortc_dlist_delete(context->channels, t->id);
    } else {
      _ortc_subscribe(context, ptr->id, 1, 0, (void(*)(ortc_context*, char*, char*))ptr->callback);
      ptr = (ortc_dnode*)ptr->next;
    }
  }
}

char* _ortc_get_channel_permission(ortc_context *context, char* channel){
  ortc_dnode *ch = (ortc_dnode*)_ortc_dlist_searchWild(context->permissions, channel);

  if(ch!=NULL)
    return ch->val1;
  else
    return (char*)"";
}

void _ortc_subscribe(ortc_context* context, char* channel, int subscribeOnReconnected, int toBeSaved, void (*onMessage)(ortc_context*, char*, char*)){
  char *hash = _ortc_get_channel_permission(context, channel);
  size_t len = 16 + strlen(context->appKey) + strlen(context->authToken) + strlen(channel) + strlen(hash);
  char *subscribeCommand = (char*)malloc(len + 1);

  if(subscribeCommand == NULL){
    _ortc_exception(context,  (char*)"malloc() failed in ortc subscribe");
    return;
  }
  subscribeOnReconnected = (subscribeOnReconnected>0)?1:0;
  if(toBeSaved)
    _ortc_dlist_insert(context->channels, channel, NULL, NULL, subscribeOnReconnected, (void(*)(void*, char*, char*))onMessage);
  snprintf(subscribeCommand, len, "\"subscribe;%s;%s;%s;%s\"", context->appKey, context->authToken, channel, hash);
  _ortc_send_command(context, subscribeCommand);
}

void _ortc_unsubscribe(ortc_context* context, char* channel){
  size_t len = 16 + strlen(context->appKey) + strlen(channel);
  char *unsubscribeCommand = (char*)malloc(len + 1);
  ortc_dnode *ch;

  if(unsubscribeCommand == NULL){
    _ortc_exception(context,  (char*)"malloc() failed in ortc unsubscribe");
    return;
  }
  ch = _ortc_dlist_search(context->channels, channel);
  if(ch != NULL)
    ch->num = -1; //is trying to unsbscribed
  snprintf(unsubscribeCommand, len, "\"unsubscribe;%s;%s\"", context->appKey, channel);

  _ortc_send_command(context, unsubscribeCommand);
}

int _ortc_is_subscribing(ortc_context *context, char* channel){
  ortc_dnode *t = _ortc_dlist_search(context->channels, channel);

  if(t){
    if(t->num==0 || t->num==1)
      return 1;
  }
  return 0;
}

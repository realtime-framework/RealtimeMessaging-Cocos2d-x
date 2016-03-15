//
//  OrtcClient.cpp
//  cocos2d_libs
//
//  Created by João Caixinha on 19/10/15.
//
//

#include "OrtcClient.h"

//------- ortc standart delegate ---------

void onMessage(ortc_context *ortc, char *channel, char *message){
    ortcClient->delegate->onMessage(ortcClient, new std::string(message), new std::string(channel));
}

void onConnect(ortc_context* ortc){
    ortcClient->delegate->onConnect(ortcClient);
}

void onDisconnect(ortc_context* ortc){
    ortcClient->delegate->onDisconnect(ortcClient);
}

void onSubscribe(ortc_context* ortc, char* channel){
    ortcClient->delegate->onSubscribe(ortcClient, new std::string(channel));
}

void onUnsubscribe(ortc_context* ortc, char* channel){
    ortcClient->delegate->onUnsubscribe(ortcClient, new std::string(channel));
}

void onException(ortc_context* ortc, char* error){
    ortcClient->delegate->onException(ortcClient, new std::string(error));
}

void onReconnect(ortc_context* ortc){
    ortcClient->delegate->onReconnect(ortcClient);
}

void onReconnecting(ortc_context* ortc){
    ortcClient->delegate->onReconnecting(ortcClient);
}




//------- presence delegate ---------

void onEnable_presence(ortc_context *context, char* channel, char* error, char* result){
    if (ortcClient->presenceDelegate) {
        ortcClient->presenceDelegate->onEnable_presence(ortcClient, new std::string(channel), new std::string(error), new std::string(result));
    }
}



void onDisable_presence(ortc_context *context, char* channel, char* error, char* result){
    if (ortcClient->presenceDelegate) {
        ortcClient->presenceDelegate->onDisable_presence(ortcClient, new std::string(channel), new std::string(error), new std::string(result));
    }
}



void onPresence(ortc_context *context, char* channel, char* error, ortc_presenceData* result){
    if (ortcClient->presenceDelegate) {
        ortcClient->presenceDelegate->onPresence(ortcClient, new std::string(channel), new std::string(error), result);
    }
}







//------- authentication delegate ---------

void onSave_authentication(ortc_context *context, char* error, char* result){
    if (ortcClient->authenticationDelegate) {
        ortcClient->authenticationDelegate->onSave_authentication(ortcClient, new std::string(error), new std::string(result));
    }
}




OrtcClient::OrtcClient(OrtcClientDelegate* pDelegate){
    this->delegate = pDelegate;
    context = ortc_create_context();
    ortc_set_onConnected(context, onConnect);
    ortc_set_onDisconnected(context, onDisconnect);
    ortc_set_onSubscribed(context, onSubscribe);
    ortc_set_onUnsubscribed(context, onUnsubscribe);
    ortc_set_onReconnected(context, onReconnect);
    ortc_set_onReconnecting(context, onReconnecting);
    ortc_set_onException(context, onException);
    ortcClient = this;
}


void OrtcClient::connect(std::string* appkey, std::string* token){
    ortc_connect(context, (char*)appkey->c_str(), (char*)token->c_str());
}

void OrtcClient::disconnect(){
    ortc_disconnect(context);
}

void OrtcClient::subscribe(std::string* channel, int subscribeOnReconnected){
    ortc_subscribe(context, (char*)channel->c_str(), subscribeOnReconnected, onMessage);
}

void OrtcClient::unsubscribe(std::string* channel){
    ortc_unsubscribe(context, (char*)channel->c_str());
}

void OrtcClient::sendMessage(std::string* channel, std::string* message){
    ortc_send(context, (char *)channel->c_str(), (char *)message->c_str());
}

void OrtcClient::setCluster(std::string* clusterUrl){
    ortc_set_cluster(context, (char*)clusterUrl->c_str());
}


std::string* OrtcClient::getCluster(){
    return new std::string(ortc_get_cluster(context));
}

void OrtcClient::setURL(std::string* url){
    ortc_set_url(context, (char*)url->c_str());
}

std::string* OrtcClient::getURL(){
    return new std::string(ortc_get_url(context));
}

void OrtcClient::setConnectionMetadata(std::string* connection_metadata){
    ortc_set_connection_metadata(context, (char*) connection_metadata->c_str());
}

std::string* OrtcClient::get_connection_metadata(){
    return new std::string(ortc_get_connection_metadata(context));
}

void OrtcClient::set_announcementSubChannel(std::string* announcementSubChannel){
    ortc_set_announcementSubChannel(context, (char*)announcementSubChannel->c_str());
}

std::string* OrtcClient::get_announcementSubChannel(){
    return new std::string(ortc_get_announcementSubChannel(context));
}

std::string* OrtcClient::get_sessionId(){
    return new std::string(ortc_get_sessionId(context));
}

int OrtcClient::is_connected(){
    return ortc_is_connected(context);
}

int OrtcClient::is_subscribed(std::string* channel){
    return ortc_is_subscribed(context, (char*)channel->c_str());
}



void OrtcClient::enable_presence(std::string* privateKey,
                                 std::string* channel, int metadata){
    ortc_enable_presence(context, (char*)privateKey->c_str(), (char*)channel->c_str(), metadata, onEnable_presence);
}

void OrtcClient::enable_presence_ex(std::string* url, int isCluster,
                        std::string* appKey, std::string* privateKey, std::string* channel, int metadata,
                        void (*callback)(ortc_context*, char*, char*, char*)){
    ortc_enable_presence_ex(context, (char*)url->c_str(), isCluster, (char*)appKey->c_str(), (char*)privateKey->c_str(), (char*)channel->c_str(), metadata, onEnable_presence);

}

void OrtcClient::disable_presence(std::string* privateKey,
                      std::string* channel, void (*callback)(ortc_context*, char*, char*, char*)){
    ortc_disable_presence(context, (char*)privateKey->c_str(), (char*)channel->c_str(), onDisable_presence);

}

void OrtcClient::disable_presence_ex(std::string* url, int isCluster,
                         std::string* appKey, std::string* privateKey, std::string* channel,
                         void (*callback)(ortc_context*, char*, char*, char*)){
    ortc_disable_presence_ex(context, (char*)url->c_str(), isCluster, (char*)appKey->c_str(), (char*)privateKey->c_str(), (char*)channel->c_str(), onDisable_presence);

}

void OrtcClient::presence(std::string* channel,
              void (*callback)(ortc_context*, char*, char*, ortc_presenceData*)){
    ortc_presence(context, (char*)channel, onPresence);
}

void OrtcClient::presence_ex(std::string* url,
                 int isCluster, std::string* appKey, std::string* authToken, std::string* channel,
                 void (*callback)(ortc_context*, char*, char*, ortc_presenceData*)){
    ortc_presence_ex(context, (char*)url->c_str(), isCluster, (char*)appKey->c_str(), (char*)authToken->c_str(), (char*)channel->c_str(), onPresence);
}





void OrtcClient::save_authentication(
                         std::string *authToken, int isPrivate, int ttl, std::string *privateKey,
                         ortc_channelPermissions *permissions, int sizeOfChannelPermissions){
    ortc_save_authentication(context, (char*)authToken, isPrivate, ttl, (char*)privateKey, permissions, sizeOfChannelPermissions, onSave_authentication);
}

void OrtcClient::save_authentication_ex( std::string* url,
                                 int isCluster, std::string* authToken, int isPrivate, std::string* appKey,
                                 int ttl, std::string *privateKey, ortc_channelPermissions *permissions,
                                 int sizeOfChannelPermissions){
    ortc_save_authentication_ex(context, (char*)url->c_str(), isCluster, (char*)authToken->c_str(), isPrivate, (char*)appKey->c_str(), ttl, (char*)privateKey->c_str(), permissions, sizeOfChannelPermissions, onSave_authentication);
}



void OrtcClient::disable_ca_verification(){
    ortc_disable_ca_verification(context);
}

void OrtcClient::enable_ca_verification(){
    ortc_enable_ca_verification(context);
}

int OrtcClient::getHeartbeatActive(){
    return ortc_getHeartbeatActive(context);
}

int OrtcClient::getHeartbeatFails(){
    return ortc_getHeartbeatFails(context);
}

int OrtcClient::getHeartbeatTime(){
    return ortc_getHeartbeatTime(context);
}

void OrtcClient::setHeartbeatActive(int isActive){
    ortc_setHeartbeatActive(context, isActive);
}

void OrtcClient::setHeartbeatFails(int newHeartbeatFails){
    ortc_setHeartbeatFails(context, newHeartbeatFails);
}

void OrtcClient::setHeartbeatTime(int newHeartbeatTime){
    ortc_setHeartbeatTime(context, newHeartbeatTime);
}

std::string* OrtcClient::getVersion(){
    return new std::string(ortc_getVersion());
}

std::string* OrtcClient::getVersionVerbose(){
    return new std::string(ortc_getVersionVerbose());
}





























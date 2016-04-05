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
    std::string *msg = new std::string(message);
    std::string *ch = new std::string(channel);
    ortcClient->delegate->onMessage(ortcClient, msg, ch);
    delete msg;
    delete ch;
}

void onConnect(ortc_context* ortc){
    ortcClient->delegate->onConnect(ortcClient);
}

void onDisconnect(ortc_context* ortc){
    ortcClient->delegate->onDisconnect(ortcClient);
}

void onSubscribe(ortc_context* ortc, char* channel){
    std::string *ch = new std::string(channel);
    ortcClient->delegate->onSubscribe(ortcClient, ch);
    delete ch;
}

void onUnsubscribe(ortc_context* ortc, char* channel){
    std::string *ch = new std::string(channel);
    ortcClient->delegate->onUnsubscribe(ortcClient, ch);
    delete ch;
}

void onException(ortc_context* ortc, char* error){
    std::string *err = new std::string(error);
    ortcClient->delegate->onException(ortcClient, err);
    delete err;
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
        std::string* ch = new std::string(channel);
        std::string* err = new std::string(error);
        std::string* res = new std::string(result);
        ortcClient->presenceDelegate->onEnable_presence(ortcClient, ch, err, res);
        delete ch;
        delete err;
        delete res;
    }
}



void onDisable_presence(ortc_context *context, char* channel, char* error, char* result){
    if (ortcClient->presenceDelegate) {
        std::string* ch = new std::string(channel);
        std::string* err = new std::string(error);
        std::string* res = new std::string(result);
        ortcClient->presenceDelegate->onDisable_presence(ortcClient, new std::string(channel), new std::string(error), new std::string(result));
        delete ch;
        delete err;
        delete res;
    }
}



void onPresence(ortc_context *context, char* channel, char* error, ortc_presenceData* result){
    if (ortcClient->presenceDelegate) {
        std::string* ch = new std::string(channel);
        std::string* err = new std::string(error);
        ortcClient->presenceDelegate->onPresence(ortcClient, new std::string(channel), new std::string(error), result);
        delete ch;
        delete err;
    }
}




//------- authentication delegate ---------

void onSave_authentication(ortc_context *context, char* error, char* result){
    if (ortcClient->authenticationDelegate) {
        std::string* err = new std::string(error);
        std::string* res = new std::string(result);
        ortcClient->authenticationDelegate->onSave_authentication(ortcClient, new std::string(error), new std::string(result));
        delete err;
        delete res;
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
    const std::string::size_type size = appkey->size();
    char *appkeybuffer = new char[size + 1];
    memcpy(appkeybuffer, appkey->c_str(), size + 1);
    
    const std::string::size_type tokenbuffersize = token->size();
    char *tokenbuffer = new char[tokenbuffersize + 1];
    memcpy(tokenbuffer, token->c_str(), tokenbuffersize + 1);
    
    ortc_connect(context, (char*)appkeybuffer, (char*)tokenbuffer);
    delete appkey;
    delete token;
}

void OrtcClient::disconnect(){
    ortc_disconnect(context);
}

void OrtcClient::subscribe(std::string* channel, int subscribeOnReconnected){
    ortc_subscribe(context, (char*)channel->c_str(), subscribeOnReconnected, onMessage);
    delete channel;
}

void OrtcClient::unsubscribe(std::string* channel){
    ortc_unsubscribe(context, (char*)channel->c_str());
    delete channel;
}

void OrtcClient::sendMessage(std::string* channel, std::string* message){
    ortc_send(context, (char *)channel->c_str(), (char *)message->c_str());
    delete channel;
    delete message;
}

void OrtcClient::setCluster(std::string* clusterUrl){
    const std::string::size_type size = clusterUrl->size();
    char *clusterUrlbuffer = new char[size + 1];
    memcpy(clusterUrlbuffer, clusterUrl->c_str(), size + 1);
    ortc_set_cluster(context, (char*)clusterUrlbuffer);
    delete clusterUrl;
}


std::string* OrtcClient::getCluster(){
    return new std::string(ortc_get_cluster(context));
}

void OrtcClient::setURL(std::string* url){
    const std::string::size_type size = url->size();
    char *urlbuffer = new char[size + 1];
    memcpy(urlbuffer, url->c_str(), size + 1);
    ortc_set_url(context, (char*)urlbuffer);
    delete url;
}

std::string* OrtcClient::getURL(){
    return new std::string(ortc_get_url(context));
}

void OrtcClient::setConnectionMetadata(std::string* connection_metadata){
    const std::string::size_type size = connection_metadata->size();
    char *connection_metadatabuffer = new char[size + 1];
    memcpy(connection_metadatabuffer, connection_metadata->c_str(), size + 1);
    ortc_set_connection_metadata(context, (char*) connection_metadatabuffer);
    delete connection_metadata;
}

std::string* OrtcClient::get_connection_metadata(){
    return new std::string(ortc_get_connection_metadata(context));
}

void OrtcClient::set_announcementSubChannel(std::string* announcementSubChannel){
    const std::string::size_type size = announcementSubChannel->size();
    char *announcementSubChannelbuffer = new char[size + 1];
    memcpy(announcementSubChannelbuffer, announcementSubChannel->c_str(), size + 1);
    ortc_set_announcementSubChannel(context, (char*)announcementSubChannelbuffer);
    delete announcementSubChannel;
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
    const std::string::size_type size = channel->size();
    char *channelbuffer = new char[size + 1];
    memcpy(channelbuffer, channel->c_str(), size + 1);
    return ortc_is_subscribed(context, (char*)channelbuffer);
    delete channel;
}

void OrtcClient::enable_presence(std::string* privateKey,
                                 std::string* channel, int metadata){
    const std::string::size_type size = privateKey->size();
    char *privateKeybuffer = new char[size + 1];
    memcpy(privateKeybuffer, privateKey->c_str(), size + 1);
    
    const std::string::size_type size2 = channel->size();
    char *channelbuffer = new char[size2 + 1];
    memcpy(channelbuffer, channel->c_str(), size2 + 1);
    
    ortc_enable_presence(context, (char*)privateKeybuffer, (char*)channelbuffer, metadata, onEnable_presence);
    delete privateKey;
    delete channel;
}

void OrtcClient::enable_presence_ex(std::string* url, int isCluster,
                                    std::string* appKey, std::string* privateKey, std::string* channel, int metadata,
                                    void (*callback)(ortc_context*, char*, char*, char*)){
    const std::string::size_type size = url->size();
    char *urlbuffer = new char[size + 1];
    memcpy(urlbuffer, url->c_str(), size + 1);
    
    const std::string::size_type size2 = appKey->size();
    char *appKeybuffer = new char[size2 + 1];
    memcpy(appKeybuffer, appKey->c_str(), size2 + 1);
    
    const std::string::size_type size3 = privateKey->size();
    char *privateKeybuffer = new char[size3 + 1];
    memcpy(privateKeybuffer, privateKey->c_str(), size3 + 1);
    
    const std::string::size_type size4 = channel->size();
    char *channelbuffer = new char[size4 + 1];
    memcpy(channelbuffer, channel->c_str(), size4 + 1);
    
    ortc_enable_presence_ex(context, (char*)urlbuffer, isCluster, (char*)appKeybuffer, (char*)privateKeybuffer, (char*)channelbuffer, metadata, onEnable_presence);
    delete url;
    delete privateKey;
    delete appKey;
    delete channel;
}

void OrtcClient::disable_presence(std::string* privateKey,
                                  std::string* channel, void (*callback)(ortc_context*, char*, char*, char*)){
    const std::string::size_type size3 = privateKey->size();
    char *privateKeybuffer = new char[size3 + 1];
    memcpy(privateKeybuffer, privateKey->c_str(), size3 + 1);
    
    const std::string::size_type size4 = channel->size();
    char *channelbuffer = new char[size4 + 1];
    memcpy(channelbuffer, channel->c_str(), size4 + 1);
    ortc_disable_presence(context, (char*)privateKeybuffer, (char*)channelbuffer, onDisable_presence);
    delete privateKey;
    delete channel;
    
}

void OrtcClient::disable_presence_ex(std::string* url, int isCluster,
                                     std::string* appKey, std::string* privateKey, std::string* channel,
                                     void (*callback)(ortc_context*, char*, char*, char*)){
    const std::string::size_type size = url->size();
    char *urlbuffer = new char[size + 1];
    memcpy(urlbuffer, url->c_str(), size + 1);
    
    const std::string::size_type size2 = appKey->size();
    char *appKeybuffer = new char[size2 + 1];
    memcpy(appKeybuffer, appKey->c_str(), size2 + 1);
    
    const std::string::size_type size3 = privateKey->size();
    char *privateKeybuffer = new char[size3 + 1];
    memcpy(privateKeybuffer, privateKey->c_str(), size3 + 1);
    
    const std::string::size_type size4 = channel->size();
    char *channelbuffer = new char[size4 + 1];
    memcpy(channelbuffer, channel->c_str(), size4 + 1);
    ortc_disable_presence_ex(context, (char*)urlbuffer, isCluster, (char*)appKeybuffer, (char*)privateKeybuffer, (char*)channelbuffer, onDisable_presence);
    delete url;
    delete privateKey;
    delete appKey;
    delete channel;
}

void OrtcClient::presence(std::string* channel,
                          void (*callback)(ortc_context*, char*, char*, ortc_presenceData*)){
    const std::string::size_type size4 = channel->size();
    char *channelbuffer = new char[size4 + 1];
    memcpy(channelbuffer, channel->c_str(), size4 + 1);
    ortc_presence(context, (char*)channel, onPresence);
    delete channel;
}

void OrtcClient::presence_ex(std::string* url,
                             int isCluster, std::string* appKey, std::string* authToken, std::string* channel,
                             void (*callback)(ortc_context*, char*, char*, ortc_presenceData*)){
    const std::string::size_type size = url->size();
    char *urlbuffer = new char[size + 1];
    memcpy(urlbuffer, url->c_str(), size + 1);
    
    const std::string::size_type size2 = appKey->size();
    char *appKeybuffer = new char[size2 + 1];
    memcpy(appKeybuffer, appKey->c_str(), size2 + 1);
    
    const std::string::size_type size3 = authToken->size();
    char *authTokenbuffer = new char[size3 + 1];
    memcpy(authTokenbuffer, authToken->c_str(), size3 + 1);
    
    const std::string::size_type size4 = channel->size();
    char *channelbuffer = new char[size4 + 1];
    memcpy(channelbuffer, channel->c_str(), size4 + 1);
    ortc_presence_ex(context, (char*)urlbuffer, isCluster, (char*)appKeybuffer, (char*)authTokenbuffer, (char*)channelbuffer, onPresence);
    delete url;
    delete authToken;
    delete appKey;
    delete channel;
}



void OrtcClient::save_authentication(
                                     std::string *authToken, int isPrivate, int ttl, std::string *privateKey,
                                     ortc_channelPermissions *permissions, int sizeOfChannelPermissions){
    const std::string::size_type size = privateKey->size();
    char *privateKeybuffer = new char[size + 1];
    memcpy(privateKeybuffer, privateKey->c_str(), size + 1);
    
    const std::string::size_type size3 = authToken->size();
    char *authTokenbuffer = new char[size3 + 1];
    memcpy(authTokenbuffer, authToken->c_str(), size3 + 1);
    
    ortc_save_authentication(context, (char*)authTokenbuffer, isPrivate, ttl, (char*)privateKeybuffer, permissions, sizeOfChannelPermissions, onSave_authentication);
    delete authToken;
    delete privateKey;
}

void OrtcClient::save_authentication_ex( std::string* url,
                                        int isCluster, std::string* authToken, int isPrivate, std::string* appKey,
                                        int ttl, std::string *privateKey, ortc_channelPermissions *permissions,
                                        int sizeOfChannelPermissions){
    const std::string::size_type size = url->size();
    char *urlbuffer = new char[size + 1];
    memcpy(urlbuffer, url->c_str(), size + 1);
    
    const std::string::size_type size2 = appKey->size();
    char *appKeybuffer = new char[size2 + 1];
    memcpy(appKeybuffer, appKey->c_str(), size2 + 1);
    
    const std::string::size_type size3 = privateKey->size();
    char *privateKeybuffer = new char[size3 + 1];
    memcpy(privateKeybuffer, privateKey->c_str(), size3 + 1);
    
    const std::string::size_type size4 = authToken->size();
    char *authTokenbuffer = new char[size4 + 1];
    memcpy(authTokenbuffer, authToken->c_str(), size4 + 1);
    
    ortc_save_authentication_ex(context, (char*)urlbuffer, isCluster, (char*)authTokenbuffer, isPrivate, (char*)appKeybuffer, ttl, (char*)privateKeybuffer, permissions, sizeOfChannelPermissions, onSave_authentication);
    delete url;
    delete authToken;
    delete appKey;
    delete privateKey;
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





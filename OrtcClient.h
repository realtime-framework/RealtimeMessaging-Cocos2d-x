//
//  OrtcClient.hpp
//  cocos2d_libs
//
//  Created by João Caixinha on 19/10/15.
//
//

#ifndef OrtcClient_hpp
#define OrtcClient_hpp

#include <stdio.h>
#include <string>
#include "libortc.h"

using namespace std;

class OrtcClientDelegate;
class OrtcClientPresenceDelegate;
class OrtcClientAuthenticationDelegate;

using namespace std;
class OrtcClient{
public:
    OrtcClientDelegate* delegate;
    OrtcClientPresenceDelegate* presenceDelegate;
    OrtcClientAuthenticationDelegate* authenticationDelegate;
    
    OrtcClient(OrtcClientDelegate* pDelegate);
    
    void connect(std::string* appkey, std::string* token);
    void disconnect();
    void subscribe(std::string* channel, int subscribeOnReconnected);
    void unsubscribe(std::string* channel);
    void sendMessage(std::string* channel, std::string* message);
    
    void setCluster(std::string* clusterUrl);
    std::string* getCluster();
    void setURL(std::string* url);
    std::string* getURL();
    void setConnectionMetadata(std::string* connection_metadata);
    std::string* get_connection_metadata();
    void set_announcementSubChannel(std::string* announcementSubChannel);
    std::string* get_announcementSubChannel();
    std::string* get_sessionId();
    int is_connected();
    int is_subscribed(std::string* channel);
    void enable_presence(std::string* privateKey,
                                         std::string* channel, int metadata);
    void enable_presence_ex(std::string* url, int isCluster,
                                            std::string* appKey, std::string* privateKey, std::string* channel, int metadata,
                                            void (*callback)(ortc_context*, char*, char*, char*));
    void disable_presence(std::string* privateKey,
                                          std::string* channel, void (*callback)(ortc_context*, char*, char*, char*));
    void disable_presence_ex(std::string* url, int isCluster,
                                             std::string* appKey, std::string* privateKey, std::string* channel,
                                             void (*callback)(ortc_context*, char*, char*, char*));
    void presence(std::string* channel,
                  void (*callback)(ortc_context*, char*, char*, ortc_presenceData*));
    void presence_ex(std::string* url,
                                     int isCluster, std::string* appKey, std::string* authToken, std::string* channel,
                                     void (*callback)(ortc_context*, char*, char*, ortc_presenceData*));
    void save_authentication(
                                             std::string *authToken, int isPrivate, int ttl, std::string *privateKey,
                                             ortc_channelPermissions *permissions, int sizeOfChannelPermissions);
    void save_authentication_ex( std::string* url,
                                                int isCluster, std::string* authToken, int isPrivate, std::string* appKey,
                                                int ttl, std::string *privateKey, ortc_channelPermissions *permissions,
                                                int sizeOfChannelPermissions);
    
    int getHeartbeatActive();
    int getHeartbeatFails();
    int getHeartbeatTime();
    void setHeartbeatActive(int isActive);
    void setHeartbeatFails(int newHeartbeatFails);
    void setHeartbeatTime(int newHeartbeatTime);
    std::string* getVersion();
    std::string* getVersionVerbose();
    
private:
    ortc_context *context;
    void disable_ca_verification();
    void enable_ca_verification();
};


class OrtcClientDelegate{
public:
    virtual void onMessage(OrtcClient* ortc, std::string *message, std::string *channel)=0;
    virtual void onConnect(OrtcClient* ortc)=0;
    virtual void onDisconnect(OrtcClient* ortc)=0;
    virtual void onSubscribe(OrtcClient* ortc, std::string* channel)=0;
    virtual void onUnsubscribe(OrtcClient* ortc, std::string* channel)=0;
    virtual void onException(OrtcClient* ortc, std::string* error)=0;
    virtual void onReconnect(OrtcClient* ortc)=0;
    virtual void onReconnecting(OrtcClient* ortc)=0;
};


class OrtcClientPresenceDelegate{
public:
    virtual void onEnable_presence(OrtcClient *ortc, std::string* channel, std::string* error, std::string* result)=0;
    virtual void onDisable_presence(OrtcClient *ortc, std::string* channel, std::string* error, std::string* result)=0;
    virtual void onPresence(OrtcClient *ortc, std::string* channel, std::string* error, ortc_presenceData* result)=0;
};

class OrtcClientAuthenticationDelegate{
public:
    virtual void onSave_authentication(OrtcClient *ortc, std::string* error, std::string* result)=0;
};

#endif /* OrtcClient_hpp */

static OrtcClient* ortcClient;


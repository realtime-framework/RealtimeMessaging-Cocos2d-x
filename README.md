# Realtime Cloud Messaging Cocos2d-x SDK
Part of the [The Realtime® Framework](http://framework.realtime.co), Realtime Cloud Messaging (aka ORTC) is a secure, fast and highly scalable cloud-hosted Pub/Sub real-time message broker for web and mobile apps.

If your app has data that needs to be updated in the user’s interface as it changes (e.g. real-time stock quotes or ever changing social news feed) Realtime Cloud Messaging is the reliable, easy, unbelievably fast, “works everywhere” solution.

## Example
Checkout the ready-to-use example at [https://github.com/realtime-framework/RealtimeMessaging-Cocos2d-x-example](https://github.com/realtime-framework/RealtimeMessaging-Cocos2d-x-example)

---
## Cocos2d-x instalation

*	Download Cocos2dx from [cocos2d-x page](http://www.cocos2d-x.org/download).
*	Place the sdk in your system files, in the folder run `./setup.py` on terminal.
*	Create a new cocos2dx project by running the command `cocos new -l cpp -p [YOUR_PACKAGE_NAME] -d [PATH_TO_NEW_PROJECT]`.


## Setup the example

*	Copy the content of the example Classes folder to Classes folder in your project.
*	Copy the content of the example Resources folder to Resources folder in your project.

## Install cocos Realtime Messaging SDK

*	Copy cocosRealtimeMessaging SDK to cocos2d/extenssions.
*	Edit Android.mk file in cocos2d/extenssions and add the following lins under `LOCAL_SRC_FILES := \`:

		cocosRealtimeMessaging/OrtcClient.cpp \
		cocosRealtimeMessaging/common.cpp \
		cocosRealtimeMessaging/loops.cpp \
		cocosRealtimeMessaging/authentication.cpp \
		cocosRealtimeMessaging/connection.cpp \
		cocosRealtimeMessaging/message.cpp \
		cocosRealtimeMessaging/balancer.cpp \
		cocosRealtimeMessaging/dlist.cpp \
		cocosRealtimeMessaging/ortc.cpp \
		cocosRealtimeMessaging/channel.cpp \
		cocosRealtimeMessaging/events.cpp \
		cocosRealtimeMessaging/presence.cpp \
		cocosRealtimeMessaging/slre/slre.cpp \
			
*	add `LOCAL_STATIC_LIBRARIES += cocos_curl_static` to local static libraries entries in `Android.mk` if using cocos2dx version >= 3.13
*	In the last line add `$(call import-module,curl/prebuilt/android)`
*	If building for iOS, make sure in the XCode project the cocos2d_libs is using libcurl.a in link with binary libraries.


That's it, run the project.

---

## Interface OrtcClientDelegate
Handles ortc events:

#### On message received:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method
*	**message** Received message
*	**channel** Message channel

		void onMessage(OrtcClient* ortc, 
			std::string *message, 
			std::string *channel)
    
#### On connect:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method

		void onConnect(OrtcClient* ortc)
    
#### On disconnect:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method


		void onDisconnect(OrtcClient* ortc)
    
#### On channel subscribe:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method
*	**channel** Subscribed channel.


		void onSubscribe(OrtcClient* ortc, std::string* channel)

#### On channel unsubscribe:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method
*	**channel** Unsubscribed channel.

		void onUnsubscribe(OrtcClient* ortc, std::string* channel)
    
#### On ORTC exception:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method
*	**error** Error description

		void onException(OrtcClient* ortc, std::string* error)
    
#### On reconnect:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method

		void onReconnect(OrtcClient* ortc)
    
#### On reconnecting:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method

		void onReconnecting(OrtcClient* ortc)


## Interface OrtcClientPresenceDelegate
Handles ORTC presence events.

#### On enable presence response: 

Parameters

*	**ortc** OrtcClient object that invoked this delegate method
*	**channel** enabled channel
*	**error** Error description (if error exists)
*	**result** Request result

		void onEnable_presence(OrtcClient *ortc, 
			std::string* channel, 
			std::string* error, 
			std::string* result)

#### On disable presence response:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method
*	**channel** disabled channel
*	**error** Error description (if error exists)
*	**result** Request result

		void onDisable_presence(OrtcClient *ortc, 
			std::string* channel,
			std::string* error,
			std::string* result)

#### On presence request response:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method
*	**channel** channel
*	**error** Error description (if error exists)
*	**result** Request result

		void onPresence(OrtcClient *ortc, 
			std::string* channel,
			std::string* error,
			ortc_presenceData* result)


## Interface OrtcClientAuthenticationDelegate
Handles ORTC authentication events.

#### On save authentication request response:

Parameters

*	**ortc** OrtcClient object that invoked this delegate method
*	**error** Error description (if error exists)
*	**result** Request result


		void onSave_authentication(OrtcClient *ortc, 
			std::string* error,
			std::string* result)
    


## Class OrtcClient

#### connect
Connects to ORTC service using the supplied application key and authentication token.

Parameters

*	**applicationKey**	string containing the application key

*	**authenticationToken** string containing the authentication token 

When ortc connects the onConnect method in the ortcClientDelegate class is invoked.

	void connect(std::string* appkey, std::string* token);
	
#### disconnect
Disconnects from the ORTC service.

When ortc disconnects the onDisconnect method in the ortcClientDelegate class is invoked.

	void disconnect();
	
#### subscribe
Subscribes to the supplied channel to receive messages sent to it.

Parameters

*	**channel** string containing the channel name
*	**subscribeOnReconnected**	1 - Indicates whether subscription to the channel should be restated when reconnected (if it was previously subscribed when connected).

When ortc subscribes a channel the onSubscribe method in the ortcClientDelegate class is invoke.
The message and channel is receive on onMessage	 in the ortcClientDelegate class.

	void subscribe(std::string* channel, int subscribeOnReconnected);
	

	
#### unsubscribe
Unsubscribes from the supplied channel to stop receiving messages sent to it.

Parameters

*	**channel** string containing the channel name

When ortc unsubscribes a channel the onUnsubscribe method in the ortcClientDelegate class is invoke.
The message and channel is receive on onMessage	 in the ortcClientDelegate class.

	void unsubscribe(std::string* channel);
	
#### sendMessage
Sends the supplied message to the supplied channel.

Parameters

*	**channel**	 string containing the channel name
*	**message**	 string containing the message

		void sendMessage(std::string* channel, std::string* message);
	
#### setCluster
Sets a cluster url for ORTC connection.
You have to call this method or setUrl() before calling the connect() method.

Parameters

*	**cluster**	string containing url address

		void setCluster(std::string* clusterUrl);
	
#### getCluster
Gets the current cluster url for ORTC connection


*	**string** url address


		std::string* getCluster();
	
	
#### setURL
Sets a server url for the ORTC connection
You have to call this method or setCluster() before calling the connect() method.

Parameters

*	**url**	string containing url address

		void setURL(std::string* url);
	
#### getURL
Gets the current server url for ORTC connection.

Returns

*	**string** containing url address

		std::string* getURL();
	
#### setConnectionMetadata
Sets the connection metadata.

Parameters

*	**connection_metadata**	 string containing connection metadata

		void setConnectionMetadata(std::string* connection_metadata);
	
#### get_connection_metadata
Gets the current connection metadata.


Returns

*	**string** containing connection metadata

		std::string* get_connection_metadata();
	
#### set_announcementSubChannel
Sets the announcement sub-channel.

Parameters

*	**announcementSubChannel** string containing announcement subchannel

		void set_announcementSubChannel(std::string* announcementSubChannel);
	
#### get_announcementSubChannel
Gets the current connection announcement sub-channel.

Returns

*	**string** containing announcement sub-channel

		std::string* get_announcementSubChannel();
	
#### get_sessionId
Gets the current sessionId.

Returns

*	**string** containing sessionId

		std::string* get_sessionId();
	
#### is_connected
Indicates whether an ORTC connection is established.

Returns

*	**int** 1 when connection is established otherwise 0

		int is_connected();
	
#### is_subscribed
Indicates whether subscription is valid to the supplied channel.

Parameters

*	**channel**	string containing the channel name

Returns

*	**int** 1 when subscription to the supplied channel is valid otherwise 0

		int is_subscribed(std::string* channel);
	
#### enable_presence
Enables presence for the specified channel.
This method does not require previous connection.

Note: This method will send your Private Key over the Internet. Make sure to use secure connection.

Parameters

*	**url**	 string containing ORTC server URL
*	**isCluster**	indicates whether the ORTC server is in a cluster (1 - yes, 0 - no)
*	**appKey**	 string containing the application key
*	**privateKey**	 string containing private key provided when the ORTC service is purchased
*	**channel**	 string containing channel name
*	**metadata**	defines if to collect first 100 unique metadata (1 - yes, 0 - no)

The onEnable_presence method in the ortcClientPresenceDelegate class is invoked with channel, error and result parameters.

	void enable_presence(std::string* privateKey,
	                     std::string* channel, int metadata);
	                                     
#### disable_presence
Disables presence for the specified channel.
This method does not require previous connection.

Note: This method will send your Private Key over the Internet. Make sure to use secure connection.

Parameters

*	**url** string containing ORTC server URL
*	**isCluster**	indicates whether the ORTC server is in a cluster (1 - yes, 0 - no)
*	**appKey** string containing the application key
*	**privateKey** string containing private key provided when the ORTC service is purchased
*	**channel** string containing channel name

The onDisable_presence method in the ortcClientPresenceDelegate class is invoke with with channel, error and result parameters.

	void disable_presence(std::string* privateKey,
	                      std::string* channel);
	                                      
	                                         
#### presence
Gets a presence data indicating the subscriptions number in the specified channel and if active the first 100 unique metadata.
This method requires a previous connection.

Parameters

*	**channel** string containing channel name

The onPresence method in the ortcClientPresenceDelegate class is invoke with with channel, error and result parameters.

	void presence(std::string* channel);
	              
#### presence_ex
Gets a presence data indicating the subscriptions number in the specified channel and if active the first 100 unique metadata.
This method does not require previous connection.

Parameters

*	**url** string containing ORTC server URL
*	**isCluster**	indicates whether the ORTC server is in a cluster (1 - yes, 0 - no)
*	**appKey** string containing the application key
*	**authToken** string containing the authentication token
*	**channel** string containing channel name

The onPresence_ex method in the ortcClientPresenceDelegate class is invoke with with channel, error and result parameters.

	void presence_ex(std::string* url,
	                 int isCluster, 
	                 std::string* appKey, 
	                 std::string* authToken, 
	                 std::string* channel);
	                                 
#### save_authentication
Saves the channel and its permissions for the supplied application key and authentication token.
Note: This method will send your Private Key over the Internet. Make sure to use secure connection.

Parameters

*	**authToken** string containing the authentication token
*	**isPrivate**	indicates whether the authentication token is private (1 - yes, 0 - no)
*	**ttl**	the authentication token time to live (TTL), in other words, the allowed activity time (in seconds)
*	**privateKey** string containing private key provided when the ORTC service is purchased
*	**permissions**	pointer to array of structures (ortc_channelPermissions) containing channels and their permissions (read, write or presence)
*	**sizeOfChannelPermissions**	size of permissions array

The onSave_authentication method in the ortcClientPresenceDelegate class is invoke with with channel, error and result (ortc_presenceData) parameters.

	void save_authentication(std::string *authToken, 
		int isPrivate, 
		int ttl, 
		std::string *privateKey,
		ortc_channelPermissions *permissions,
		int sizeOfChannelPermissions);
	                                         
#### `save_authentication_ex`
Saves the channel and its permissions for the supplied application key and authentication token.
This method does not require previous connection.
Note: This method will send your Private Key over the Internet. Make sure to use secure connection.

Parameters

*	**url** string containing ORTC server URL
*	**isCluster**	indicates whether the ORTC server is in a cluster (1 - yes, 0 - no)
*	**authToken** string containing the authentication token
*	**isPrivate**	indicates whether the authentication token is private (1 - yes, 0 - no)
*	**appKey** string containing the application key
*	**ttl**	the authentication token time to live (TTL), in other words, the allowed activity time (in seconds)
*	**privateKey** string containing private key provided when the ORTC service is purchased
*	**permissions**	pointer to array of structures (ortc_channelPermissions) containing channels and their permissions (read, write or presence)
*	**sizeOfChannelPermissions**	size of permissions array

The onSave_authentication_ex method in the ortcClientPresenceDelegate class is invoke with with channel, error and result (ortc_presenceData) parameters.

	void save_authentication_ex( std::string* url,
	     int isCluster, 
	     std::string* authToken,
	     int isPrivate,
	     std::string* appKey,
	     int ttl,
	     std::string *privateKey,
	     ortc_channelPermissions *permissions,
	     int sizeOfChannelPermissions);

	
#### getHeartbeatActive
Checks if the client heartbeat is active

Returns

*	**int** 1 if active, 0 if not active.

		int getHeartbeatActive();
	
#### getHeartbeatFails
Gets the curent number of allowed heartbeat fails.

Returns

*	**int** heartbeats fails.

		int getHeartbeatFails();
	
#### getHeartbeatTime
Gets the current heartbeat interval.

Returns

*	**int** heartbeats interval.

		int getHeartbeatTime();
	
#### setHeartbeatActive
Sets client heartbeat active.

Parameters

*	**isActive** 1 for active, 0 to not active.

		void setHeartbeatActive(int isActive);
	
#### setHeartbeatFails
Sets the number of allowed heartbeat fails.

Parameters

*	**newHeartbeatFails** Number of allowed heartbeat fails.

		void setHeartbeatFails(int newHeartbeatFails);
	
#### setHeartbeatTime
Sets client heartbeat interval.

Parameters

*	**newHeartbeatTime** new interval.

		void setHeartbeatTime(int newHeartbeatTime);
	
#### getVersion
Returns the current SDK version.

*	**string** sdk version.

		std::string* getVersion();
	
#### getVersionVerbose
Returns the current SDK version (verbose mode).

*	**string** sdk version verbose.

		std::string* getVersionVerbose();
	
## Authors
Realtime.co

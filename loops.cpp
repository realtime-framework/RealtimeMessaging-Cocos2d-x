#include "libortc.h"
#include "loops.h"
#include "common.h"
#include "events.h"
#include <errno.h>

#include "libwebsockets.h"


int _ortc_finish_loops(ortc_context* context);
int _ortc_disconnecting(ortc_context* context);
int _ortc_run_communication_loop(ortc_context* context);
int _ortc_run_monotorization_loops(ortc_context* context);
int _ortc_run_reconnecting_loop(ortc_context* context);
int _ortc_run_reconnecting_loop(ortc_context* context);
int _ortc_prepare_websocket(ortc_context* context);
int _ortc_open_socket(ortc_context* context);
void _ortc_send_command(ortc_context* context, char *message);

int _ortc_getBalancer(char* url, char* appKey, int verifyCA, char** response);
int _ortc_parseUrl(char* url, char** host, int* port, int* useSSL);

int ortc_log = 0;

void _ortc_on_socket_closed(ortc_context* context){
	if(ortc_log) printf("ortc_log on socket stop, current state: %s\n", _ortc_decodeState(context->state));
}

void *_ortc_main_thread(void *ptr){
	ortc_context *context;
	enum _ortc_state newState;
	int toBeAlive = 1;
	struct timeval now;
	struct timespec timeout;
	int retcode;
	
	context = (ortc_context *) ptr;
	
	pthread_mutex_lock(&context->mutex_state);
	while(toBeAlive){
		if(context->state == OPENED){ //We don't want to stay in this state for more than ORTC_HANDSHAKE_TIMEOUT seconds
			gettimeofday(&now, NULL);
			timeout.tv_sec = now.tv_sec + ORTC_HANDSHAKE_TIMEOUT;
			timeout.tv_nsec = now.tv_usec * 1000;
			retcode = 0;
			
			retcode = pthread_cond_timedwait(&context->pcond, &context->mutex_state, &timeout);
			if(retcode == ETIMEDOUT){
				if(ortc_log) printf("ortc_log timeout for state OPENED\n");
				context->stateToBe = RECONNECTING;
			}
		} else {
			pthread_cond_wait(&context->pcond, &context->mutex_state);
		}

		newState = context->stateToBe;

		switch(context->state){
			case DISCONNECTED:
				if(newState == QUITTING){
					toBeAlive = 0;
					context->state = newState;
				} else if(newState == CONNECTING) {
					if(_ortc_run_connecting_loop(context) == 0) 
						context->state = newState;
				} else {
					if(ortc_log) printf("ortc_log illegal state change, from DISCONNECTED to: %d\n", newState);	
				}
			break;

			case CONNECTING:
				if(newState == QUITTING){
					toBeAlive = 0;
					context->state = newState;
					_ortc_finish_loops(context);
				} else if(newState == DISCONNECTING){
					if(_ortc_disconnecting(context)==0)
						context->state = newState;
				} else if(newState == OPENED){
					if(_ortc_run_communication_loop(context)==0){
						context->state = newState;							
					}
				} else {
					if(ortc_log) printf("ortc_log illegal state change, from CONNECTING to: %d\n", newState);
				}
			break;
			
			case OPENED:
				if(newState == QUITTING){
					toBeAlive = 0;
					context->state = newState;
					_ortc_finish_loops(context);
				} else if(newState == CONNECTED){
					if(_ortc_run_monotorization_loops(context)==0){
						context->state = newState;
						if(context->virginConnect){
							_ortc_fireOnConnected(context);
							context->virginConnect = 0;
						} else {
							_ortc_fireOnReconnected(context);
						}
					}
				} else if(newState == DISCONNECTING){
					if(_ortc_disconnecting(context)==0)
						context->state = newState;
				} else if(newState == RECONNECTING){
					if(_ortc_run_reconnecting_loop(context)==0)
						context->state = newState;
				} else {
					if(ortc_log) printf("ortc_log illegal state change, from OPENED to: %d\n", newState);
				}
			break;
			
			case CONNECTED:
				if(newState == QUITTING){
					toBeAlive = 0;
					context->state = newState;
					_ortc_finish_loops(context);
				} else if(newState == DISCONNECTING){
					if(_ortc_disconnecting(context)==0)
						context->state = newState;
				} else if(newState == RECONNECTING){
					if(_ortc_run_reconnecting_loop(context)==0)
						context->state = newState;
				} else {
					if(ortc_log) printf("ortc_log illegal state change, from CONNECTED to: %d\n", newState);
				}
			break;
			
			case DISCONNECTING:
				if(newState == QUITTING){
					toBeAlive = 0;
					context->state = newState;
				} else if(newState == DISCONNECTED) {
					context->state = newState;
					_ortc_fireOnDisconnected(context);
				} else { 
					if(ortc_log) printf("ortc_log illegal state change, from DISCONNECTING to: %d\n", newState);
				}
			break;

			case RECONNECTING:
				if(newState == QUITTING){
					toBeAlive = 0;
					context->state = newState;
					_ortc_finish_loops(context);
				} else if(newState == DISCONNECTING){
					if(_ortc_disconnecting(context)==0)
						context->state = newState;
				} else if(newState == OPENED){
					if(_ortc_run_communication_loop(context)==0){
						context->state = newState;							
					}
				} else {
					if(ortc_log) printf("ortc_log illegal state change, from RECONNECTING to: %d\n", newState);
				}			
			break;
                
            case QUITTING:
                break;
		}
	}
	
	pthread_mutex_unlock(&context->mutex_state);
	if(ortc_log) printf("ortc_log exiting main thread\n");
	return 0;
}

int _ortc_init_main_thread(ortc_context *context){
	int tret = pthread_create(&context->thrdMain, NULL, _ortc_main_thread, context);
	if(tret!=0){    
		_ortc_exception(context,  (char*)"Error creating reconnecting thread!");
		return -1;
	}
	return 0;
}

int _ortc_quit_main_thread(ortc_context *context){
	_ortc_change_state(context, QUITTING);
	pthread_join(context->thrdMain, NULL);
    return 0;
}

void _ortc_change_state(ortc_context* context, enum _ortc_state newState){
	pthread_mutex_lock(&context->mutex_state);
	if(ortc_log) printf("ortc_log new state request: %s\n", _ortc_decodeState(newState));
	context->stateToBe = newState;
	pthread_cond_signal(&context->pcond);
	pthread_mutex_unlock(&context->mutex_state);
	return;
}


void _ortc_init_connection(ortc_context* context){
	if(_ortc_prepare_websocket(context) < 0)
		return;
	context->virginConnect = 1;
	_ortc_change_state(context, CONNECTING);
}

int _ortc_finish_loops(ortc_context* context){
	if(context->loop_active_connecting){
		context->loop_active_connecting = 0;
		if(ortc_log) printf("ortc_log waiting for thread connecting\n");
		pthread_join(context->thrdConnecting, NULL);
	}

	if(context->loop_active_communication){
		context->loop_active_communication = 0;
		if(ortc_log) printf("ortc_log waiting for thread communication\n");
		pthread_join(context->thrdCommunication, NULL);
	}
	
	if(context->loop_active_throttle){
		context->loop_active_throttle = 0;
		if(ortc_log) printf("ortc_log waiting for thread throttle\n");
		pthread_join(context->thrdThrottle, NULL);		
	}
	
	if(context->loop_active_serverHeartbeat){
		context->loop_active_serverHeartbeat = 0;
		if(ortc_log) printf("ortc_log waiting for thread serverHeartbeat\n");
		pthread_join(context->thrdServerHeartbeat, NULL);
	}
	
	if(context->loop_active_clientHeartbeat){
		context->loop_active_clientHeartbeat = 0;
		if(ortc_log) printf("ortc_log waiting for thread clientHeartbeat\n");
		pthread_join(context->thrdClientHeartbeat, NULL);
	}
	
	if(context->loop_active_reconnecting){
		context->loop_active_reconnecting = 0;
		if(ortc_log) printf("ortc_log waiting for thread reconnecting\n");
		pthread_join(context->thrdReconnect, NULL);
	}
	if(ortc_log) printf("ortc_log all running threads are stopped\n");
	return 0;
}


// *************** LOOPS *********************

void *_ortc_loop_communication(void *ptr){
	ortc_context *context;
	int c = 0;
	
	if(ortc_log) printf("ortc_log communication loop started\n");
	context = (ortc_context *) ptr;
	while(context->loop_active_communication){
		if(context->lws_context)
			lws_service(context->lws_context, 10);
		if(c == 50){
			c = 0;
			if(context->lws_context && context->wsi)
				lws_callback_on_writable(context->wsi);
		}
		c++;
	}
	if(ortc_log) printf("ortc_log communication loop leaving\n");
	return 0;
}

void *_ortc_loop_connecting(void *ptr){
	ortc_context *context;
	int socketResult = -1, centiseconds = 0, tret;
	
	if(ortc_log) printf("ortc_log connecting loop started\n");
	context = (ortc_context *) ptr;
	while(context->loop_active_connecting && socketResult != 0) {
		if(centiseconds == 0){
			if(context->state != CONNECTING && context->state != DISCONNECTED){
				_ortc_fireOnReconnecting(context);
			}
			socketResult = _ortc_open_socket(context);
			if(socketResult == 0){
				_ortc_change_state(context, OPENED);
				context->loop_active_connecting = 0;
				pthread_detach(pthread_self());
				return 0;
			}
		}
		Sleep(10);
		centiseconds++;
		if(centiseconds > ORTC_RECONNECT_INTERVAL * 100){
			centiseconds = 0;
		}
	}
	if(ortc_log) printf("ortc_log connecting loop leaving\n");
	return 0;
}

void *_ortc_loop_throttle(void *ptr){
	ortc_context *context;
	int centiseconds = 0;
	
	if(ortc_log) printf("ortc_log throttle loop started\n");
	context = (ortc_context *) ptr;
	context->throttleCounter = 0;	
	while(context->loop_active_throttle){			
		centiseconds++;
		if(centiseconds >= 100){ //one second has passed
			centiseconds = 0;
			context->throttleCounter = 0;
		}
		Sleep(10);
	}
	if(ortc_log) printf("ortc_log throttle loop leaving\n");
	return 0;
}

void *_ortc_loop_serverHeartbeat(void *ptr){
	ortc_context *context;
	int centiseconds = 0;
	
	if(ortc_log) printf("ortc_log server hb loop started\n");
	context = (ortc_context *) ptr;
	while(context->loop_active_serverHeartbeat){
		Sleep(10);
		centiseconds++;
		if(centiseconds >= 100){ //one second has passed
			centiseconds=0;
			context->heartbeat_counter++;
			if(context->heartbeat_counter > ORTC_HEARTBEAT_TIMEOUT && context->loop_active_communication){
				_ortc_exception(context, (char*)"Server heartbeat failed!");
				_ortc_change_state(context, RECONNECTING);
				break;
			}
		}
	}
	context->loop_active_serverHeartbeat = 0;
	if(ortc_log) printf("ortc_log server hb loop leaving\n");
    return 0;
}

void *_ortc_loop_clientHeartbeat(void *ptr){
	ortc_context *context;
	int centiseconds = 0;
	char* hb;
	
	if(ortc_log) printf("ortc_log client hb loop started\n");
	context = (ortc_context *) ptr;
	while(context->loop_active_clientHeartbeat){
		Sleep(10);
		if(centiseconds==1){
			hb = (char*)malloc(sizeof(char) * (strlen("\"b\"")+1));
			sprintf(hb, "\"b\"");
			_ortc_send_command(context, hb);
			if(ortc_log) printf("ortc_log sending hb\n");
		} else if(centiseconds > context->heartbeatTime * 100){
			centiseconds = 0;
		} 		
		centiseconds++;
	}
	if(ortc_log) printf("ortc_log client hb loop leaving\n");
	return 0;
}

void *_ortc_loop_reconnect(void *ptr){
	ortc_context *context;
	int socketResult = -1, centiseconds = 0, tret;
	
	if(ortc_log) printf("ortc_log reconnecting loop started\n");
	context = (ortc_context *) ptr;
	context->loop_active_reconnecting = 0; //to not kill itself:)
	_ortc_finish_loops(context);
	context->loop_active_reconnecting = 1;
	if(context->lws_context)
		lws_context_destroy(context->lws_context);
	context->lws_context = NULL;
	context->wsi = NULL;
	
	while(context->loop_active_reconnecting && _ortc_prepare_websocket(context) < 0){
		if(ortc_log) printf("ortc_log prepare ws failed\n");
		Sleep(100);
	}
	
	while(context->loop_active_reconnecting && socketResult != 0) {
		if(centiseconds == 0){
			if(!context->virginConnect)
				_ortc_fireOnReconnecting(context);
			socketResult = _ortc_open_socket(context);
			if(socketResult == 0){
				_ortc_change_state(context, OPENED);
				break;
			}
		}
		Sleep(10);
		centiseconds++;
		if(centiseconds > ORTC_RECONNECT_INTERVAL * 100){
			centiseconds = 0;
		}
	}
	context->loop_active_reconnecting = 0;
	if(ortc_log) printf("ortc_log reconnecting loop leaving\n");
	return 0;	
}


// *************** END OF LOOPS *********************


// ***************** STARTERS ***********************

int _ortc_run_connecting_loop(ortc_context* context){
	int tret;
	context->loop_active_connecting = 1;
	tret = pthread_create(&context->thrdConnecting, NULL, _ortc_loop_connecting, context);
	if(tret!=0){
		context->loop_active_connecting = 0;
		_ortc_exception(context,  (char*)"Error creating connection thread!");
		return -1;
	}
	return 0;
}

int _ortc_run_reconnecting_loop(ortc_context *context){
	int tret;
	context->loop_active_reconnecting = 1;
	tret = pthread_create(&context->thrdReconnect, NULL, _ortc_loop_reconnect, context);
	if(tret!=0){    
		context->loop_active_reconnecting = 0;
		_ortc_exception(context,  (char*)"Error creating reconnecting thread!");
		return -1;
	}
	return 0;
}

int _ortc_run_communication_loop(ortc_context* context){
	int tret;
	context->loop_active_communication = 1;
	tret = pthread_create(&context->thrdCommunication, NULL, _ortc_loop_communication, context);
	if(tret!=0){
		context->loop_active_communication = 0;
		_ortc_exception(context,  (char*)"Error creating communication thread!");
	}
	return 0;
}

int _ortc_run_monotorization_loops(ortc_context *context){
	int tret;
	context->loop_active_throttle = 1;
	tret = pthread_create(&context->thrdThrottle, NULL, _ortc_loop_throttle, context);
	if(tret!=0){    
		context->loop_active_throttle = 0;
		_ortc_exception(context,  (char*)"Error creating throttle thread!");
		return -1;
	}
	context->loop_active_serverHeartbeat = 1;
	tret = pthread_create(&context->thrdServerHeartbeat, NULL, _ortc_loop_serverHeartbeat, context);
	if(tret!=0){    
		context->loop_active_serverHeartbeat = 0;
		_ortc_exception(context,  (char*)"Error creating heartbeat thread!");
		return -1;
	}
	
	if(context->heartbeatActive){
		context->loop_active_clientHeartbeat = 1;
		tret = pthread_create(&context->thrdClientHeartbeat, NULL, _ortc_loop_clientHeartbeat, context);
		if(tret!=0){    
			context->loop_active_clientHeartbeat = 0;
			_ortc_exception(context,  (char*)"Error creating client heartbeat thread!");
			return -1;
		}
	}
	return 0;
}

// *************** END OF STARTERS *******************


void *_ortc_disconnecting_worker(void *ptr){
	ortc_context *context;
	context = (ortc_context *) ptr;
	_ortc_finish_loops(context);
	
	if(context->lws_context)
		lws_context_destroy(context->lws_context);
	context->lws_context = NULL;
	context->wsi = NULL;

	_ortc_dlist_clear(context->multiparts);
	_ortc_dlist_clear(context->channels);
	_ortc_dlist_clear(context->permissions);
	_ortc_dlist_clear(context->messagesToSend);
	_ortc_dlist_clear(context->ortcCommands);

	_ortc_change_state(context, DISCONNECTED);
	return 0;
}

int _ortc_disconnecting(ortc_context *context){
	pthread_t thr;
	int tret = pthread_create(&thr, NULL, _ortc_disconnecting_worker, context);
	if(tret!=0){    
		_ortc_exception(context,  (char*)"Error creating reconnecting thread!");
		return -1;
	}
	return 0;
}

//======================================

void _ortc_cancel_connecting(ortc_context* context){// in case of ORTC handshake, ex. Invalid connection
	if(context->state == OPENED){
		_ortc_change_state(context, DISCONNECTING);
	}
}


int _ortc_open_socket(ortc_context* context){
	char *balancerResponse = NULL, *path = NULL;
	if(ortc_log) printf("ortc_log will try to connect\n");
	if(!context->url){//get the url from balancer    
		if(_ortc_getBalancer(context->cluster, context->appKey, context->verifyPeerCert, &balancerResponse)!=0){
			_ortc_exception(context, balancerResponse);
			free(balancerResponse);
			return -1;
		}
		context->server = strdup(balancerResponse);
		if(_ortc_parseUrl(balancerResponse, &context->host, &context->port, &context->useSSL) < 0){
			_ortc_exception(context, (char*)"malloc() failed! parsing cluster url");
			free(balancerResponse);
			return -1;
		}
		free(balancerResponse);
	} else {//use url provided by user
		context->server = strdup(context->url);
		if(_ortc_parseUrl(context->url, &context->host, &context->port, &context->useSSL) < 0){
			_ortc_exception(context, (char*)"malloc() failed! parsing user url");
			return -1;
		}
	}
	path = (char*)_ortc_prepareConnectionPath();
 
	if(context->verifyPeerCert && context->useSSL > 0){
		context->useSSL = 1;
	}

	lws_cancel_service(context->lws_context);

	context->wsi = lws_client_connect_extended(context->lws_context, context->host, context->port, context->useSSL, path, "", "", "ortc-protocol", -1, context);
	free(path);
	if(context->wsi == NULL){
		_ortc_exception(context,  (char*)"Creating websocket failed!");
		return -1;
	}
  
	#if !defined(WIN32) && !defined(_WIN32) && !defined(__APPLE__) && !defined(ANDROID)
		int wsSock = libwebsocket_get_socket_fd(context->wsi);
		int opt = 1;
		setsockopt(wsSock, IPPROTO_TCP, TCP_CORK, &opt, sizeof(opt));
	#endif
	if(ortc_log) printf("ortc_log socket is opened\n");
	return 0;
}

const char* _ortc_decodeState(enum _ortc_state state){
	switch(state){
		case DISCONNECTED: return "DISCONNECTED";
		case CONNECTED: return "CONNECTED";
		case CONNECTING: return "CONNECTING";
		case OPENED: return "OPENED";
		case DISCONNECTING: return "DISCONNECTING";
		case RECONNECTING: return "RECONNECTING";
		case QUITTING: return "QUITTING";
		default: return "UNKNOWN";
	}
}

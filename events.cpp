#include "libortc.h"
#include "events.h"
#include "common.h"

void _ortc_subscribeOnReconnected(ortc_context *context);

void *_ortc_job_onConnected(void *ptr){
	ortc_context *context;
	context = (ortc_context *) ptr;
	context->onConnected(context);
	pthread_detach(pthread_self());
	return 0;
}

void _ortc_fireOnConnected(ortc_context *context){
	pthread_t thr;
	if(context->onConnected != NULL){
		int tret = pthread_create(&thr, NULL, _ortc_job_onConnected, context);
		if(tret!=0){    
			_ortc_exception(context,  (char*)"Error creating event (onConnected) thread!");
		}
	}
}



void *_ortc_job_onDisconnected(void *ptr){
	ortc_context *context;
	context = (ortc_context *) ptr;
	context->onDisconnected(context);
	pthread_detach(pthread_self());
	return 0;
}

void _ortc_fireOnDisconnected(ortc_context *context){
	pthread_t thr;
	if(context->onDisconnected != NULL){
		int tret = pthread_create(&thr, NULL, _ortc_job_onDisconnected, context);
		if(tret!=0){    
			_ortc_exception(context,  (char*)"Error creating event (onDisconnected) thread!");
		}
	}
}



void *_ortc_job_onReconnected(void *ptr){
	ortc_context *context;
	context = (ortc_context *) ptr;
	context->onReconnected(context);
	_ortc_subscribeOnReconnected(context);
	pthread_detach(pthread_self());
	return 0;
}

void _ortc_fireOnReconnected(ortc_context *context){
	pthread_t thr;
	if(context->onReconnected != NULL){
		int tret = pthread_create(&thr, NULL, _ortc_job_onReconnected, context);
		if(tret!=0){    
			_ortc_exception(context,  (char*)"Error creating event (onReconnected) thread!");
		}
	}
}



void *_ortc_job_onReconnecting(void *ptr){
	ortc_context *context;
	context = (ortc_context *) ptr;
	context->onReconnecting(context);
}

void _ortc_fireOnReconnecting(ortc_context *context){
	pthread_t thr;
	if(context->onReconnecting != NULL){
		int tret = pthread_create(&thr, NULL, _ortc_job_onReconnecting, context);
		if(tret!=0){    
			_ortc_exception(context,  (char*)"Error creating event (onReconnecting) thread!");
		}
	}
}
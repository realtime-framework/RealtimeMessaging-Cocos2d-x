#ifndef libortc_h__balancer
#define libortc_h__balancer

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _ortc_getBalancer(char* url, char* appKey, int verifyCA, char** response);
int _ortc_parseUrl(char* url, char **host, int *port, int *useSSL);
#endif  // libortc_h__balancer

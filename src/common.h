#pragma once
#ifndef COMMON_H
#define COMMON_H

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32)
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include "version.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"

using namespace std;

#ifndef ts_C
#ifdef __cplusplus
#define ts_C extern "C"
#else
#define ts_C extern
#endif
#endif

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128
	
#define PLUGIN_NAME "Channel Watcher"
#define PLUGIN_VERSION VER_FILE_VERSION_STR
#define PLUGIN_API_VERSION 23
#define PLUGIN_AUTHOR "nimradium"
#define PLUGIN_DESCRIPTION "Watches selected channels (e.g. Support-Channels) and notifies you when a user joins this channels."

extern struct TS3Functions ts3Functions;
extern char* pluginID;
extern char pluginPath[PATH_BUFSIZE];
struct ReturnCode {
	char name[128];
	char code[RETURNCODE_BUFSIZE];
	char arg[128];
};
extern map<std::string,ReturnCode> returnCodes;

enum CWErrorType {
	ERROR_CW_noChannelWithThisID	= 0x2000,
	ERROR_CW_noServerWithThisSchID	= 0x2001
};
#endif
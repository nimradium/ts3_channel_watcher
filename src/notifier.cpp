#include "common.h"
#include "notifier.h"
#include <time.h>

char* cw_getTimeS() {
	time_t t = time(nullptr);
	tm time;
	localtime_s(&time, &t);
	char time_s[9];
	strftime(time_s, 9, "%T", &time);
	return time_s;
}

void cw_playNotificationSound(uint64 schID) {
	char path[PATH_BUFSIZE];
	ts3Functions.getResourcesPath(path, PATH_BUFSIZE);
	char filePath[] = "sound/default/stop_talking.wav";

	strcat_s(path, filePath);

	if (ts3Functions.playWaveFile(schID, path) != ERROR_ok) {
		ts3Functions.logMessage("Error playing notification sound", LogLevel_ERROR, "Channel Watcher", schID);
		return;
	}
}

void cw_notification(uint64 schID, const char* text) {
	char time_s[9];
	strcpy_s(time_s, 9, cw_getTimeS());

	char msg[TS3_MAX_SIZE_TEXTMESSAGE];
	snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "---[i]<%s>[/i] [color=black][b]Channel Watcher  :[/b][/color]  %s", time_s, text);
	ts3Functions.printMessage(schID, msg, PLUGIN_MESSAGE_TARGET_SERVER);
}

void cw_notifyOnJoin(uint64 schID, anyID clientID, uint64 newChannelID) {

	char* clientName;
	if (ts3Functions.getClientVariableAsString(schID, clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
		ts3Functions.logMessage("Error querying client name", LogLevel_ERROR, "Channel Watcher", schID);
		return;
	}

	char* clientUID;
	if (ts3Functions.getClientVariableAsString(schID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) != ERROR_ok) {
		ts3Functions.logMessage("Error querying client Unique ID", LogLevel_ERROR, "Channel Watcher", schID);
		return;
	}

	char* channelName;
	if (ts3Functions.getChannelVariableAsString(schID, newChannelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
		ts3Functions.logMessage("Error querying channel name", LogLevel_ERROR, "Channel Watcher", schID);
		return;
	}

	char msg[TS3_MAX_SIZE_TEXTMESSAGE];
	snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "[URL=client://%i/%s]\"%s\"[/URL] joined [URL=channelid://%llu]\"%s\"[/URL]", clientID, clientUID, clientName, newChannelID, channelName);
	//SECOND URL IS UNDERLINED
	cw_notification(schID, msg);

	cw_playNotificationSound(schID);

	ts3Functions.freeMemory(clientName);
	ts3Functions.freeMemory(clientUID);
	ts3Functions.freeMemory(channelName);
}
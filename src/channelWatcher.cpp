#include "common.h"
#include "notifier.h"
#include "FileHandler.h"
#include "channelWatcher.h"
#include <mutex>

#pragma warning(disable : 4996)
#pragma warning(disable : 6387)

vector<WatchServer> servers; //vector containing all active WatchServers
SaveFile saveFile;

#pragma region WatchChannel

void WatchChannel::setChannelID(uint64 channelID) {
	this->channelID = channelID;
}

uint64 WatchChannel::getChannelID() {
	return channelID;
}

void WatchChannel::setChannelName(char* channelName) {
	size_t sz = strlen(channelName) + 1;
	this->channelName = (char*)malloc(sz * sizeof(char));
	strcpy_s(this->channelName, sz, channelName);
}

char* WatchChannel::getChannelName() {
	return channelName;
}

int WatchChannel::createWatchChannel(uint64 schID, uint64 channelID, WatchChannel& channel)
{
	char* channelName;
	if (ts3Functions.getChannelVariableAsString(schID, channelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
		ts3Functions.logMessage("Error querying Channel Name", LogLevel_ERROR, "Channel Watcher", schID);
		return ERROR_undefined;
	}
	channel.setChannelID(channelID);
	channel.setChannelName(channelName);

	ts3Functions.freeMemory(channelName);
	return ERROR_ok;
}

#pragma endregion

#pragma region WatchServer
void WatchServer::setServerUID(char* serverUID)
{
	size_t sz = strlen(serverUID) + 1;
	this->serverUID = (char*)malloc(sz * sizeof(char));
	strcpy_s(this->serverUID, sz, serverUID);
}
char* WatchServer::getServerUID()
{
	return serverUID;
}
void WatchServer::setServerName(char* serverName)
{
	size_t sz = strlen(serverName) + 1;
	this->serverName = (char*)malloc(sz * sizeof(char));
	strcpy_s(this->serverName, sz, serverName);
}

char* WatchServer::getServerName()
{
	return serverName;
}

void WatchServer::setSchID(uint64 schID)
{
	this->schID = schID;
}

uint64 WatchServer::getSchID()
{
	return schID;
}


int WatchServer::getServerFromSchID(uint64 schID, WatchServer** cwServer)
{
	for (WatchServer& server : servers) {
		if (server.getSchID() == schID) {
			if (cwServer != nullptr) {
				*cwServer = &server;
			}
			return ERROR_ok;
		}
	}
	return ERROR_CW_noServerWithThisSchID;
}

vector<WatchChannel>* WatchServer::getWatchChannels()
{
	return &watchChannels;
}

int WatchServer::createConnection(uint64 schID, WatchServer& server) {

	char* serverUID;
	char* serverName;

	if (ts3Functions.getServerVariableAsString(schID, VIRTUALSERVER_UNIQUE_IDENTIFIER, &serverUID) != ERROR_ok) {
		ts3Functions.logMessage("Error querying Virtualserver Unique Identifier", LogLevel_ERROR, "Channel Watcher", schID);
		return ERROR_undefined;
	}

	if (ts3Functions.getServerVariableAsString(schID, VIRTUALSERVER_NAME, &serverName) != ERROR_ok) {
		ts3Functions.logMessage("Error querying Virtualserver Name", LogLevel_ERROR, "Channel Watcher", schID);
		return ERROR_undefined;
	}
#ifdef DEBUG
	printf("CHANNEL WATCHER: adding Server with Server ID: %s\n", serverUID);
#endif // DEBUG
	server.setSchID(schID);
	server.setServerUID(serverUID);
	server.setServerName(serverName);

	ts3Functions.freeMemory(serverName);

	return ERROR_ok;
}

void WatchServer::addWatchChannelToWatchChannelsList(WatchChannel& channel) {
	watchChannels.push_back(channel);
	ChannelWatcher::channelInfoUpdate(schID, channel.getChannelID());
}

unsigned int WatchServer::loadWatchChannelsFromSaveFile()
{
	if (saveFile.containsServer(serverUID)) {
		ts3Functions.logMessage("Loading WatchChannels", LogLevel_DEBUG, "Channel Watcher", schID);
		uint64* channelIDs;
		size_t size;
		if (saveFile.loadWatchChannels(serverUID, schID, channelIDs, size) != ERROR_ok) {
		}
		for (int i = 0; i < size;i++) {
			int buf;
			if (int res = ts3Functions.getChannelVariableAsInt(schID, channelIDs[i], CHANNEL_ENDMARKER,&buf); res  == ERROR_ok) {
				ChannelWatcher::addChannel(schID, channelIDs[i], "CW_ADDCHANNEL_LOAD");
				char* channelName;
				if (ts3Functions.getChannelVariableAsString(schID, channelIDs[i], CHANNEL_NAME, &channelName) != ERROR_ok) {
					ts3Functions.logMessage("Error querying Channel Name", LogLevel_ERROR, "Channel Watcher", schID);
				}
				saveFile.updateChannel(serverUID, channelIDs[i], channelName);
				ts3Functions.freeMemory(channelName);
			}
			else {
				saveFile.removeWatchChannel(serverUID, channelIDs[i]);
			}
		}
		free(channelIDs);
	}
	return 0;
}

int WatchServer::removeWatchChannel(WatchChannel* channel)
{
	uint64 channelID = channel->getChannelID();
	vector<WatchChannel>::iterator i;
	//auto compChannelID = [channel](WatchChannel& wc) {if (&wc == channel) { return true; }return false; }; Maybe use this idea in the future to make the comparison easier to understand.
	i = find_if(watchChannels.begin(), watchChannels.end(), [channel](WatchChannel& wc) -> bool {if (&wc == channel) { return true; }return false; });
	if (i != watchChannels.end()) {
		watchChannels.erase(i);
		ChannelWatcher::channelInfoUpdate(schID, channelID);
		return ERROR_ok;
	}
	ts3Functions.logMessage("ERROR could not remove channel from WatchChannels list (WatchServer::removeWatchChannel). Please report this bug.", LogLevel_ERROR, "Channel Watcher", this->getSchID());
	return ERROR_undefined;
}

int WatchServer::removeWatchChannelByID(uint64 channelID)
{
	vector<WatchChannel>::iterator i;

	i = find_if(watchChannels.begin(), watchChannels.end(), [channelID](WatchChannel& wc) -> bool {if (wc.getChannelID() == channelID) { return true; }return false; });
	if (i != watchChannels.end()) {
		watchChannels.erase(i);
		ChannelWatcher::channelInfoUpdate(schID, channelID);
		return ERROR_ok;
	}

	ts3Functions.logMessage("ERROR could not remove channel from WatchChannels list (WatchServer::removeWatchChannelByID). Please report this bug.", LogLevel_ERROR, "Channel Watcher", this->getSchID());
	return ERROR_undefined;
}

int WatchServer::getWatchChannelFromID(uint64 channelID, WatchChannel** channel)
{
	for (WatchChannel& c : watchChannels) {
		if (c.getChannelID() == channelID) {
			if (channel != nullptr) {
				*channel = &c;
			}
			return ERROR_ok;
		}
	}
	return ERROR_CW_noChannelWithThisID;
}
int WatchServer::getWatchChannelFromID(uint64 schID, uint64 channelID, WatchChannel** channel)
{
	WatchServer* server;
	WatchServer::getServerFromSchID(schID, &server);
	for (WatchChannel& c : *server->getWatchChannels()) {
		if (c.getChannelID() == channelID) {
			if (channel != nullptr) {
				*channel = &c;
			}
			return ERROR_ok;
		}
	}
	return ERROR_CW_noChannelWithThisID;
}

const char* WatchServer::getWatchChannelNameFromID(uint64 channelID)
{
	for (WatchChannel& c : watchChannels) {
		if (c.getChannelID() == channelID) {
			char channelName[TS3_MAX_SIZE_CHANNEL_NAME];
			snprintf(channelName, TS3_MAX_SIZE_CHANNEL_NAME, "%s", c.getChannelName());
			printf(channelName);
			return channelName;
		}
	}
	return "";
}

const char* WatchServer::getWatchChannelNameFromID(uint64 schID, uint64 channelID)
{
	WatchServer* server;
	WatchServer::getServerFromSchID(schID, &server);
	for (WatchChannel& c : *server->getWatchChannels()) {
		if (c.getChannelID() == channelID) {
			char channelName[TS3_MAX_SIZE_CHANNEL_NAME];
			snprintf(channelName, TS3_MAX_SIZE_CHANNEL_NAME, "%s", c.getChannelName());
			return channelName;
		}
	}
	return "";
}
#pragma endregion

#pragma region ChannelWatcher
int ChannelWatcher::init() {
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, pluginID);
	snprintf(pluginPath, PATH_BUFSIZE, "%sChannelWatcher", pluginPath);
	char saveFilePath[PATH_BUFSIZE];
	snprintf(saveFilePath, PATH_BUFSIZE, "%s/watchchannels.json", pluginPath);
	saveFile.setPath(saveFilePath);
	ChannelWatcher::loadDataFromSaveFile();

	uint64* connections = nullptr;
	if (ts3Functions.getServerConnectionHandlerList(&connections) == ERROR_ok) {
		//Check before
		int i = 0;
		while (connections[i]) {
			int result;
			if (ts3Functions.getConnectionStatus(connections[i], &result) != ERROR_ok) {
				return 1;
			}
			if (result == 1) {
				ChannelWatcher::onConnectStatusChangeEvent(connections[i], STATUS_CONNECTION_ESTABLISHED);
			}
			i++;
		}
		ts3Functions.freeMemory(connections);
	}
	else {
		return 1;
	}

	return 0;
}

void ChannelWatcher::shutdown()
{
	saveFile.save();
}

anyID ChannelWatcher::getOwnClientID(uint64 schID) {
	anyID id;
	if (ts3Functions.getClientID(schID, &id) != ERROR_ok) {
		ts3Functions.logMessage("Error querying own client ID", LogLevel_ERROR, "Channel Watcher", schID);
	}
	return id;
}

uint64 ChannelWatcher::getOwnChannelID(uint64 schID) {
	uint64 id;
	anyID clientID;
	clientID = getOwnClientID(schID);
	if (ts3Functions.getChannelOfClient(schID, clientID, &id)!= ERROR_ok) {
		ts3Functions.logMessage("Error querying current channel ID", LogLevel_ERROR, "Channel Watcher", schID);
	}
	return id;
}


bool ChannelWatcher::checkChannel(uint64 schID, uint64 channelID) {
	WatchServer* server;
	if (WatchServer::getServerFromSchID(schID, &server) != ERROR_ok) {
		return true; //This may trigger a notification or other errors but it does not create multiple watchchannels
	}
	int ret = server->getWatchChannelFromID(channelID, nullptr);
	switch (ret) {
	case ERROR_ok:
		return true;
	case ERROR_CW_noChannelWithThisID:
		return false;
	default:
		return true; //This may trigger a notification or other errors but it does not create multiple watchchannels
	}
}
bool ChannelWatcher::checkServer(uint64 schID)
{
	int ret = WatchServer::getServerFromSchID(schID, nullptr);
	switch (ret) {
	case ERROR_ok:
		return true;
	case ERROR_CW_noServerWithThisSchID:
		return false;
	default:
		return false;
	}
}
//Is called when the MenuItem MENU_CHANNEL_SELECT_CW is clicked
void ChannelWatcher::selectWatchChannel(uint64 schID, uint64 channelID) {
	if (!checkChannel(schID, channelID)) {
		if (addChannel(schID, channelID, "CW_ADDCHANNEL_SELECT") != ERROR_ok) {
			return;
		}
	}
	else {
		cw_notification(schID, "This channel is already on your watchlist.");
	}

}
//Is called when the MenuItem MENU_CHANNEL_REMOVE_CW is clicked
void ChannelWatcher::unselectWatchChannel(uint64 schID, uint64 channelID) {
	if (checkChannel(schID, channelID)) {
		WatchChannel* channel;
		if (WatchServer::getWatchChannelFromID(schID, channelID, &channel) != ERROR_ok) {
			ts3Functions.logMessage("ERROR in WatchServer::getWatchChannelFromID function (ChannelWatcher::unselectWatchChannel). Please report this bug.", LogLevel_ERROR, "Channel Watcher", schID);
			return;
		}
		char* channelName = channel->getChannelName();
		ChannelWatcher::removeChannel(schID, channelID);

		char msg[TS3_MAX_SIZE_TEXTMESSAGE];
		snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "Removed [URL=channelid://%llu][color=blue][b]\"%s\"[/b][/color][/URL] from your watch list", channelID, channelName);
		cw_notification(schID, msg);
	}
	else {
		cw_notification(schID, "This channel is not on your watchlist.");
	}
}

unsigned int ChannelWatcher::addChannel(uint64 schID, uint64 channelID, const char* mode) {
	if (!checkChannel(schID, channelID)) {
		if (subChannel(schID, channelID, mode) != ERROR_ok) {
			return ERROR_undefined;
		}
		return ERROR_ok;
	}
	return ERROR_undefined;
}

unsigned int ChannelWatcher::removeChannel(uint64 schID, uint64 channelID)
{
	if (checkChannel(schID, channelID)) {
		WatchServer* server;
		WatchChannel* channel;
		if (WatchServer::getServerFromSchID(1, &server) != ERROR_ok) {
			return ERROR_undefined;
		}
		if(server->getWatchChannelFromID(channelID, &channel)!= ERROR_ok){
			return ERROR_undefined;
		}
		if (saveFile.removeWatchChannel(server, channel) != ERROR_ok) {
			return ERROR_undefined;
		}
		if (server->removeWatchChannel(channel) != ERROR_ok) {
			return ERROR_undefined;
		}
		return ERROR_ok;
	}
	return ERROR_undefined;
}

unsigned int ChannelWatcher::subChannel(uint64 schID, uint64 channelID, const char* rcName) {
	const uint64 chID[]{ channelID, NULL };
	ReturnCode rc;
	ts3Functions.createReturnCode(pluginID, rc.code, RETURNCODE_BUFSIZE);
	snprintf(rc.arg, 128, "%llu", (unsigned long long)channelID);
	snprintf(rc.name, 128, rcName);
	string code(rc.code);
	returnCodes[code] = rc;

	if (ts3Functions.requestChannelSubscribe(schID, chID, rc.code) != ERROR_ok) {
		ts3Functions.logMessage("ERROR in addChannel function. Please report this bug.", LogLevel_ERROR, "Channel Watcher", schID);
		return ERROR_undefined;
	}
	return ERROR_ok;
}

int ChannelWatcher::channelInfoUpdate(uint64 schID, uint64 channelID)
{
	if (ts3Functions.requestInfoUpdate(schID, PLUGIN_CHANNEL, channelID) == ERROR_ok) {
		return ERROR_ok;
	};
	return ERROR_undefined;
}

void ChannelWatcher::loadDataFromSaveFile() {
	saveFile.load();
}

int ChannelWatcher::onConnectStatusChangeEvent(uint64 schID, int newStatus)
{
	if (newStatus == STATUS_CONNECTION_ESTABLISHED) {
		WatchServer server;
		if (WatchServer::createConnection(schID, server) != ERROR_ok) {
			return ERROR_undefined;
		}
		servers.push_back(server);
#ifdef DEBUG
		printf("CHANNEL WATCHER: new Server added:\n Name: %s\n UID: %s\n schID: %llu\n servers Size: %zu\n WatchChannels Size: %zu\n", server.getServerName(), server.getServerUID(), server.getSchID(), servers.size(), server.getWatchChannels()->size());
#endif
		if (saveFile.containsServer(server.getServerUID())) {
			saveFile.updateServer(server.getServerUID(), server.getServerName());
			server.loadWatchChannelsFromSaveFile();
		}
		return ERROR_ok;
	}else if(newStatus == STATUS_DISCONNECTED){
		WatchServer* server;
		if (WatchServer::getServerFromSchID(schID, &server) != ERROR_ok) {
			return ERROR_undefined;
		}
		vector<WatchServer>::iterator i = find_if(servers.begin(), servers.end(), [server](WatchServer& wc) -> bool {if (&wc == server) { return true; }return false; });
		if (i != servers.end()) {
			servers.erase(i);
			return ERROR_ok;
		}
	}
	return ERROR_undefined;
}

int ChannelWatcher::onAddChannelSubEvent(uint64 schID, unsigned int error, char* arg, int mode){
	uint64 channelID = atol(arg);
	switch (error)
	{
	case ERROR_ok:
	{
		switch (mode)
		{
		case 0:
			if (!checkChannel(schID, channelID)) {
				WatchChannel channel;
				if (WatchChannel::createWatchChannel(schID, channelID, channel) != ERROR_ok) {
					return 0;
				}

				WatchServer* server;
				if (WatchServer::getServerFromSchID(schID, &server) != ERROR_ok) {

					return ERROR_undefined;
				}
				server->addWatchChannelToWatchChannelsList(channel);
				saveFile.addWatchChannel(server, channel);

				char msg[TS3_MAX_SIZE_TEXTMESSAGE];
				snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "Added [URL=channelid://%llu][color=blue][b]\"%s\"[/b][/color][/URL] to your watch list", channel.getChannelID(), channel.getChannelName());
				cw_notification(schID, msg);
			}


			return 1;
		case 1:
			if (!checkChannel(schID, channelID)) {
				WatchChannel channel;
				if (WatchChannel::createWatchChannel(schID, channelID, channel) != ERROR_ok) {
					return 0;
				}
				WatchServer* server;
				if (WatchServer::getServerFromSchID(schID, &server) != ERROR_ok) {

					return ERROR_undefined;
				}
				server->addWatchChannelToWatchChannelsList(channel);
				char msg[TS3_MAX_SIZE_TEXTMESSAGE];
				snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "Loaded [URL=channelid://%llu][color=blue][b]\"%s\"[/b][/color][/URL] from your saved watch list", channel.getChannelID(), channel.getChannelName());
				cw_notification(schID, msg);
			}
			return 1;
		default:
			return 0;
			break;
		}
	}
	case ERROR_permissions_client_insufficient:
	{
		switch (mode) {
		case 0:
			cw_notification(schID, "[color=red][b]ERROR: You need to be able to subscribe to a channel to add it to your watch list (failed on i_channel_needed_subscribe_power)[/b][/color]");
			if (checkChannel(schID, channelID)) {
				WatchServer* server;
				if (WatchServer::getServerFromSchID(schID, &server) != ERROR_ok) {
					return 0;
				}
				WatchChannel* channel;
				if (server->getWatchChannelFromID(channelID, &channel) != ERROR_ok) {
					server->removeWatchChannel(channel);
				}
			}
			return 1;
		case 1:
			cw_notification(schID, "[color=red][b]ERROR: You need to be able to subscribe to a channel to load it from your saved watch list (failed on i_channel_needed_subscribe_power)[/b][/color]");
			if (checkChannel(schID, channelID)) {
				WatchServer* server;
				if (WatchServer::getServerFromSchID(schID, &server) != ERROR_ok) {
					return 0;
				}
				WatchChannel* channel;
				if (server->getWatchChannelFromID(channelID, &channel) != ERROR_ok) {
					return 0;
				}
				server->removeWatchChannel(channel);
				saveFile.removeWatchChannel(server, channel);
			}
			return 1;
		}
	}
	default:
		return 0;
	}
	return 0;
}

int ChannelWatcher::onUnsubSubEvent(uint64 schID, unsigned int error, char* arg) {
	uint64 channelID = atol(arg);
	list<uint64>::iterator i;

	switch (error)
	{
	case ERROR_ok:
		{
			char* channelName;
			if (ts3Functions.getChannelVariableAsString(schID, channelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
				ts3Functions.logMessage("ERROR querying Channel Name (onUnsubSubEvent::ERROR_ok).", LogLevel_ERROR, "Channel Watcher", schID);
			}

			char msg[TS3_MAX_SIZE_TEXTMESSAGE];
			snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "You need to stay subscribed to [URL=channelid://%llu][color=red][b]\"%s\"[/b][/color][/URL] while it's on your watchlist", channelID, channelName);
			cw_notification(schID, msg);

			ts3Functions.freeMemory(channelName);
			return 1;
		}
	case ERROR_permissions_client_insufficient:
		{
			char msg[TS3_MAX_SIZE_TEXTMESSAGE];
			char* channelName;
			if (ts3Functions.getChannelVariableAsString(schID, channelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
				ts3Functions.logMessage("ERROR querying Channel Name (onUnsubSubEvent::ERROR_permissions_client_insufficient).", LogLevel_ERROR, "Channel Watcher", schID);
				return 1;
			}

			snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "[color=red][b]ERROR: You need to be able to stay subscribed to [URL=channelid://%llu][b]\"%s\"[/b][/URL] to watch it (failed on i_channel_needed_subscribe_power)[/b][/color]", channelID, channelName);
			cw_notification(schID, msg);

			WatchServer* server;
			if (WatchServer::getServerFromSchID(schID, &server) != ERROR_ok) {
				return 1;
			}
			WatchChannel* channel;
			if (server->getWatchChannelFromID(channelID, &channel) != ERROR_ok) {
				ts3Functions.logMessage("ERROR no Channel with this ID found (onUnsubSubEvent::ERROR_permissions_client_insufficient::getWatchChannelFromID)", LogLevel_ERROR, "Channel Watcher", schID);
				return 1;
			}
			if (saveFile.removeWatchChannel(server, channel) != ERROR_ok) {
				ts3Functions.logMessage("ERROR not able to remove watchchannel form saveFile (onUnsubSubEvent::ERROR_permissions_client_insufficient::saveFile.removeWatchChannel)", LogLevel_ERROR, "Channel Watcher", schID);
			}
			if (server->removeWatchChannel(channel) != ERROR_ok) {
				ts3Functions.logMessage("ERROR not able to remove watchchannel (onUnsubSubEvent::ERROR_permissions_client_insufficient::server.removeWatchChannel)", LogLevel_ERROR, "Channel Watcher", schID);
				return 1;
			}

			snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "Removed [URL=channelid://%llu][color=red][b]\"%s\"[/b][/color][/URL] from your Watchlist", channelID, channelName);
			cw_notification(schID, msg);

			ts3Functions.freeMemory(channelName);
			return 1;
		}
	default:
		return 0;
	}
	return 0;
}

void ChannelWatcher::onClientMoveEvent(uint64 schID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
	if (newChannelID != 0) {
		if (clientID != getOwnClientID(schID)) {
			if (newChannelID != getOwnChannelID(schID)) {
				if (checkChannel(schID, newChannelID)) {
					cw_notifyOnJoin(schID, clientID, newChannelID);
				}
			}
		}
	}
}

void ChannelWatcher::onChannelDeletedEvent(uint64 schID, uint64 channelID)
{
	if (checkChannel(schID, channelID)) {
		printf("test");
		WatchServer* server;
		WatchChannel* channel;
		WatchServer::getServerFromSchID(schID, &server);
		server->getWatchChannelFromID(channelID, &channel);
		char* channelName;
		channelName = channel->getChannelName();

		saveFile.removeWatchChannel(server, channel);
		server->removeWatchChannel(channel);

		char msg[TS3_MAX_SIZE_TEXTMESSAGE];
		snprintf(msg,TS3_MAX_SIZE_TEXTMESSAGE, "Removed [URL=channelid://%llu][color=red][b]\"%s\"[/b][/color][/URL] from your Watchlist because it got deleted.", channelID, channelName);
		cw_notification(schID, msg);
	}
}

void ChannelWatcher::onChannelEditedEvent(uint64 schID, uint64 channelID)
{
	if (checkChannel(schID, channelID)) {
		WatchChannel* channel;
		WatchServer::getWatchChannelFromID(schID, channelID, &channel);
		char* channelNameNew;
		if (ts3Functions.getChannelVariableAsString(schID, channelID, CHANNEL_NAME, &channelNameNew) != ERROR_ok) {
			ts3Functions.logMessage("ERROR querying Channel Name (onChannelEditedEvent).", LogLevel_ERROR, "Channel Watcher", schID);
			return;
		}
		if (strcmp(channelNameNew, channel->getChannelName())!= 0) {
			channel->setChannelName(channelNameNew);
			saveFile.updateChannel(schID, channelID, channelNameNew);
		}
		ts3Functions.freeMemory(channelNameNew);
	}
}

#pragma endregion

//ADD
void cw_logMessage() {
}
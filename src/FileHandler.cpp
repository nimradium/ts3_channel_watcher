#include "common.h"
#include "FileHandler.h"
#include <direct.h>

#pragma region JSONHandler

#define KEY_SERVERNAME "serverName"
#define KEY_SERVERUID "serverUID"
#define KEY_WATCHCHANNELS "watchChannels"
#define KEY_CHANNELID "channelID"
#define KEY_CHANNELNAME "channelName"

int JSONHandler::loadJSON(FileHandler& fileHandler, json& saveObject)
{
	char* jstr;
	fileHandler.readFile(jstr);
	saveObject = json::parse(jstr);
	free(jstr);
	return 0;
}

int JSONHandler::saveJSON(FileHandler& fileHandler, json& saveObject)
{
	string jstr = saveObject.dump(1, '\t');
	fileHandler.writeFile(jstr.c_str());
	return 0;
}
int JSONHandler::test(FileHandler& fileHandler, json& saveObject)
{
	return 0;
}
#pragma endregion

#pragma region FileHandler

FileHandler::FileHandler()
{
}

void FileHandler::setFilePath(char* path)
{
	size_t sz = strlen(path) + 1;
	this->path = (char*)malloc(sz * sizeof(char));
	strcpy_s(this->path, sz, path);
}

const char* FileHandler::getPath()
{
	return path;
}

FILE* FileHandler::openFile(const char* method, const char* path) {
	FILE* f;
	if (errno_t err = fopen_s(&f, path, method); err != 0) {
		char msg[80];
		strerror_s(msg, err);
		printf(msg);
		ts3Functions.logMessage("ERROR opening file (FileHandler::openFile::fopen_s)", LogLevel_CRITICAL, "Channel Watcher", 0);
		return f;
	}
	return f;
}

int FileHandler::writeFile(const char* str) {
	FILE* f;
	f = openFile("w", path);
	fprintf_s(f, "%s\0", str);
	fclose(f);
	return 0;
}

int FileHandler::readFile(char*& buffer)
{
	FILE* file;
	long size;

	file = openFile("rb", path);

	fseek(file, 0, SEEK_END);
	size = ftell(file) + 1;
	rewind(file);

	buffer = (char*)malloc(size * sizeof(char));
	fread_s(buffer, size, sizeof(char),size, file);
	snprintf(buffer, size, "%s\0", buffer);

	fclose(file);
	return 0;
}

int FileHandler::appendFile(const char* str) {
	FILE* f;
	f = openFile("a+", path);
	fprintf_s(f, str);
	fclose(f);
	return 0;
}

#pragma endregion

SaveFile::SaveFile()
{
}

//not used
SaveFile::SaveFile(char* path)
{
	size_t sz = strlen(this->path) + 1;
	this->path = (char*)malloc(sz * sizeof(char));
	strcpy_s(this->path, sz, path);
	file.setFilePath(path);
	file.appendFile("");
}

void SaveFile::setPath(char* path)
{
	size_t sz = strlen(path) + 1;
	this->path = (char*)malloc(sz * sizeof(char));
	strcpy_s(this->path, sz, path);
	file.setFilePath(path);
}

int SaveFile::load()
{
	file.appendFile("\0");
	JSONHandler::loadJSON(file, data);
	return 0;
}

int SaveFile::save()
{
	JSONHandler::saveJSON(file, data);
	return 0;
}

int SaveFile::addWatchChannel(WatchServer* server, WatchChannel& channel)
{
	const char* serverUID = server->getServerUID();
	const char* serverName = server->getServerName();
	char channelID[10];
	_ltoa_s(channel.getChannelID(), channelID, 10);
	const char* channelName = channel.getChannelName();

	if (!data.contains(serverUID)) {
		this->addServer(serverUID, serverName);
	}
	else {
		updateServer(serverUID,serverName);
	}

	data[serverUID][KEY_WATCHCHANNELS][channelID] = {{KEY_CHANNELNAME, channelName},{KEY_CHANNELID, channelID}};
	save();

	return 0;
}

int SaveFile::loadWatchChannels(const char* serverUID, uint64 schID, uint64*& arr, size_t& size)
{
	size = data[serverUID][KEY_WATCHCHANNELS].size();
	arr = (uint64*)calloc(size,sizeof(uint64));
	int i = 0;
	for (json::value_type channel : data[serverUID][KEY_WATCHCHANNELS]) {
		const uint64 channelID = atol(channel.value(KEY_CHANNELID, "0").c_str());
		arr[i] = channelID;
		i++;
	}
	return ERROR_ok;
}

int SaveFile::removeWatchChannel(WatchServer* server, WatchChannel* channel)
{
	const char* serverUID = server->getServerUID();
	char channelID[10];
	snprintf(channelID, 10,"%llu", channel->getChannelID());
	if (data.contains(serverUID)) {
		if (data[serverUID][KEY_WATCHCHANNELS].contains(channelID)) {
			data[serverUID][KEY_WATCHCHANNELS].erase(channelID);
			if (data[serverUID][KEY_WATCHCHANNELS].size() == 0) {
				removeServer(serverUID);
			}
			save();
			return ERROR_ok;
		}
		else {
			return ERROR_undefined;
		}
	}
	else {
		return ERROR_undefined;
	}
	return 0;
}

int SaveFile::removeWatchChannel(const char* serverUID, uint64 channelIDi)
{
	char channelID[10];
	snprintf(channelID, 10, "%llu", channelIDi);
	if (data.contains(serverUID)) {
		if (data[serverUID][KEY_WATCHCHANNELS].contains(channelID)) {
			data[serverUID][KEY_WATCHCHANNELS].erase(channelID);
			if (data[serverUID][KEY_WATCHCHANNELS].size() == 0) {
				removeServer(serverUID);
			}
			save();
			return ERROR_ok;
		}
		else {
			return ERROR_undefined;
		}
	}
	else {
		return ERROR_undefined;
	}
	return 0;
}

int SaveFile::addServer(const char* serverUID, const char* serverName)
{
	data[serverUID] = { {KEY_SERVERNAME , serverName} ,{KEY_SERVERUID,serverUID},{KEY_WATCHCHANNELS,{}} };
	save();
	return 0;
}

int SaveFile::addServer(WatchServer& server)
{
	return 0;
}

int SaveFile::removeServer(const char* serverUID)
{
	if (data.contains(serverUID)) {
		data.erase(serverUID);
		return ERROR_ok;
	}
	return 0;
}

int SaveFile::updateChannel(const char* serverUID, const char* channelID, const char* channelName)
{
	if (data[serverUID][KEY_WATCHCHANNELS][channelID].value(KEY_CHANNELNAME, "") != channelName) {
		data[serverUID][KEY_WATCHCHANNELS][channelID][KEY_CHANNELNAME] = channelName;
		save();
		return 0;
	}
	return 0;
}

int SaveFile::updateChannel(const char* serverUID, uint64 channelIDint, const char* channelName)
{
	char channelID[36];
	_ltoa_s(channelIDint, channelID, 36);
	if (data[serverUID][KEY_WATCHCHANNELS][channelID].value(KEY_CHANNELNAME, "") != channelName) {
		data[serverUID][KEY_WATCHCHANNELS][channelID][KEY_CHANNELNAME] = channelName;
		save();
		return 0;
	}
	return 0;
}

int SaveFile::updateChannel(uint64 schID, const char* channelID, const char* channelName)
{
	char* serverUID;
	if (ts3Functions.getServerVariableAsString(schID, VIRTUALSERVER_UNIQUE_IDENTIFIER, &serverUID)) {
		ts3Functions.logMessage("ERROR querying ServerUID (SaveFile::updateChannel).", LogLevel_ERROR, "Channel Watcher", schID);
		return 0;
	}
	if (data[serverUID][KEY_WATCHCHANNELS][channelID].value(KEY_CHANNELNAME, "") != channelName) {
		data[serverUID][KEY_WATCHCHANNELS][channelID][KEY_CHANNELNAME] = channelName;
		save();
		return 0;
	}
	return 0;
	ts3Functions.freeMemory(serverUID);
}

int SaveFile::updateChannel(uint64 schID, uint64 channelIDint, const char* channelName)
{
	char* serverUID;
	if (ts3Functions.getServerVariableAsString(schID, VIRTUALSERVER_UNIQUE_IDENTIFIER, &serverUID)) {
		ts3Functions.logMessage("ERROR querying ServerUID (SaveFile::updateChannel).", LogLevel_ERROR, "Channel Watcher", schID);
		return 0;
	}
	char channelID[36];
	_ltoa_s(channelIDint, channelID, 36);
	if (data[serverUID][KEY_WATCHCHANNELS][channelID].value(KEY_CHANNELNAME, "") != channelName) {
		data[serverUID][KEY_WATCHCHANNELS][channelID][KEY_CHANNELNAME] = channelName;
		save();
		return 0;
	}
	return 0;
	ts3Functions.freeMemory(serverUID);
}

int SaveFile::updateServer(const char* serverUID, const char* serverName)
{
	if (data[serverUID].value(KEY_SERVERNAME,"") != serverName) {
		data[serverUID][KEY_SERVERNAME] = serverName;
		save();
		return 0;
	}
	return 0;
}

bool SaveFile::containsServer(const char* serverUID)
{
	if (data.contains(serverUID)) {
		return true;
	}
	return false;
}


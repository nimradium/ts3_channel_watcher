#pragma once
#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "channelWatcher.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
class FileHandler
{
public:
	FileHandler();
	void setFilePath(char* path);
	const char* getPath();
	FILE* openFile(const char* method, const char* path);
	int writeFile(const char* str);
	int readFile(char*& buffer);
	int appendFile(const char* str);
private:
	char* path;
};

class JSONHandler
{
public:
	static int loadJSON(FileHandler& fileHandler, json& saveObject);
	static int saveJSON(FileHandler& fileHandler, json& saveObject);
	static int test(FileHandler& fileHandler, json& saveObject);
private:

};

class SaveFile
{
public:
	SaveFile();
	SaveFile(char* path);
	void setPath(char* path);
	int load();
	int save();
	int addWatchChannel(WatchServer* server, WatchChannel& channel);
	int loadWatchChannels(const char* serverUID, uint64 schID, uint64*& arr, size_t& size);
	int removeWatchChannel(WatchServer* server, WatchChannel* channel);
	int removeWatchChannel(const char* serverUID, uint64 channelIDi);
	int addServer(const char* serverUID, const char* serverName);
	int addServer(WatchServer& server);
	int removeServer(const char* serverUID);
	int updateChannel(const char* serverUID, const char* channelID, const char* channelName);
	int updateChannel(const char* serverUID, uint64 channelIDint, const char* channelName);
	int updateChannel(uint64 schID, const char* channelID, const char* channelName);
	int updateChannel(uint64 schID, uint64 channelIDint, const char* channelName);
	int updateServer(const char* serverUID, const char* serverName);
	bool containsServer(const char* serverUID);

private:
	char* path;
	FileHandler file;
	json data = json::object();
};

#endif // !FILEHANDLER_H


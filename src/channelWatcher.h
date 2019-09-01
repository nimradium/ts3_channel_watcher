#pragma once
#ifndef MAIN_H
#define MAIN_H

class WatchChannel {
	public:
		void setChannelID(uint64 channelID);
		uint64 getChannelID();
		void setChannelName(char* channelName);
		char* getChannelName();
		//Creates a new WatchChannel and writes it to channel pointer.
		static int createWatchChannel(uint64 schID, uint64 channelID, WatchChannel& channel);
	private:
		uint64 channelID;
		char* channelName;
};

class WatchServer {
	public:
		void setServerUID(char* serverUID);
		char* getServerUID();
		void setServerName(char* serverName);
		char* getServerName();
		void setSchID(uint64 schID);
		uint64 getSchID();
		//Searches for a WatchServer in servers list that listens on this serverconnection and writes it to cwServer pointer.
		static int getServerFromSchID(uint64 schID, WatchServer** cwServer);
		vector<WatchChannel>* getWatchChannels();
	
		//Creates a new WatchChannel and writes it to server pointer.
		static int createConnection(uint64 schID, WatchServer& server);
		//Adds a WatchChannel to the WatchChannels list.
		void addWatchChannelToWatchChannelsList(WatchChannel& channel);
		unsigned int loadWatchChannelsFromSaveFile();
		//Removes a WatchChannel from the WatchChannels list. NOT IMPLEMENTED YET!
		int removeWatchChannel(WatchChannel* channel);
		//Removes a WatchChannel from the WatchChannels list. NOT IMPLEMENTED YET!
		int removeWatchChannelByID(uint64 channelID);
		//Searches for a Channel with this ID in WatchChannels list and writes the associated WatchChannel to channel pointer. 
		int getWatchChannelFromID(uint64 channelID, WatchChannel** channel);
		static int getWatchChannelFromID(uint64 schID, uint64 channelID, WatchChannel** channel);
		const char* getWatchChannelNameFromID(uint64 channelID);
		static const char* getWatchChannelNameFromID(uint64 schID, uint64 channelID);
	private:
		char* serverUID;
		char* serverName;
		uint64 schID; //maybe list so you can be connected to one server in multiple tabs
		vector<WatchChannel> watchChannels;
};

class ChannelWatcher {
public:
	//ChannelWatcher init
	static int init();
	static void shutdown();
	//returns ClientID form own client
	static anyID getOwnClientID(uint64 schID);
	//returns ChannelID of current channel
	static uint64 getOwnChannelID(uint64 schID);
	//checks if a WatchChannel with this ID is contained in WatchChannels list of a the associated server. Returns boolean!
	static bool checkChannel(uint64 schID, uint64 channelID);
	static bool checkServer(uint64 schID);
	//checks if channel is not already on the watchlist and adds it (user) 
	static void selectWatchChannel(uint64 schID, uint64 channelID);
	//checks if channel is on the watchlist and removes it (user)
	static void unselectWatchChannel(uint64 schID, uint64 channelID);
	//Function to add a WatchChannel (internal)
	static unsigned int addChannel(uint64 schID, uint64 channelID, const char* mode);
	static unsigned int removeChannel(uint64 schID, uint64 channelID);
	//Is called to subscribe to a channel and is passed a returncode because of callback identification
	static unsigned int subChannel(uint64 schID, uint64 channelID, const char* rcName);
	//Updates the ChannelInfoData (watchstatus)
	static int channelInfoUpdate(uint64 schID, uint64 channelID);
	static void loadDataFromSaveFile();
	//Is called when connectionStatus is changed or when the client is already connected to a server on plugin init
	static int onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus);
	//Callback function is called after addChannel requested to subscribe to a channel. mode: 0 = add; 1 = load
	static int onAddChannelSubEvent(uint64 schID, unsigned int error, char* chid, int mode);
	//Is called when client requests to unsubscribe from an active watchchannel.
	static int onUnsubSubEvent(uint64 schID, unsigned int error, char* arg);
	//Checks if a client that moved has moved to a watchchannel.
	static void onClientMoveEvent(uint64 schID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage);
	static void onChannelDeletedEvent(uint64 schID, uint64 channelID);
	static void onChannelEditedEvent(uint64 schID, uint64 channelID);
};

extern vector<WatchServer> servers;
#endif // !MAIN_H
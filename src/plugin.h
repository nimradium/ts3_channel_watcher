#pragma once
#ifndef PLUGIN_H
#define PLUGIN_H

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32)
#define PLUGINS_EXPORTDLL __declspec(dllexport)
#else
#define PLUGINS_EXPORTDLL __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif
	#pragma region required_plugin_init
	PLUGINS_EXPORTDLL const char* ts3plugin_name();
	PLUGINS_EXPORTDLL const char* ts3plugin_version();
	PLUGINS_EXPORTDLL int ts3plugin_apiVersion();
	PLUGINS_EXPORTDLL const char* ts3plugin_author();
	PLUGINS_EXPORTDLL const char* ts3plugin_description();
	PLUGINS_EXPORTDLL void ts3plugin_setFunctionPointers(const struct TS3Functions funcs);
	PLUGINS_EXPORTDLL int ts3plugin_init();
	PLUGINS_EXPORTDLL void ts3plugin_shutdown();
	#pragma endregion

	#pragma region optional_plugin_init
	PLUGINS_EXPORTDLL void ts3plugin_registerPluginID(const char* id);
	PLUGINS_EXPORTDLL const char* ts3plugin_infoTitle();
	PLUGINS_EXPORTDLL void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data);
	PLUGINS_EXPORTDLL void ts3plugin_freeMemory(void* data);
	PLUGINS_EXPORTDLL int ts3plugin_requestAutoload();
	PLUGINS_EXPORTDLL const char* ts3plugin_commandKeyword();
	PLUGINS_EXPORTDLL int ts3plugin_processCommand(uint64 serverConnectionHandlerID, const char* command);
	PLUGINS_EXPORTDLL void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon);
	#pragma endregion

	PLUGINS_EXPORTDLL void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
	PLUGINS_EXPORTDLL void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage);
	PLUGINS_EXPORTDLL void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID);
	PLUGINS_EXPORTDLL void ts3plugin_onChannelUnsubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID);
	PLUGINS_EXPORTDLL int ts3plugin_onServerErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, const char* extraMessage);
	PLUGINS_EXPORTDLL void ts3plugin_onDelChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier);
	PLUGINS_EXPORTDLL void ts3plugin_onUpdateChannelEditedEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier);

#ifdef __cplusplus
}
#endif

struct TS3Functions ts3Functions;
char* pluginID;
char pluginPath[PATH_BUFSIZE];
map<string, ReturnCode> returnCodes;

#endif

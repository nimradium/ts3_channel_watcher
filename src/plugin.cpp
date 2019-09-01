#include "common.h"
#include "plugin.h"
#include "channelWatcher.h"
#include "notifier.h"

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
//#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#pragma region required_plugin_init

/*********************************** Required functions ************************************/

const char* ts3plugin_name() {
	return PLUGIN_NAME;
}

const char* ts3plugin_version() {
	return PLUGIN_VERSION;
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
	return PLUGIN_AUTHOR;
}

const char* ts3plugin_description() {
	return PLUGIN_DESCRIPTION;
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

int ts3plugin_init() {
	ChannelWatcher::init();
	return 0;
}

void ts3plugin_shutdown() {
	ChannelWatcher::shutdown();
	if (pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

#pragma endregion

#pragma region optional_plugin_init

/****************************** Optional functions ********************************/

void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);
}
const char* ts3plugin_infoTitle() {
	return "Channel Watcher";
}

void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {
	switch (type) {
	case PLUGIN_SERVER:
		*data = nullptr;
		return;
	case PLUGIN_CHANNEL:
		const char* channelStatus;
		if (ChannelWatcher::checkChannel(serverConnectionHandlerID, id)) {
			channelStatus = "[color=green][b]watching this channel[/b][/color]";
		}
		else {
			channelStatus = "not watched";
		}
		*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  /* Must be allocated in the plugin! */
		snprintf(*data, INFODATA_BUFSIZE, "   %s", channelStatus);
		break;
	case PLUGIN_CLIENT:
		*data = nullptr;
		return;
	default:
		printf("Invalid item type: %d\n", type);
		data = NULL;  /* Ignore */
		return;
	}
}
/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
	free(data);
}

int ts3plugin_requestAutoload() {
	return 0; /* 1 = request autoloaded, 0 = do not request autoload */
}

int ts3plugin_offersConfigure() {
	return PLUGIN_OFFERS_NO_CONFIGURE;
	/*NOT YET IMPLEMENTED*/
}

void ts3plugin_configure(void* handle, void* qParentWidget) {
}

static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}

/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);

enum {
	MENU_CHANNEL_SELECT_CW,
	MENU_CHANNEL_REMOVE_CW
};

void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
	/*
	 * Create the menus
	 * There are three types of menu items:
	 * - PLUGIN_MENU_TYPE_CLIENT:  Client context menu
	 * - PLUGIN_MENU_TYPE_CHANNEL: Channel context menu
	 * - PLUGIN_MENU_TYPE_GLOBAL:  "Plugins" menu in menu bar of main window
	 *
	 * Menu IDs are used to identify the menu item when ts3plugin_onMenuItemEvent is called
	 *
	 * The menu text is required, max length is 128 characters
	 *
	 * The icon is optional, max length is 128 characters. When not using icons, just pass an empty string.
	 * Icons are loaded from a subdirectory in the TeamSpeak client plugins folder. The subdirectory must be named like the
	 * plugin filename, without dll/so/dylib suffix
	 * e.g. for "test_plugin.dll", icon "1.png" is loaded from <TeamSpeak 3 Client install dir>\plugins\test_plugin\1.png
	 */

	BEGIN_CREATE_MENUS(2);  /* IMPORTANT: Number of menu items must be correct! */
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_CHANNEL_SELECT_CW, "Select Watchchannel", "");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_CHANNEL_REMOVE_CW, "Remove Watchchannel", "");
	END_CREATE_MENUS;  /* Includes an assert checking if the number of menu items matched */

	/*
	 * Specify an optional icon for the plugin. This icon is used for the plugins submenu within context and main menus
	 * If unused, set menuIcon to NULL
	 */
	*menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
	_strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, "logo.png");
}


void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
	switch (type) {
		case PLUGIN_MENU_TYPE_CHANNEL:
			switch (menuItemID)
			{
			case MENU_CHANNEL_SELECT_CW:
				ChannelWatcher::selectWatchChannel(serverConnectionHandlerID, selectedItemID);
				break;
			case MENU_CHANNEL_REMOVE_CW:
				ChannelWatcher::unselectWatchChannel(serverConnectionHandlerID, selectedItemID);
				break;
			default:
				break;
		}
	default:
		break;
	}
}

const char* ts3plugin_commandKeyword() {
	return "cw";
}

int ts3plugin_processCommand(uint64 serverConnectionHandlerID, const char* command) {
	char buf[COMMAND_BUFSIZE];
	char* s, * param1 = NULL, * param2 = NULL;
	int i = 0;
	enum { CMD_NONE = 0, CMD_PLUGINID , CMD_NOTIFY, CMD_TEST, CMD_DEBUG} cmd = CMD_NONE;
#ifdef _WIN32
	char* context = NULL;
#endif
	_strcpy(buf, COMMAND_BUFSIZE, command);
#ifdef _WIN32
	s = strtok_s(buf, " ", &context);
#else
	s = strtok(buf, " ");
#endif
	while (s != NULL) {
		if (i == 0) {
			if (!strcmp(s, "pluginid")) {
				cmd = CMD_PLUGINID;
			}
			if (!strcmp(s, "notify")) {
				cmd = CMD_NOTIFY;
			}
			if (!strcmp(s, "test")) {
				cmd = CMD_TEST;
			}
			if (!strcmp(s, "debug")) {
				cmd = CMD_DEBUG;
			}
		}
		else if (i == 1) {
			param1 = s;
		}
		else {
			param2 = s;
		}
#ifdef _WIN32
		s = strtok_s(NULL, " ", &context);
#else
		s = strtok(NULL, " ");
#endif
		i++;
	}

	switch (cmd)
	{
		case CMD_NONE:
			return 1;
		case CMD_PLUGINID:
			ts3Functions.printMessageToCurrentTab(pluginID);
			break;
		case CMD_NOTIFY:

		case CMD_TEST:
			/*char* srvUID;
			if (ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_UNIQUE_IDENTIFIER, &srvUID)!= ERROR_ok) {
				ts3Functions.logMessage("Error querying channel name", LogLevel_ERROR, "Channel Watcher", 0);
				return 1;
			}
			char msg[TS3_MAX_SIZE_TEXTMESSAGE];
			snprintf(msg, TS3_MAX_SIZE_TEXTMESSAGE, "%s", srvUID);
			cw_notification(serverConnectionHandlerID, msg);
			ts3Functions.freeMemory(srvUID);
			break;*/
		case CMD_DEBUG:
			#define DEBUG
			break;
		default:
			break;
	}
	return 0;
}

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
	ChannelWatcher::onConnectStatusChangeEvent(serverConnectionHandlerID, newStatus);
}
void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
	ChannelWatcher::onClientMoveEvent(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, moveMessage);
}
void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
	ChannelWatcher::onClientMoveEvent(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, moveMessage);
}

void ts3plugin_onChannelUnsubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
	if (ChannelWatcher::checkChannel(serverConnectionHandlerID, channelID)) {
		ChannelWatcher::subChannel(serverConnectionHandlerID, channelID, "CW_ONUNSUBSCRIBE");
	}
}

int ts3plugin_onServerErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, const char* extraMessage) {
	try {
		string code(returnCode);
		ReturnCode rc = returnCodes.at(code);
		if (strcmp(rc.name, "CW_ADDCHANNEL_SELECT") == 0) {
			return ChannelWatcher::onAddChannelSubEvent(serverConnectionHandlerID, error, rc.arg, 0);
		}
		else if (strcmp(rc.name, "CW_ADDCHANNEL_LOAD") == 0) {
			return ChannelWatcher::onAddChannelSubEvent(serverConnectionHandlerID, error, rc.arg, 1);
		}
		else if (strcmp(rc.name, "CW_ONUNSUBSCRIBE") == 0) {
			return ChannelWatcher::onUnsubSubEvent(serverConnectionHandlerID, error, rc.arg);
		}
		returnCodes.erase(code);
		return 0;
	}
	catch (exception& e) {
		printf(e.what());
	}
	return 0;
}

void ts3plugin_onDelChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier)
{
	ChannelWatcher::onChannelDeletedEvent(serverConnectionHandlerID, channelID);
	return;
}

void ts3plugin_onUpdateChannelEditedEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier)
{
	ChannelWatcher::onChannelEditedEvent(serverConnectionHandlerID, channelID);
	return;
}

#pragma endregion
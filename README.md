![GitHub All Releases](https://img.shields.io/github/downloads/nimradium/ts3_channel_watcher/total)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/nimradium/ts3_channel_watcher)
![Project status](https://img.shields.io/badge/status-paused-red)
### Project is paused (I will continue fixing reported bugs) until Teamspeak releases the Plugin API for Teamspeak 5
![Channel Watcher](https://raw.githubusercontent.com/nimradium/ts3_channel_watcher/master/doc/channelwatcher.png "Channel Watcher")
by [@nimradium](https://github.com/nimradium)


## Description
ChannelWatcher is a Teamspeak 3 plugin that enables you to select certain channels (e.g. Support-Channels) and notifies you when a user joins this channels. It also saves these channels so they get automatically loaded each time you start Teamspeak and connect to the server.

## Installation
There are two ways to install the plugin. The first way is to use the Teamspeak Package Installer, that should be included in the Teamspeak installation and that should be associated with the .ts3_plugin file extenstion. The second way is to manually copy the plugin files to your plugins folder.
#### Teamspeak Package Installer (Recommended):  
  1. Download `ChannelWatcher.ts3plugin` from the [latest release](https://github.com/nimradium/ts3_channel_watcher/releases/latest).
  2. Install the plugin by double-clicking on the file.
  3. Activate the plugin in the Teamspeak Settings.
#### Manually copying Files:
  1. Download and unpack `ChannelWatcher.zip` from the [latest release](https://github.com/nimradium/ts3_channel_watcher/releases/latest).
  2. Copy the contents of `win64` folder, on a 64 bit system, or `win32` folder, on a 32 bit system, into the plugins folder (should be located at `%APPDATA%\TS3Client\plugins`).
  3. Activate the plugin in the Teamspeak Settings.

## Usage
#### Selecting channels
To add a channel to your Watchlist just select and right-click on a channel and click on `Select Watchchannel` in the Channel Watcher Menu.

#### Deselecting channels
To remove a channel from your Watchlist just select and right-click on a channel and click on `Remove Watchchannel` in the Channel Watcher Menu.

## Planned Features
* GUI
* Configuration

## watchchannels.json
Your watchchannels are saved in `watchchannels.json`. You can edit this file manually but i would not recommend it.

Object structure:
```
{
serverUID:{
  "serverName": serverName,
  "serverUID": serverUID,
  "watchChannels":{
    channelID: {
      "channelID": channelID,
      "channelName": channelName
    },
    ...
  },
...
}
```
`"serverName"` and `"channelName"` are automatically updated so there is no effect in editing those.

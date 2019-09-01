#pragma once
#ifndef NOTIFIER_H
#define NOTIFIER_H
void cw_notification(uint64 schID, const char* text);
void cw_notifyOnJoin(uint64 schID, anyID clientID, uint64 newChannelID);
#endif // !NOTIFIER_H
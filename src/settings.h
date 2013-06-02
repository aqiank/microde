#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <glib.h>

typedef enum _SettingType {
	SETTING_TYPE_STRING,
	SETTING_TYPE_INT,
	SETTING_TYPE_BOOL,
	SETTING_TYPE_HEX,
} SettingType;

typedef struct _Setting {
	const char *key;
	void *value;
	SettingType type;
} Setting;

extern int save_settings(const char *filename, const Setting settings[]);
extern int load_settings(const char *filename, Setting settings[]);

#endif

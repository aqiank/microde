#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "settings.h"

static inline void save_string(FILE *file, const char *key, const void *value)
{
	const char *s = *((const char **)value) != NULL ? *((const char **)value)
							: "";
	fprintf(file, "%s=%s\n", key, s);
}

static inline void save_int(FILE *file, const char *key, const void *value)
{
	fprintf(file, "%s=%d\n", key, *((int *)value));
}

static inline void save_bool(FILE *file, const char *key, const void *value)
{
	fprintf(file, "%s=%s\n", key, *((gboolean *)value) ? "true" : "false");
}

static inline void save_hex(FILE *file, const char *key, const void *value)
{
	fprintf(file, "%s=%x\n", key, *((int *)value));
}

int save_settings(const char *filename, const Setting settings[])
{
	FILE *file;

	file = fopen(filename, "w");
	if (file == NULL) {
		fprintf(stderr, "Could not open or create file for writing\n");
		return -1;
	}

	int i;
	for (i = 0; settings[i].key != NULL; i++) {
		switch(settings[i].type) {
		case SETTING_TYPE_STRING:
			save_string(file, settings[i].key, settings[i].value); break;
		case SETTING_TYPE_INT:
			save_int(file, settings[i].key, settings[i].value); break;
		case SETTING_TYPE_BOOL:
			save_bool(file, settings[i].key, settings[i].value); break;
		case SETTING_TYPE_HEX:
			save_hex(file, settings[i].key, settings[i].value); break;
		}
	}

	fclose(file);
	return 0;
}

static inline void load_string(Setting *setting, const char key[], const char value[])
{
	char **s = (char **) setting->value;
	*s = malloc(strlen(value) + 1);
	strcpy(*s, value);
}

static inline void load_int(Setting *setting, const char key[], const char value[])
{
	int a = atoi(value);
	memcpy(setting->value, &a, sizeof(a));
}

static inline void load_bool(Setting *setting, const char key[], const char value[])
{
	gboolean b = strcmp(value, "true") == 0 ? TRUE : FALSE;
	memcpy(setting->value, &b, sizeof(b));
}

static inline void load_hex(Setting *setting, const char key[], const char value[])
{
	int a = strtol(value, NULL, 16);
	memcpy(setting->value, &a, sizeof(a));
}

static void load_setting(Setting settings[], const char key[], const char value[])
{
	int i;
	for (i = 0; settings[i].key != NULL; i++) {
		if (strcmp(settings[i].key, key) == 0) {
			switch(settings[i].type) {
			case SETTING_TYPE_STRING:
				load_string(&settings[i], key, value); break;
			case SETTING_TYPE_INT:
				load_int(&settings[i], key, value); break;
			case SETTING_TYPE_BOOL:
				load_bool(&settings[i], key, value); break;
			case SETTING_TYPE_HEX:
				load_hex(&settings[i], key, value); break;
			default:
				break;
			}
		}
	}
}

static void parse_settings(char *content, Setting settings[])
{
	char *line;
	char *key;
	char *value;
	char *ptr;
	size_t len;

	line = strtok(content, "\n");
	while (line != NULL) {
		ptr = strchr(line, '=');
		if (ptr == NULL)
			break;
		
		/* get key */
		len = ptr - line; /* length of key */
		key = malloc(len + 1);
		strncpy(key, line, len);
		key[len] = '\0';
		
		/* get value */
		len = strlen(ptr + 1); /* length of value */
		value = malloc(len + 1);
		strcpy(value, ptr + 1);
		value[len] = '\0';
		
		load_setting(settings, key, value);

		free(key);
		free(value);
		line = strtok(NULL, "\n");
	}
}

int load_settings(const char *filename, Setting settings[])
{
	char *content;
	GError *error = NULL;
	
	g_file_get_contents(filename, &content, NULL, &error);
	if (error != NULL) {
		fprintf(stderr, "%s not found: %s\n", filename, error->message);
		g_error_free(error);
		return -1;
	}

	parse_settings(content, settings);
	
	free(content);
	return 0;
}

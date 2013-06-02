#include "microde_settings.h"

#include "util.h"

Setting program_settings[] =
{
	{ "font-family", &font_family, SETTING_TYPE_STRING },
	{ "font-size", &font_size, SETTING_TYPE_INT },
	{ "tab-width", &tab_width, SETTING_TYPE_INT },
	{ "show-line-numbers", &show_line_numbers, SETTING_TYPE_BOOL },
	{ "auto-indent", &auto_indent, SETTING_TYPE_BOOL },
	{ "indent-on-tab", &indent_on_tab, SETTING_TYPE_BOOL },
	{ "style-scheme", &style_scheme, SETTING_TYPE_STRING },
	{ "make-threads", &make_threads, SETTING_TYPE_INT },
	{ NULL }
};

Setting project_settings[] =
{
	{ "microcontroller", &microcontroller, SETTING_TYPE_STRING },
	{ "programmer", &programmer, SETTING_TYPE_STRING },
	{ "syntax", &syntax, SETTING_TYPE_STRING },
	{ "baud-rate", &baud_rate, SETTING_TYPE_STRING },
	{ "serial-port", &serial_port, SETTING_TYPE_STRING },
	{ "clock_speed", &clock_speed, SETTING_TYPE_INT },
	{ "high-fuse", &high_fuse, SETTING_TYPE_HEX },
	{ "low-fuse", &low_fuse, SETTING_TYPE_HEX },
	{ "libraries", &libraries, SETTING_TYPE_STRING },
	{ NULL }
};

char program_settings_filename[] = ".program_settings";
char project_settings_filename[] = ".project_settings";

/* Program settings */
char *	 font_family  = NULL;
int	 font_size;
int	 tab_width;
gboolean show_line_numbers;
gboolean auto_indent;
gboolean indent_on_tab;
gboolean highlight_syntax;
char *	 style_scheme = NULL;
int	 make_threads;

/* Project settings */
char *	microcontroller	  = NULL;
char *	programmer	  = NULL;
char *	syntax		  = NULL;
char *	baud_rate	  = NULL;
char *	serial_port	  = NULL;
int	clock_speed;
int	high_fuse;
int	low_fuse;
char *	libraries	  = NULL;

int save_program_settings(void)
{
	if (project_path == NULL)
		return -1;

	return save_settings(program_settings_filename, program_settings);
}

int save_project_settings(void)
{
	if (project_path == NULL)
		return -1;
	
	return save_settings(project_settings_filename, project_settings);
}

void load_default_program_settings(void)
{
	font_family = "Inconsolata";
	font_size = 10;
	tab_width = 8;
	show_line_numbers = TRUE;
	auto_indent = TRUE;
	indent_on_tab = TRUE;
	highlight_syntax = TRUE;
	style_scheme = "classic";
	make_threads = 4;
}

void load_default_project_settings(void)
{
	microcontroller = "atmega328p";
	programmer = "stk500";
	syntax = "c";
	baud_rate = "115200";
	serial_port = NULL;
	clock_speed = 20000000;
	high_fuse = 0xd9;
	low_fuse = 0x62;
	libraries = NULL;
}

int load_program_settings(void)
{
	load_default_program_settings();

	if (project_path == NULL) {
		fprintf(stderr, "%s not found\n", program_settings_filename);
		return -1;
	}

	return load_settings(program_settings_filename, program_settings);
}

int load_project_settings(void)
{
	load_default_project_settings();

	if (project_path == NULL) {
		fprintf(stderr, "%s not found\n", project_settings_filename);
		return -1;
	}

	return load_settings(project_settings_filename, project_settings);
}

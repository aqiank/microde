#ifndef __MICRODE_SETTINGS_H__
#define __MICRODE_SETTINGS_H__

#include "settings.h"

/* Program settings */
extern char *font_family;
extern int font_size;
extern int tab_width;
extern gboolean show_line_numbers;
extern gboolean auto_indent;
extern gboolean indent_on_tab;
extern gboolean highlight_syntax;
extern char *style_scheme;
extern int make_threads;

/* Project settings */
extern char *microcontroller;
extern char *programmer;
extern char *syntax;
extern char *baud_rate;
extern char *serial_port;
extern int clock_speed;
extern int high_fuse;
extern int low_fuse;
extern char *libraries;

extern Setting program_settings[];
extern Setting project_settings[];

extern char program_settings_filename[];
extern char project_settings_filename[];

extern int save_program_settings(void);
extern int save_project_settings(void);
extern void load_default_program_settings(void);
extern void load_default_project_settings(void);
extern int load_program_settings(void);
extern int load_project_settings(void);

#endif

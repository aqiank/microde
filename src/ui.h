#ifndef __UI_H__
#define __UI_H__

#include <gtk/gtk.h>

struct options;

/* UI creation functions */
extern GArray *create_settings_dialog(GtkWidget *dialog);
extern GtkWidget *create_library_item(GtkWidget *vbox, const char *text);
extern void create_libraries_dialog(GArray *items, GtkWidget *dialog);
extern GtkWidget *create_clock_speed_dialog(GtkWidget *dialog);
extern GArray *create_high_fuse_dialog(GtkWidget *dialog);
extern GArray *create_low_fuse_dialog(GtkWidget *dialog);
extern void create_serial_port_menu();
extern void build_ui(struct options *options);

/* UI update functions */
extern void update_settings(GArray *items);
extern void update_library_list(GArray *items);
extern void update_serial_port_list(void);
extern void update_high_fuse(GArray *items);
extern void update_low_fuse(GArray *items);
extern void update_source_views(void);
extern void update_status_bar(void);

/* UI helper functions */
extern void remove_all_pages(void);
extern GtkWidget *add_page(const char *file_path);
extern void set_current_page_name(const char *file_name);
extern int get_current_page_index(void);
extern int get_num_pages(void);
extern GtkWidget *get_nth_source_view(int n);
extern GtkWidget *get_current_source_view(void);
extern GtkWidget *get_current_page_label(void);
extern const char *get_current_page_name(void);

#endif

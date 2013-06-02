#ifndef __CALLBACKS_GTK_H__
#define __CALLBACKS_GTK_H__

#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>

#include "ui.h"

extern void new_project_cb(GtkButton *button, GtkWindow *window);
extern void open_project_cb(GtkButton *button, GtkWindow *window);
extern void save_project_cb(GtkButton *button, GtkWindow *window);
extern void new_cb(GtkButton *button, GtkWindow *window);
extern void open_cb(GtkButton *button, GtkWindow *window);
extern void save_cb(GtkButton *button, GtkWindow *window);
extern void edit_menu_cb(GtkWidget *edit_menu, gpointer data);
extern void settings_cb(GtkButton *button, GtkWindow *window);
extern void compile_cb(GtkWidget *tool, GtkTextView *output);
extern void upload_cb(GtkWidget *tool, GtkTextView *output);

extern void libraries_cb(GtkButton *button, GtkWindow *window);
extern void library_add_cb(GtkWidget *add, GArray *items);
extern void library_browse_cb(GtkWidget *browse, GtkWindow *data);
extern void library_remove_cb(GtkWidget *remove, gpointer data);

extern void microcontroller_cb(GtkWidget *item, GtkWindow *window);
extern void programmer_cb(GtkWidget *item, GtkWindow *window);
extern void syntax_cb(GtkWidget *item, GtkWindow *window);
extern void baud_rate_cb(GtkWidget *item, GtkWindow *window);
extern void serial_port_cb(GtkWidget *item, gpointer data);
extern void clock_speed_cb(GtkWidget *item, GtkWindow *window);
extern void high_fuse_cb(GtkWidget *item, GtkWindow *window);
extern void low_fuse_cb(GtkWidget *item, GtkWindow *window);
extern void buffer_changed_cb(GtkWidget *buffer, gpointer data);
extern void serial_cb(GtkWidget *tool, GtkWindow *window);
extern void join_serial_thread_cb(GtkWidget *window, GThread *thread);

extern void destroy_cb(GtkWidget *window, gpointer data);

#endif

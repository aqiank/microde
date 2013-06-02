#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>

#include "microde_settings.h"
#include "util.h"
#include "gen_makefile.h"
#include "callbacks_gtk.h"

#define BUF_LEN 64

void new_project_cb(GtkButton *button, GtkWindow *window)
{
	GtkWidget *dialog;
	int result;

	dialog = gtk_file_chooser_dialog_new("Create New Project", window,
					GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN,
					GTK_RESPONSE_ACCEPT,
					NULL);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		char *chosen_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (chosen_path != NULL)
			create_new_project(chosen_path);
	}

	gtk_widget_destroy(dialog);
}

void open_project_cb(GtkButton *button, GtkWindow *window)
{
	GtkWidget *dialog;
	int result;

	dialog = gtk_file_chooser_dialog_new("Open Existing Project", window,
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		char *chosen_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (chosen_path != NULL)
			open_existing_project(chosen_path);
	}

	gtk_widget_destroy(dialog);
}

void save_project_cb(GtkButton * button, GtkWindow * window)
{
	GtkWidget *dialog;
	int result;

	if (project_path != NULL) {
		save_current_project(project_path->str);
		return;
	}

	dialog = gtk_file_chooser_dialog_new("Save Project", window,
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		char *chosen_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (chosen_path != NULL)
			save_current_project(chosen_path);
	}

	gtk_widget_destroy(dialog);
}

void new_cb(GtkButton *button, GtkWindow *window)
{
	add_page("Untitled");
}

void open_cb(GtkButton *button, GtkWindow *window)
{
	GtkWidget *dialog;
	int result;

	dialog = gtk_file_chooser_dialog_new("Open File", window,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL
	);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		char *chosen_file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (chosen_file_path != NULL)
			add_page(chosen_file_path);
	}

	gtk_widget_destroy(dialog);
}

static inline int is_new_file(void)
{
	return !(open_files->len > get_current_page_index());
}

void save_cb(GtkButton *button, GtkWindow *window)
{
	GtkWidget *dialog;
	int result;

	if (get_num_pages() == 0)
		return;

	if (is_new_file() == FALSE) {
		save_current_file();
		return;
	}

	dialog = gtk_file_chooser_dialog_new("Save File", window,
					     GTK_FILE_CHOOSER_ACTION_SAVE,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					     NULL);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		char *chosen_file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (chosen_file_path != NULL)
			save_new_file(chosen_file_path);
	}

	gtk_widget_destroy(dialog);
}

void edit_menu_cb(GtkWidget *edit_menu, gpointer data)
{
	update_serial_port_list();

	if (serial_ports->len == 0)
		return;

	create_serial_port_menu();
}

void settings_cb(GtkButton *button, GtkWindow *window)
{
	GtkWidget *dialog;
	GArray *items;
	int result;

	dialog = gtk_dialog_new_with_buttons("Settings", window, GTK_DIALOG_MODAL,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					     NULL);

	items = create_settings_dialog(dialog);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		update_settings(items);
		update_source_views();
		save_program_settings();
	}

	gtk_widget_destroy(dialog);
}

void libraries_cb(GtkButton *button, GtkWindow *window)
{
	GtkWidget *dialog;
	GArray *items;
	int result;

	dialog = gtk_dialog_new_with_buttons("Libraries", window,
					GTK_DIALOG_MODAL,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					NULL);

	items = g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
	create_libraries_dialog(items, dialog);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		update_library_list(items);
		update_file_lists();
		save_project_settings();
	}

	g_array_free(items, TRUE);
	gtk_widget_destroy(dialog);
}

void library_add_cb(GtkWidget *add, GArray *items)
{
	GtkWidget *parent;
	GtkWidget *item;
	
	parent = gtk_widget_get_parent(add);
	item = create_library_item(parent, "");
	gtk_widget_show_all(parent);
	
	g_array_append_val(items, item);
}

void library_browse_cb(GtkWidget *browse, GtkWindow *window)
{
	GtkWidget *dialog;
	GtkWidget *parent;
	GList *children;
	GtkWidget *entry;
	int result;

	dialog = gtk_file_chooser_dialog_new("Open Existing Project", window,
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

	parent = gtk_widget_get_parent(browse);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		char *chosen_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (chosen_path != NULL) {
			children = gtk_container_get_children(GTK_CONTAINER(parent));
			entry = GTK_WIDGET(g_list_nth_data(children, 0));
			gtk_entry_set_text(GTK_ENTRY(entry), chosen_path);
		}
	}

	gtk_widget_destroy(dialog);
}

void library_remove_cb(GtkWidget *remove, gpointer data)
{
	GtkWidget *parent;

	parent = gtk_widget_get_parent(remove);
	gtk_widget_destroy(parent);
}

void compile_cb(GtkWidget *tool, GtkTextView *output)
{
	FILE *make;
	char tmp[BUF_LEN];
	GtkTextBuffer *text_buffer;
	GtkTextIter end;
	GtkTextMark *mk;

	if (project_path->str == NULL)
		return;

	text_buffer = gtk_text_view_get_buffer(output);

	printf("Generating Makefile..\n");
	generate_makefile();
	printf("Done.\n");
	
	make = popen("make clean 2>&1", "r");
	if (make == NULL) {
		fprintf(stderr, "Failed to call 'make clean'\n");
		return;
	}

	while (fgets(tmp, BUF_LEN, make) != NULL) {
		gtk_text_buffer_get_end_iter(text_buffer, &end);
		gtk_text_buffer_insert(text_buffer, &end, tmp, -1);
	}

	sprintf(tmp, "make -j%d compile 2>&1", make_threads);
	make = popen(tmp, "r");
	if (make == NULL) {
		fprintf(stderr, "Failed to call '%s'\n", tmp);
		return;
	}

	while (fgets(tmp, BUF_LEN, make) != NULL) {
		gtk_text_buffer_get_end_iter(text_buffer, &end);
		gtk_text_buffer_insert(text_buffer, &end, tmp, -1);
	}

	mk = gtk_text_buffer_get_mark (text_buffer, "insert");
	gtk_text_view_scroll_to_mark (output, mk, 0.0, FALSE, 0.0, 0.0);	
}

void upload_cb(GtkWidget *tool, GtkTextView *output)
{
	if (project_path->str == NULL)
		return;

	FILE *make;
	char tmp[BUF_LEN];
	GtkTextBuffer *text_buffer;
	GtkTextIter end;
	GtkTextMark *mk;
	
	text_buffer = gtk_text_view_get_buffer(output);

	make = popen("make upload 2>&1", "r");
	if (make == NULL) {
		fprintf(stderr, "Failed to call 'make clean'\n");
		return;
	}

	while (fgets(tmp, BUF_LEN, make) != NULL) {
		gtk_text_buffer_get_end_iter(text_buffer, &end);
		gtk_text_buffer_insert(text_buffer, &end, tmp, -1);
	}

	mk = gtk_text_buffer_get_mark (text_buffer, "insert");
	gtk_text_view_scroll_to_mark (output, mk, 0.0, FALSE, 0.0, 0.0);	
}

void destroy_cb(GtkWidget *window, gpointer data)
{
	if (project_path == NULL)
		goto out;

	if (save_program_settings() == -1)
		fprintf(stderr, "Failed to save program settings\n");
	if (save_project_settings() == -1)
		fprintf(stderr, "Failed to save project settings\n");
	
out:
	gtk_main_quit();
}

void switch_page_cb(GtkWidget * notebook, GtkWidget * page, unsigned n,
		    gpointer data)
{
}

void microcontroller_cb(GtkWidget *item, GtkWindow *window)
{
	microcontroller = strdup(gtk_menu_item_get_label(GTK_MENU_ITEM(item)));
	update_source_views();
	update_status_bar();
}

void programmer_cb(GtkWidget *item, GtkWindow *window)
{
	programmer = strdup(gtk_menu_item_get_label(GTK_MENU_ITEM(item)));
	update_source_views();
	update_status_bar();
}

void syntax_cb(GtkWidget *item, GtkWindow *window)
{
	syntax = strdup(gtk_menu_item_get_label(GTK_MENU_ITEM(item)));
	update_source_views();
}

void baud_rate_cb(GtkWidget *item, GtkWindow *window)
{
	baud_rate = strdup(gtk_menu_item_get_label(GTK_MENU_ITEM(item)));
	update_status_bar();
}

void serial_port_cb(GtkWidget *item, gpointer data)
{
	serial_port = strdup(gtk_menu_item_get_label(GTK_MENU_ITEM(item)));
	update_status_bar();
}

void clock_speed_cb(GtkWidget *item, GtkWindow *window)
{
	GtkWidget *dialog;
	GtkWidget *spin_button;
	int result;
	
	dialog = gtk_dialog_new_with_buttons("Clock Speed", window,
					GTK_DIALOG_MODAL,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					NULL);

	spin_button = create_clock_speed_dialog(dialog);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT)
		clock_speed = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_button));

	update_status_bar();
	gtk_widget_destroy(dialog);
}

void high_fuse_cb(GtkWidget *item, GtkWindow *window)
{
	GtkWidget *dialog;
	GArray *items;
	int result;
	
	dialog = gtk_dialog_new_with_buttons("High Fuse", window,
					GTK_DIALOG_MODAL,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					NULL);

	items = create_high_fuse_dialog(dialog);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		update_high_fuse(items);
		save_project_settings();
		int i;
		for (i = 0; i < items->len; i++) {
			GtkWidget *item = g_array_index(items, GtkWidget*, i);
			gtk_widget_destroy(item);
		}
	}

	update_status_bar();
	gtk_widget_destroy(dialog);
}

void low_fuse_cb(GtkWidget *item, GtkWindow *window)
{
	GtkWidget *dialog;
	GArray *items;
	int result;
	
	dialog = gtk_dialog_new_with_buttons("Low Fuse", window,
					GTK_DIALOG_MODAL,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
					NULL);

	items = create_low_fuse_dialog(dialog);
	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		update_low_fuse(items);
		save_project_settings();
		int i;
		for (i = 0; i < items->len; i++) {
			GtkWidget *item = g_array_index(items, GtkWidget*, i);
			gtk_widget_destroy(item);
		}
	}

	update_status_bar();
	gtk_widget_destroy(dialog);
}

void buffer_changed_cb(GtkWidget *buffer, gpointer data)
{
	/*
	GtkWidget *label;
	GString *text;

	label = get_current_page_label();
	text = g_string_new(gtk_label_get_text(GTK_LABEL(label)));

	if (strchr(text->str, '*') == NULL) {
		g_string_append(text, "*");
		gtk_label_set_text(GTK_LABEL(label), text->str);
		g_string_free(text, TRUE);
	}
	*/
}

void serial_cb(GtkWidget *tool, GtkWindow *window)
{
	GtkWidget *serial_window;
	GtkWidget *scrolled_window;
	GtkWidget *text_view;
	GtkTextBuffer *buffer;
	GThread *thread;
	
	serial_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	text_view = gtk_text_view_new();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	gtk_window_set_title(GTK_WINDOW(serial_window), "Serial Output");
	gtk_container_set_border_width(GTK_CONTAINER(serial_window), 8);
	gtk_widget_set_size_request(serial_window, 640, 480);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
	gtk_container_add(GTK_CONTAINER(serial_window), scrolled_window);
	gtk_widget_show_all(serial_window);

	read_serial_port = TRUE;
	thread = g_thread_new("Serial", read_serial_into_buffer, buffer);
	g_signal_connect(G_OBJECT(serial_window), "destroy", G_CALLBACK(join_serial_thread_cb), thread);
}

void join_serial_thread_cb(GtkWidget *window, GThread *thread)
{
	FILE *port;

	read_serial_port = FALSE;
	port = (FILE *) g_thread_join(thread);
	if (port != NULL)
		fclose(port);
}

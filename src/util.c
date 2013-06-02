#define _XOPEN_SOURCE 600
#define _GNU_SOURCE
#include <ftw.h>
#include <libgen.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>

#include "microde_settings.h"
#include "callbacks_gtk.h"
#include "gen_makefile.h"
#include "util.h"

#define SERIAL_BUF_LEN 4096

char * find_extension(const char *s1, const char *s2)
{
	char *ptr;

	ptr = strstr(s1, s2);
	if (ptr == NULL)
		return NULL;

	return strcmp(ptr, s2) == 0 ? ptr : NULL;
}

static int find_source_files(const char *file_path, const struct stat *sb, int type,
			struct FTW *ftwbuf)
{
	const char *extensions[] = { ".c", ".cpp", ".S", ".h", ".hpp" };
	const char *tmp;

	int i;
	for (i = 0; i < G_N_ELEMENTS(extensions); i++) {
		if (find_extension(file_path, extensions[i])) {
			tmp = strdup(file_path);
			g_array_append_val(source_files, tmp);
			break;
		}
	}
	
	return 0;
}

static void find_library_source_files(GArray *library_list)
{
	char *tmp;

	int i;
	for (i = 0; i < library_list->len; i++) {
		tmp = g_array_index(library_list, char *, i);
		if (nftw(tmp, find_source_files, 20, FTW_DEPTH) == -1) {
			perror("find_libraries");
			return;
		}
	}
}

static void find_libraries(void)
{
	GArray *library_list;
	char *tmp, *tok;

	if (libraries == NULL)
		return;

	library_list = g_array_new(FALSE, FALSE, sizeof(char *));

	/* convert 'libraries' to array */
	tmp = strdup(libraries);
	tok = strtok(tmp, " ");
	while (tok != NULL) {
		tok = strdup(tok);
		g_array_append_val(library_list, tok);
		tok = strtok(NULL, " ");
	}
	free(tmp);

	find_library_source_files(library_list);
}

static void find_object_files(void)
{
	char *file_path, *ptr;
	const char *extensions[] = { ".c", ".cpp", ".S" };
	
	int i, j;
	for (i = 0; i < source_files->len; i++) {
		file_path = strdup(g_array_index(source_files, char *, i));
		for (j = 0; j < G_N_ELEMENTS(extensions); j++) {
			ptr = find_extension(file_path, extensions[j]);
			if (ptr != NULL) {
				strcpy(ptr, ".o");
				g_array_append_val(object_files, file_path);
			}
		}
	}
}

void update_file_lists(void)
{
	if (source_files != NULL) {
		g_array_free(source_files, TRUE);
		g_array_free(object_files, TRUE);
	}

	source_files = g_array_new(FALSE, FALSE, sizeof(char *));
	object_files = g_array_new(FALSE, FALSE, sizeof(char *));

	if (nftw(project_path->str, find_source_files, 20, FTW_DEPTH) == -1) {
		g_printerr("update_file_lists: ntfw() on %s failed!",
			    project_path->str);
		return;
	}

	find_libraries();
	find_object_files();
}

void set_project_path(const char *chosen_path)
{
	if (project_path != NULL)
		g_string_free(project_path, TRUE);
	
	project_path = g_string_new(chosen_path);
	g_chdir(project_path->str);
}

void create_new_project(const char *chosen_path)
{
	set_project_path(chosen_path);

	if (open_files != NULL)
		g_array_free(open_files, TRUE);

	open_files = g_array_new(FALSE, FALSE, sizeof(GString *));

	remove_all_pages();
	add_page("Untitled");
}

static void load_file_to_source_view(const char *file_path, GtkWidget *source_view)
{
	GtkTextBuffer *text_buffer;
	GError *status;
	char *content;

	status = NULL;
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));

	g_file_get_contents(file_path, &content, NULL, &status);
	if (status != NULL) {
		g_printerr("Could not open %s\n", file_path);
		return;
	}

	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_buffer), content, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(source_view), GTK_TEXT_BUFFER(text_buffer));
}

static void load_files(void)
{
	GtkWidget *source_view;
	const char *file_path;

	/* clear the list of open files and remove all tabs */
	g_array_free(open_files, TRUE);
	open_files = g_array_new(FALSE, FALSE, sizeof(char *));
	remove_all_pages();

	/* only opening source files */
	int i;
	for (i = 0; i < source_files->len; i++) {
		g_array_append_val(open_files, g_array_index(source_files, char *, i));
		file_path = g_array_index(open_files, char *, i);
		if (strstr(file_path, project_path->str) == NULL)
			continue;
		source_view = add_page(file_path);
		load_file_to_source_view(file_path, source_view);
	}
}

void open_existing_project(const char *chosen_path)
{
	set_project_path(chosen_path);
	load_program_settings();
	load_project_settings();
	update_file_lists();
	update_status_bar();
	load_files();
}

void save_current_project(const char *chosen_path)
{
	/* if it's a new project, save it to the chosen path */
	if (project_path == NULL)
		set_project_path(chosen_path);

	save_all_files();
}

static void save_buffer_to_disk(const GtkSourceBuffer *buffer, const char *file_path)
{
	int fd;
	GtkTextIter start;
	GtkTextIter end;
	char *content;

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
	content = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer),
					   &start, &end, TRUE);

	remove(file_path);

	fd = open(file_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
	if (fd == -1) {
		g_printerr("open %s\n", file_path);
		return;
	}

	if (write(fd, content, strlen(content)) == -1) {
		g_printerr("write");
		return;
	}

	close(fd);
}

void save_new_file(const char *chosen_file_path)
{
	GtkWidget *source_view;
	GtkSourceBuffer *buffer;
	GtkWidget *label;
	GString *tmp;
	char *dup;
	char *file_name;
	
	source_view = get_current_source_view();
	buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view)));
	dup = strdup(chosen_file_path);

	save_buffer_to_disk(buffer, chosen_file_path);
	label = get_current_page_label();
	file_name = g_path_get_basename(dup);
	gtk_label_set_text(GTK_LABEL(label), file_name);
	tmp = g_string_new(chosen_file_path);
	g_array_append_val(open_files, tmp);

	free(dup);
}

void save_current_file(void)
{
	GtkWidget *source_view;
	GtkSourceBuffer *buffer;
	GString *file_path;
	int n;

	source_view = get_current_source_view();
	buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view)));

	n = get_current_page_index();
	file_path = g_array_index(open_files, GString*, n);
	save_buffer_to_disk(buffer, file_path->str);
}

void save_all_files(void)
{
	GtkWidget *source_view;
	GtkSourceBuffer *buffer;
	const char *file_path;

	int i, num_pages = get_num_pages();
	for (i = 0; i < num_pages; i++) {
		source_view = get_nth_source_view(i);
		buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view)));
		file_path = g_array_index(open_files, char *, i);
		save_buffer_to_disk(buffer, file_path);
	}
}

void *read_serial_into_buffer(void *buffer_ptr)
{
	FILE *port;
	char serial_buf[SERIAL_BUF_LEN];
	GtkTextBuffer *buffer;
	GtkTextIter end;

	if (serial_port == NULL) {
		fprintf(stderr, "No serial port is selected!\n");
		return NULL;
	}
	
	port = fopen(serial_port, "r");
	if (port == NULL) {
		fprintf(stderr, "Cannot open the serial port: %s\n", serial_port);
		return NULL;
	}
	
	buffer = (GtkTextBuffer*) buffer_ptr;
	if (buffer_ptr == NULL)
		fprintf(stderr, "Buffer is NULL!\n");

	char *ret;
	while (read_serial_port == TRUE) {
		ret = fgets(serial_buf, SERIAL_BUF_LEN, port);
		if (ret == NULL)
			continue;
		gdk_threads_enter();
		gtk_text_buffer_get_end_iter(buffer, &end);
		gtk_text_buffer_insert(buffer, &end, serial_buf, -1);
		gdk_threads_leave();
	}

	return port;
}

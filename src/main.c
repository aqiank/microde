#include <stdlib.h>
#include <gtk/gtk.h>

#include "util.h"
#include "ui.h"
#include "options.h"
#include "microde_settings.h"

static struct options options;
static gchar **cmd_path = NULL;

GArray *open_files = NULL;
GArray *modified_files = NULL;
GArray *serial_ports = NULL;
GString *project_path = NULL;
GArray *source_files = NULL;
GArray *object_files = NULL;
int read_serial_port = FALSE;

static GOptionEntry entries[] =
{
	{ "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &options.fullscreen, "Launch the application in fullscreen mode", NULL },
	{ "maximize", 'm', 0, G_OPTION_ARG_NONE, &options.maximize, "Maximize the application", NULL },
	{ "editor-height", 0, 0, G_OPTION_ARG_INT, &options.editor_height, "Set text editor height", "HEIGHT" },
	{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &cmd_path, "Open a project in this path", NULL },
	{ NULL }
};

static void microde_init(int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;

	/* get command line options and pass them to 'options' */
	context = g_option_context_new ("- an IDE for AVR Microcontrollers");
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_printerr("option parsing failed: %s\n", error->message);
		exit(1);
	}

	gdk_threads_init();
	gtk_init(&argc, &argv);

	open_files = g_array_new(FALSE, FALSE, sizeof(char *));
	serial_ports = g_array_new(FALSE, FALSE, sizeof(GString *));
	modified_files = g_array_new(FALSE, FALSE, sizeof(int));
	
	build_ui(&options);
	
	if (cmd_path != NULL) { /* has command-line options */
		open_existing_project(cmd_path[0]);
	} else { /* no options */
		load_default_program_settings();
		load_default_project_settings();
		add_page("Untitled");
	}

	update_status_bar();
}

static void microde_exit(void)
{
	g_array_free(open_files, TRUE);
	g_array_free(serial_ports, TRUE);
	g_array_free(modified_files, TRUE);
}

int main(int argc, char *argv[])
{
	microde_init(argc, argv);
	gtk_main();
	microde_exit();

	exit(0);
}

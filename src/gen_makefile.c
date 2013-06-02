#include <libgen.h>
#include <stdlib.h>
#include <string.h>

#include "makefile.h"
#include "util.h"
#include "callbacks_gtk.h"
#include "gen_makefile.h"
#include "microde_settings.h"

static void create_source_rules(struct makefile *makefile)
{
	char *object, *source, *ptr;
	GString *component, *recipe;
	const char *extensions[] = { ".c", ".cpp", ".S" };

	int i, j;
	for (i = 0; i < source_files->len; i++) {
		source = g_array_index(source_files, char *, i);
		if (find_extension(source, ".h"))
			continue;

		object = strdup(source);
		object = basename(object);
		for (j = 0; j < G_N_ELEMENTS(extensions); j++) {
			ptr = find_extension(object, extensions[j]);
			if (ptr != NULL) {
				strcpy(ptr, ".o");
				continue;
			}
		}

		component = g_string_new(source);
		recipe = g_string_new("$(CC) $(CFLAGS) -c ");
		g_string_append(recipe, source);
		g_string_append(recipe, " $(INCLUDE)");
		makefile_add_rule(makefile, object, component->str, recipe->str);
	}
}

static char *generate_includes(void)
{
	GString *includes;
	char *tmp;
	char *tok;

	includes = g_string_new(NULL);
	g_string_printf(includes, "-I%s ", project_path->str);

	tmp = strdup(libraries);
	tok = strtok(tmp, " ");
	while (tok != NULL) {
		g_string_append_printf(includes, "-I%s ", tok);
		tok = strtok(NULL, " ");
	}
	free(tmp);

	tmp = includes->str;
	g_string_free(includes, FALSE);

	return tmp;
}

void generate_makefile(void)
{
	struct makefile *makefile;
	char *includes;
	GString *file_path, *object_files_str, *tmp;
	int i;

	if (project_path == NULL)
		return;

	file_path = g_string_new(project_path->str);
	g_string_append(file_path, "/Makefile");
	makefile = makefile_create(file_path->str);

	makefile_add_rule(makefile, "all", "compile upload", NULL);
	makefile_add_macro(makefile, "user", "`whoami`");
	makefile_add_macro(makefile, "CC", "avr-gcc");

	tmp = g_string_new(NULL);
	g_string_printf(tmp, "-O2 -Wall -mmcu=%s -DF_CPU=%dUL",
			microcontroller, clock_speed);
	makefile_add_macro(makefile, "CFLAGS", tmp->str);

	if (libraries != NULL) {
		includes = generate_includes();
		printf("Includes: %s\n", includes);
		makefile_add_macro(makefile, "INCLUDE", includes);
		free(includes);
	}

	update_file_lists();

	object_files_str = g_string_new(NULL);
	for (i = 0; i < object_files->len; i++) {
		char *file_name = g_array_index(object_files, char *, i);
		file_name = basename(file_name);
		g_string_append(object_files_str, file_name);
		g_string_append(object_files_str, " ");
	}
	makefile_add_macro(makefile, "OBJS", object_files_str->str);

	makefile_add_rule(makefile, "compile", "$(OBJS)",
			"$(CC) $(INCLUDE) $(CFLAGS) -o main $(OBJS)\n"
			"\tavr-objcopy -O ihex -R .eeprom main main.hex");
	if (strcmp(programmer, "arduino") == 0) {
		g_string_printf(tmp, "avrdude -c %s -p %s -b %s -P %s "
				"-U flash:w:main.hex", programmer,
				microcontroller, baud_rate, serial_port);

	} else {
		g_string_sprintf(tmp, "avrdude -c %s -p %s -b %s -P %s "
				"-U flash:w:main.hex -U hfuse:w:0x%hhx:m "
				"-U lfuse:w:0x%hhx:m", programmer,
				microcontroller, baud_rate, serial_port,
				high_fuse, low_fuse);
	}

	makefile_add_rule(makefile, "upload", "", tmp->str);
	makefile_add_rule(makefile, "clean", "", "-rm main main.hex *.o");
	create_source_rules(makefile);

	g_string_free(file_path, TRUE);
	g_string_free(object_files_str, TRUE);
	g_string_free(tmp, TRUE);
	makefile_close(makefile);
}

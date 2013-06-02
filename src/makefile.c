#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "makefile.h"

struct makefile *makefile_create(const char *file_path)
{
	struct makefile *makefile;

	makefile = malloc(sizeof(*makefile));
	makefile->buf = g_string_new(NULL);

	remove(file_path);
	makefile->file = fopen(file_path, "w");
	if (makefile->file == NULL) {
		perror("makefile_create");
		return NULL;
	}

	return makefile;
}

void makefile_add_string(struct makefile *makefile, const char *str)
{
	g_string_append(makefile->buf, str);
}

void makefile_add_macro(struct makefile *makefile, const char *macro_name,
			const char *macro_val)
{
	g_string_append(makefile->buf, macro_name);
	g_string_append(makefile->buf, "=");
	g_string_append(makefile->buf, macro_val);
	g_string_append(makefile->buf, "\n");
}

void makefile_add_rule(struct makefile *makefile, const char *target,
		       const char *components, const char *recipe)
{
	g_string_append(makefile->buf, target);
	g_string_append(makefile->buf, ": ");
	g_string_append(makefile->buf, components);
	g_string_append(makefile->buf, "\n");
	if (recipe == NULL)
		return;

	g_string_append(makefile->buf, "\t");
	g_string_append(makefile->buf, recipe);
	g_string_append(makefile->buf, "\n");
}

void makefile_close(struct makefile *makefile)
{
	fwrite(makefile->buf->str, sizeof(*makefile->buf->str), strlen(makefile->buf->str), makefile->file);
	fclose(makefile->file);
	g_string_free(makefile->buf, TRUE);
	free(makefile);
}

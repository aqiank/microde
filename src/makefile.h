#ifndef __MAKEFILE_H__
#define __MAKEFILE_H__

#include <stdio.h>

typedef struct _GString GString;

struct makefile {
	FILE *file;
	GString *buf;
};

extern struct makefile *makefile_create(const char *file_path);
extern void makefile_add_string(struct makefile *makefile, const char *str);
extern void makefile_add_macro(struct makefile *makefile, const char *macro_name,
		const char *macro_val);
extern void makefile_add_rule(struct makefile *makefile, const char *target,
	       const char *components, const char *recipe);
extern void makefile_close(struct makefile *makefile);

#endif

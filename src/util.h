#ifndef __UTIL_H__
#define __UTIL_H__

#include <gtksourceview/gtksourcebuffer.h>

typedef struct _GArray GArray;
typedef struct _GString GString;

extern GArray *open_files;
extern GArray *modified_files;
extern GString *project_path;
extern GArray *header_paths;
extern GArray *source_files;
extern GArray *object_files;
extern GArray *library_paths;
extern GArray *serial_ports;
extern int read_serial_port;

extern char *find_extension(const char *s1, const char *s2);
extern void update_file_lists(void);
extern void set_project_path(const char *chosen_dir_path);
extern void create_new_project(const char *chosen_dir_path);
extern void open_existing_project(const char *chosen_dir_path);
extern void save_current_project(const char *chosen_dir_path);
extern void save_new_file(const char *chosen_file_path);
extern void save_current_file(void);
extern void save_all_files(void);
extern void *read_serial_into_buffer(void *buffer_ptr);

#endif

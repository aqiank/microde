#define _XOPEN_SOURCE 600
#include <ftw.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestyleschememanager.h>

#include "ui.h"
#include "callbacks_gtk.h"
#include "util.h"
#include "microde_settings.h"
#include "options.h"

#define MAX_FONT_SIZE		256
#define MAX_TAB_SIZE 		32
#define GTK_ENTRY_MAX_LENGTH 	512
#define MAX_MAKE_THREADS 	4096
#define MICROCONTROLLER_LIST 	"atmega168 atmega168p atmega328p atmega644p " \
			     	"atmega1280 atmega1281 atmega2560 atmega2561 " \
			     	"atmega32u2 atmega32u4 "
#define PROGRAMMER_LIST 	"arduino avrisp avrispv2 avrispmkii stk500 " \
				"stk500v2 "
#define SYNTAX_LIST 		"c cpp"
#define BAUD_RATE_LIST 		"8 110 150 300 1200 2400 4800 9600 19200 " \
		       		"38400 57600 115200 230400 460800 921600 "

static GtkWidget *main_window;     /* window that contains everything */
static GtkWidget *notebook;	   /* contains all text editor for different files */
static GtkWidget *status_bar;	   /* displays current project settings */
static GtkWidget *serial_port_;	   /* lists all open serial ports */
static GtkWidget *compiler_output; /* prints make / compiler messages */

static void add_microcontrollers(GtkWidget *microcontroller)
{
	char list[] = MICROCONTROLLER_LIST;
	GtkWidget *submenu;
	GtkWidget *item;
	char *token;
	
	submenu	= gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(microcontroller), submenu);

	token = strtok(list, " ");
	while (token != NULL) {
		item = gtk_menu_item_new_with_label(token);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(microcontroller_cb), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
		token = strtok(NULL, " ");
	}
}

static void add_programmers(GtkWidget *programmer)
{
	char list[] = PROGRAMMER_LIST;
	GtkWidget *submenu;
	GtkWidget *item;
	char *token;
	
	submenu	= gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(programmer), submenu);

	token = strtok(list, " ");
	while (token != NULL) {
		item = gtk_menu_item_new_with_label(token);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(programmer_cb), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
		token = strtok(NULL, " ");
	}
}

static void add_syntaxes(GtkWidget *syntax)
{
	char list[] = SYNTAX_LIST;
	GtkWidget *submenu;
	GtkWidget *item;
	char *token;
	
	submenu	= gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(syntax), submenu);

	token = strtok(list, " ");
	while (token != NULL) {
		item = gtk_menu_item_new_with_label(token);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(syntax_cb), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
		token = strtok(NULL, " ");
	}
}

static void add_baud_rates(GtkWidget *baud_rate)
{
	char list[] = BAUD_RATE_LIST;
	GtkWidget *submenu;
	GtkWidget *item;
	char *token;
	
	submenu	= gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(baud_rate), submenu);

	token = strtok(list, " ");
	while (token != NULL) {
		item = gtk_menu_item_new_with_label(token);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(baud_rate_cb), NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
		token = strtok(NULL, " ");
	}
}

static GtkWidget *create_menu_bar(void)
{
	/* Acceleration group */
	GtkAccelGroup *group;

	/* Menu bar */
	GtkWidget *menu_bar;
	GtkWidget *file_menu;
	GtkWidget *edit_menu;
	GtkWidget *help_menu;
	GtkWidget *file;
	GtkWidget *edit;
	GtkWidget *help;

	/* File menu*/
	GtkWidget *new_project;
	GtkWidget *open_project;
	GtkWidget *save_project;
	GtkWidget *new_file;
	GtkWidget *save_file;
	GtkWidget *open_file;

	/* Edit menu */
	GtkWidget *settings;
	GtkWidget *libraries_;
	GtkWidget *microcontroller_;
	GtkWidget *programmer_;
	GtkWidget *syntax_;
	GtkWidget *baud_rate_;
	GtkWidget *clock_speed_;
	GtkWidget *high_fuse_;
	GtkWidget *low_fuse_;

	/* Help menu */
	GtkWidget *about;
	
	/* Initialize menu bar */
	group = gtk_accel_group_new();
	menu_bar = gtk_menu_bar_new();
	file_menu = gtk_menu_new();
	edit_menu = gtk_menu_new();
	help_menu = gtk_menu_new();
	file = gtk_menu_item_new_with_label("File");
	edit = gtk_menu_item_new_with_label("Edit");
	help = gtk_menu_item_new_with_label("Help");

	/* Initialize menu items */
	new_project = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, group);
	open_project = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, group);
	save_project = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, group);
	new_file = gtk_menu_item_new_with_label("New File");
	open_file = gtk_menu_item_new_with_label("Open File");
	save_file = gtk_menu_item_new_with_label("Save File");
	settings = gtk_menu_item_new_with_label("Settings");
	libraries_ = gtk_menu_item_new_with_label("Libraries");
	microcontroller_ = gtk_menu_item_new_with_label("Microcontroller");
	programmer_ = gtk_menu_item_new_with_label("Programmer");
	syntax_ = gtk_menu_item_new_with_label("Syntax");
	baud_rate_ = gtk_menu_item_new_with_label("Baud Rate");
	clock_speed_ = gtk_menu_item_new_with_label("Clock Speed");
	high_fuse_ = gtk_menu_item_new_with_label("High Fuse");
	low_fuse_ = gtk_menu_item_new_with_label("Low Fuse");
	serial_port_ = gtk_menu_item_new_with_label("Serial Port");
	about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, group);

	/* Add menu items */
	add_microcontrollers(microcontroller_);
	add_programmers(programmer_);
	add_syntaxes(syntax_);
	add_baud_rates(baud_rate_);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), file_menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), edit_menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), help_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_project);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_project);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_project);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_file);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_file);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_file);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), settings);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), libraries_);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), microcontroller_);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), programmer_);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), syntax_);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), baud_rate_);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), serial_port_);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), clock_speed_);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), high_fuse_);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), low_fuse_);
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about);

	/* set signals for menu items */
	g_signal_connect(G_OBJECT(new_project), "activate", G_CALLBACK(new_project_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(open_project), "activate", G_CALLBACK(open_project_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(save_project), "activate", G_CALLBACK(save_project_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(new_file), "activate", G_CALLBACK(new_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(open_file), "activate", G_CALLBACK(open_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(save_file), "activate", G_CALLBACK(save_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(settings), "activate", G_CALLBACK(settings_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(libraries_), "activate", G_CALLBACK(libraries_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(edit_menu), "focus", G_CALLBACK(edit_menu_cb), NULL);
	g_signal_connect(G_OBJECT(clock_speed_), "activate", G_CALLBACK(clock_speed_cb), NULL);
	g_signal_connect(G_OBJECT(high_fuse_), "activate", G_CALLBACK(high_fuse_cb), NULL);
	g_signal_connect(G_OBJECT(low_fuse_), "activate", G_CALLBACK(low_fuse_cb), NULL);
	
	gtk_window_add_accel_group(GTK_WINDOW(main_window), group);

	return menu_bar;
}

/* create toolbar */
static GtkWidget *create_toolbar(void)
{
	GtkWidget *toolbar;
	GtkToolItem *open_project_tool;
	GtkToolItem *save_project_tool;
	GtkToolItem *compile_tool;
	GtkToolItem *upload_tool;
	GtkToolItem *serial_tool;
	GtkToolItem *separator;
	GtkWidget *compile_image;
	GtkWidget *upload_image;
	GtkWidget *serial_image;

	toolbar = gtk_toolbar_new();
	open_project_tool = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	save_project_tool = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	separator = gtk_separator_tool_item_new();
	compile_image = gtk_image_new_from_file("compile.png");
	upload_image = gtk_image_new_from_file("upload.png");
	serial_image = gtk_image_new_from_file("serial.png");
	compile_tool = gtk_tool_button_new(compile_image, "Compile");
	upload_tool = gtk_tool_button_new(upload_image, "Upload");
	serial_tool = gtk_tool_button_new(serial_image, "Serial Output");

	/* setup toolbar and add tool items */
	gtk_tool_item_set_tooltip_text(open_project_tool, "Open Project");
	gtk_tool_item_set_tooltip_text(save_project_tool, "Save Project");
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar), TRUE);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), open_project_tool, 0);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), save_project_tool, 1);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, 2);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), compile_tool, 3);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), upload_tool, 4);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), serial_tool, 5);

	/* set signals for tool items */
	g_signal_connect(G_OBJECT(open_project_tool), "clicked", G_CALLBACK(open_project_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(save_project_tool), "clicked", G_CALLBACK(save_project_cb), (gpointer) main_window);
	g_signal_connect(G_OBJECT(compile_tool), "clicked", G_CALLBACK(compile_cb), compiler_output);
	g_signal_connect(G_OBJECT(upload_tool), "clicked", G_CALLBACK(upload_cb), compiler_output);
	g_signal_connect(G_OBJECT(serial_tool), "clicked", G_CALLBACK(serial_cb), main_window);

	return toolbar;
}

/* initialize the source view when the application is starting up */
static GtkWidget *create_source_view(void)
{
	GtkSourceLanguageManager *lang_manager;
	GtkSourceStyleSchemeManager *style_manager;
	GtkSourceLanguage *lang;
	GtkSourceStyleScheme *style;
	GtkSourceBuffer *buffer;
	GtkWidget *source_view;
	PangoFontDescription *font;

	lang_manager = gtk_source_language_manager_get_default();
	style_manager = gtk_source_style_scheme_manager_get_default();
	lang = gtk_source_language_manager_get_language(lang_manager, syntax);
	style = gtk_source_style_scheme_manager_get_scheme(style_manager, style_scheme);
	buffer = gtk_source_buffer_new_with_language(lang);
	source_view = gtk_source_view_new_with_buffer(buffer);
	font = pango_font_description_new();

	/* apply settings from settings */
	gtk_source_buffer_set_style_scheme(buffer, style);
	gtk_source_buffer_set_highlight_syntax(buffer, highlight_syntax);
	pango_font_description_set_family(font, font_family);
	pango_font_description_set_size	(font, font_size * PANGO_SCALE);
	gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(source_view), tab_width);
	gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(source_view), show_line_numbers);
	gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(source_view), auto_indent);
	gtk_source_view_set_indent_on_tab(GTK_SOURCE_VIEW(source_view), indent_on_tab);
	gtk_widget_modify_font(GTK_WIDGET(source_view), font);

	g_signal_connect(G_OBJECT(buffer), "changed", G_CALLBACK(buffer_changed_cb), NULL);

	return source_view;
}

/* helper function to easily create a settings item */
static void create_dialog_item(GtkWidget * dialog, const char *name, GtkWidget * widget)
{
	GtkWidget *hbox;
	GtkWidget *label;

	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(name);
	
	gtk_widget_set_size_request(GTK_WIDGET(label), 160, 32);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox, TRUE, TRUE, 0);
}

GArray *create_settings_dialog(GtkWidget *dialog)
{
	GtkWidget *font_family_;
	GtkWidget *font_size_;
	GtkWidget *tab_width_;
	GtkWidget *show_line_numbers_;
	GtkWidget *auto_indent_;
	GtkWidget *indent_on_tab_;
	GtkWidget *highlight_syntax_;
	GtkWidget *style_scheme_;
	GtkWidget *make_threads_;
	GArray *items;
	
	font_family_ = gtk_entry_new();
	font_size_ = gtk_spin_button_new_with_range(1, MAX_FONT_SIZE, 1);
	tab_width_ = gtk_spin_button_new_with_range(1, MAX_TAB_SIZE, 1);
	show_line_numbers_ = gtk_check_button_new();
	auto_indent_ = gtk_check_button_new();
	indent_on_tab_ = gtk_check_button_new();
	highlight_syntax_ = gtk_check_button_new();
	style_scheme_ = gtk_entry_new();
	make_threads_ = gtk_spin_button_new_with_range(1, MAX_TAB_SIZE, 1);
	items = g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
	
	gtk_entry_set_max_length(GTK_ENTRY(font_family_), GTK_ENTRY_MAX_LENGTH);
	gtk_entry_set_text(GTK_ENTRY(font_family_), font_family);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(font_size_), font_size);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(tab_width_), tab_width);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_line_numbers_), show_line_numbers);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_indent_), auto_indent);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(indent_on_tab_), indent_on_tab);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(highlight_syntax_), highlight_syntax);
	gtk_entry_set_max_length(GTK_ENTRY(font_family_), GTK_ENTRY_MAX_LENGTH);
	gtk_entry_set_text(GTK_ENTRY(style_scheme_), style_scheme);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(make_threads_), make_threads);

	create_dialog_item(dialog, "Font Family", font_family_);
	create_dialog_item(dialog, "Font Size", font_size_);
	create_dialog_item(dialog, "Tab Width", tab_width_);
	create_dialog_item(dialog, "Show Line Numbers", show_line_numbers_);
	create_dialog_item(dialog, "Auto Indent", auto_indent_);
	create_dialog_item(dialog, "Indent on Tab", indent_on_tab_);
	create_dialog_item(dialog, "Syntax Highlighting", highlight_syntax_);
	create_dialog_item(dialog, "Style Scheme", style_scheme_);
	create_dialog_item(dialog, "Make Threads", make_threads_);
	
	g_array_append_val(items, font_family_);
	g_array_append_val(items, font_size_);
	g_array_append_val(items, tab_width_);
	g_array_append_val(items, show_line_numbers_);
	g_array_append_val(items, auto_indent_);
	g_array_append_val(items, indent_on_tab_);
	g_array_append_val(items, highlight_syntax_);
	g_array_append_val(items, style_scheme_);
	g_array_append_val(items, make_threads_);

	return items;
}

GtkWidget *create_library_item(GtkWidget *vbox, const char *text)
{
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *browse;
	GtkWidget *remove;
	
	hbox = gtk_hbox_new(FALSE, 0);
	entry = gtk_entry_new();
	browse = gtk_button_new_with_label("Browse");
	remove = gtk_button_new_with_label("Remove");
	
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);
	gtk_entry_set_max_length(GTK_ENTRY(entry), GTK_ENTRY_MAX_LENGTH);
	gtk_entry_set_text(GTK_ENTRY(entry), text);
	g_signal_connect(G_OBJECT(browse), "clicked", G_CALLBACK(library_browse_cb), main_window);
	g_signal_connect(G_OBJECT(remove), "clicked", G_CALLBACK(library_remove_cb), NULL);
	
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), browse, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), remove, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	return hbox;
}

void create_libraries_dialog(GArray *items, GtkWidget *dialog)
{
	GtkWidget *scrolled_window;
	GtkWidget *vbox;
	GtkWidget *add;
	GtkWidget *item;
	char *tmp;
	char *tok;

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	vbox = gtk_vbox_new(FALSE, 8);
	add = gtk_button_new_with_label("Add Library");

	gtk_widget_set_size_request(GTK_WIDGET(scrolled_window), 640, 480);
	gtk_box_pack_end(GTK_BOX(vbox), add, FALSE, TRUE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
				scrolled_window, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(add), "clicked", G_CALLBACK(library_add_cb), items);

	if (libraries == NULL)
		return;

	tmp = strdup(libraries);
	tok = strtok(tmp, " ");
	while (tok != NULL) {
		item = create_library_item(vbox, tok);
		g_array_append_val(items, item);
		tok = strtok(NULL, " ");
	}

	free(tmp);
}

static int find_ports(const char *file_path, const struct stat *sb, int type,
		     struct FTW *ftwbuf)
{
	GString *tmp = NULL;

	#ifdef __linux__
	if (strstr(file_path, "/dev/ttyUSB") != NULL ||
	    strstr(file_path, "/dev/ttyACM") != NULL)
	#elif __APPLE__ && __MACH__
	if (strstr(file_path, "/dev/tty.usb") != NULL)
	#endif
		tmp = g_string_new(file_path);
	
	if (tmp != NULL)
		g_array_append_val(serial_ports, tmp);

	return 0;
}

void update_serial_port_list(void)
{
	GtkWidget *submenu;

	if (serial_ports->len > 0)
		g_array_remove_range(serial_ports, 0, serial_ports->len);
	
	submenu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(serial_port_));
	if (submenu != NULL)
		gtk_menu_detach(GTK_MENU(submenu));

	#if defined __linux__ || defined __APPLE__ && __MACH__
	if (nftw("/dev", find_ports, 20, FTW_DEPTH) == -1) {
		g_printerr("ntfw() on %s failed!", project_path->str);
		return;
	}
	#endif
}

GtkWidget *create_clock_speed_dialog(GtkWidget *dialog)
{
	GtkWidget *clock_speed_;

	clock_speed_ = gtk_spin_button_new_with_range(1, INT_MAX, 1);
	
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(clock_speed_), clock_speed);
	create_dialog_item(dialog, "Clock Speed", clock_speed_);

	return clock_speed_;
}

GArray *create_high_fuse_dialog(GtkWidget *dialog)
{
	GtkWidget *rstdisbl;
	GtkWidget *dwen;
	GtkWidget *spien;
	GtkWidget *wdton;
	GtkWidget *eesave;
	GtkWidget *bootsz1;
	GtkWidget *bootsz0;
	GtkWidget *bootrst;
	GArray *items;
	
	rstdisbl = gtk_check_button_new();
	dwen = gtk_check_button_new();
	spien = gtk_check_button_new();
	wdton = gtk_check_button_new();
	eesave = gtk_check_button_new();
	bootsz1 = gtk_check_button_new();
	bootsz0 = gtk_check_button_new();
	bootrst = gtk_check_button_new();
	items = g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rstdisbl), (~high_fuse >> 7) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dwen), (~high_fuse >> 6) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spien), (~high_fuse >> 5) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wdton), (~high_fuse >> 4) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(eesave), (~high_fuse >> 3) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bootsz1), (~high_fuse >> 2) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bootsz0), (~high_fuse >> 1) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bootrst), (~high_fuse & 1));

	create_dialog_item(dialog, "RSTDISBL", rstdisbl);
	create_dialog_item(dialog, "DWEN", dwen);
	create_dialog_item(dialog, "SPIEN", spien);
	create_dialog_item(dialog, "WDTON", wdton);
	create_dialog_item(dialog, "EESAVE", eesave);
	create_dialog_item(dialog, "BOOTSZ1", bootsz1);
	create_dialog_item(dialog, "BOOTSZ0", bootsz0);
	create_dialog_item(dialog, "BOOTRST", bootrst);
	
	g_array_append_val(items, rstdisbl);
	g_array_append_val(items, dwen);
	g_array_append_val(items, spien);
	g_array_append_val(items, wdton);
	g_array_append_val(items, eesave);
	g_array_append_val(items, bootsz1);
	g_array_append_val(items, bootsz0);
	g_array_append_val(items, bootrst);

	return items;
}

GArray *create_low_fuse_dialog(GtkWidget *dialog)
{
	GtkWidget *ckdiv8;
	GtkWidget *ckout;
	GtkWidget *sut1;
	GtkWidget *sut0;
	GtkWidget *cksel3;
	GtkWidget *cksel2;
	GtkWidget *cksel1;
	GtkWidget *cksel0;
	GArray *items;
	
	ckdiv8 = gtk_check_button_new();
	ckout = gtk_check_button_new();
	sut1 = gtk_check_button_new();
	sut0 = gtk_check_button_new();
	cksel3 = gtk_check_button_new();
	cksel2 = gtk_check_button_new();
	cksel1 = gtk_check_button_new();
	cksel0 = gtk_check_button_new();
	items = g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ckdiv8), (~low_fuse >> 7) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ckout), (~low_fuse >> 6) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sut1), (~low_fuse >> 5) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sut0), (~low_fuse >> 4) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cksel3), (~low_fuse >> 3) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cksel2), (~low_fuse >> 2) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cksel1), (~low_fuse >> 1) & 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cksel0), (~low_fuse & 1));

	create_dialog_item(dialog, "CKDIV8", ckdiv8);
	create_dialog_item(dialog, "CKOUT", ckout);
	create_dialog_item(dialog, "SUT1", sut1);
	create_dialog_item(dialog, "SUT0", sut0);
	create_dialog_item(dialog, "CKSEL3", cksel3);
	create_dialog_item(dialog, "CKSEL2", cksel2);
	create_dialog_item(dialog, "CKSEL1", cksel1);
	create_dialog_item(dialog, "CKSEL0", cksel0);
	
	g_array_append_val(items, ckdiv8);
	g_array_append_val(items, ckout);
	g_array_append_val(items, sut1);
	g_array_append_val(items, sut0);
	g_array_append_val(items, cksel3);
	g_array_append_val(items, cksel2);
	g_array_append_val(items, cksel1);
	g_array_append_val(items, cksel0);

	return items;
}

void create_serial_port_menu()
{
	GtkWidget *submenu;
	GtkWidget *item;
	GString *tmp;
	
	submenu	= gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(serial_port), submenu);

	int i;
	for (i = 0; i < serial_ports->len; i++) {
		tmp = g_array_index(serial_ports, GString*, i);
		item = gtk_menu_item_new_with_label(tmp->str);
		gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(serial_port_cb), NULL);
	}

	gtk_widget_show_all(GTK_WIDGET(submenu));
}

void update_settings(GArray *items)
{
	GtkWidget *font_family_;
	GtkWidget *font_size_;
	GtkWidget *tab_width_;
	GtkWidget *show_line_numbers_;
	GtkWidget *auto_indent_;
	GtkWidget *indent_on_tab_;
	GtkWidget *highlight_syntax_;
	GtkWidget *style_scheme_;
	GtkWidget *make_threads_;

	font_family_ = g_array_index(items, GtkWidget*, 0);
	font_size_ = g_array_index(items, GtkWidget*, 1);
	tab_width_ = g_array_index(items, GtkWidget*, 2);
	show_line_numbers_ = g_array_index(items, GtkWidget*, 3);
	auto_indent_ = g_array_index(items, GtkWidget*, 4);
	indent_on_tab_ = g_array_index(items, GtkWidget*, 5);
	highlight_syntax_ = g_array_index(items, GtkWidget*, 6);
	style_scheme_ = g_array_index(items, GtkWidget*, 7);
	make_threads_ = g_array_index(items, GtkWidget*, 8);

	char *tmp = gtk_editable_get_chars(GTK_EDITABLE(font_family_), 0, -1);
	font_family = strdup(tmp);
	font_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(font_size_));
	tab_width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(tab_width_));
	show_line_numbers = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_line_numbers_));
	auto_indent = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(auto_indent_));
	indent_on_tab = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(indent_on_tab_));
	highlight_syntax = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(highlight_syntax_));
	make_threads = gtk_spin_button_get_value(GTK_SPIN_BUTTON(make_threads_));
	tmp = gtk_editable_get_chars(GTK_EDITABLE(style_scheme_), 0, -1);
	style_scheme = strdup(tmp);

	free(tmp);
}

void update_library_list(GArray *items)
{
	GtkWidget *hbox;
	GtkWidget *entry;
	GList *list;
	GString *tmp;
	const char *s;

	tmp = g_string_new(NULL);

	int i;
	for (i = 0; i < items->len; i++) {
		hbox = g_array_index(items, GtkWidget*, i);
		list = gtk_container_get_children(GTK_CONTAINER(hbox));
		entry = GTK_WIDGET(g_list_nth_data(list, 0));
		s = gtk_entry_get_text(GTK_ENTRY(entry));
		g_string_append(tmp, s);
		g_string_append(tmp, " ");
	}

	if (libraries != NULL)
		free(libraries);

	libraries = tmp->str;
	g_string_free(tmp, FALSE);
}

void update_high_fuse(GArray *items)
{
	GtkWidget *rstdisbl;
	GtkWidget *dwen;
	GtkWidget *spien;
	GtkWidget *wdton;
	GtkWidget *eesave;
	GtkWidget *bootsz1;
	GtkWidget *bootsz0;
	GtkWidget *bootrst;
	char val;

	rstdisbl = g_array_index(items, GtkWidget*, 0);
	dwen = g_array_index(items, GtkWidget*, 1);
	spien = g_array_index(items, GtkWidget*, 2);
	wdton = g_array_index(items, GtkWidget*, 3);
	eesave = g_array_index(items, GtkWidget*, 4);
	bootsz1 = g_array_index(items, GtkWidget*, 5);
	bootsz0 = g_array_index(items, GtkWidget*, 6);
	bootrst = g_array_index(items, GtkWidget*, 7);

	val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rstdisbl)) << 7 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dwen))	<< 6 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(spien))	<< 5 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdton))	<< 4 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(eesave))	<< 3 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bootsz1))	<< 2 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bootsz0))	<< 1 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bootrst));
	
	high_fuse = ~val;
}

void update_low_fuse(GArray *items)
{
	GtkWidget *ckdiv8;
	GtkWidget *ckout;
	GtkWidget *sut1;
	GtkWidget *sut0;
	GtkWidget *cksel3;
	GtkWidget *cksel2;
	GtkWidget *cksel1;
	GtkWidget *cksel0;
	char val;

	ckdiv8 = g_array_index(items, GtkWidget*, 0);
	ckout = g_array_index(items, GtkWidget*, 1);
	sut1 = g_array_index(items, GtkWidget*, 2);
	sut0 = g_array_index(items, GtkWidget*, 3);
	cksel3 = g_array_index(items, GtkWidget*, 4);
	cksel2 = g_array_index(items, GtkWidget*, 5);
	cksel1 = g_array_index(items, GtkWidget*, 6);
	cksel0 = g_array_index(items, GtkWidget*, 7);

	val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ckdiv8)) << 7 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ckout))	<< 6 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sut1))	<< 5 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sut0))	<< 4 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cksel3))	<< 3 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cksel2))	<< 2 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cksel1))	<< 1 |
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cksel0));
	
	low_fuse = ~val;
}

/* update the source view when user changes settings */
void update_source_views(void)
{
	GtkSourceLanguageManager *	lang_manager;
	GtkSourceStyleSchemeManager *	style_manager;
	GtkSourceLanguage *		lang;
	GtkSourceStyleScheme *		style;
	PangoFontDescription *		font;
	GtkWidget *			source_view;
	GtkWidget *			scrolled_window;
	GtkSourceBuffer *		buffer;

	lang_manager = gtk_source_language_manager_get_default();
	style_manager = gtk_source_style_scheme_manager_get_default();
	lang = gtk_source_language_manager_get_language(lang_manager, syntax);
	style = gtk_source_style_scheme_manager_get_scheme(style_manager, style_scheme);
	font = pango_font_description_new();

	pango_font_description_set_family(font, font_family);
	pango_font_description_set_size(font, font_size * PANGO_SCALE);

	int i, num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
	for (i = 0; i < num_pages; i++) {
		scrolled_window	= gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), i);
		source_view	= gtk_bin_get_child(GTK_BIN(scrolled_window));
		buffer		= GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view)));

		gtk_widget_modify_font(GTK_WIDGET(source_view), font);
		gtk_source_buffer_set_language(buffer, lang);
		gtk_source_buffer_set_style_scheme(buffer, style);
		gtk_source_buffer_set_highlight_syntax(buffer,  highlight_syntax);
		gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(source_view), tab_width);
		gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(source_view), show_line_numbers);
		gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(source_view), auto_indent);
		gtk_source_view_set_indent_on_tab(GTK_SOURCE_VIEW(source_view), indent_on_tab);
	}
}

void remove_all_pages(void)
{
	int num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));

	while (num_pages-- >= 0)
		gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), -1);
}

GtkWidget *add_page(const char *file_path)
{
	GtkWidget *scrolled_window;
	GtkWidget *source_view;
	GtkWidget *label;
	GtkTextBuffer *text_buffer;

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	source_view = create_source_view();
	label = gtk_label_new(basename(strdup(file_path)));
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(source_view));

	gtk_container_add(GTK_CONTAINER(scrolled_window), source_view);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, label);
	g_signal_connect(G_OBJECT(text_buffer), "modified-changed", G_CALLBACK(buffer_changed_cb), NULL);

	gtk_widget_show_all(main_window);
	return source_view;
}

void update_status_bar(void)
{
	GString *status = g_string_new(NULL);

	g_string_printf(status, "Microcontroller: %s | Programmer: %s | "
			"Clock Speed: %.2fMHz | Serial Port: %s | "
			"Baud Rate: %s | High Fuse: 0x%x | Low Fuse: 0x%x",
			microcontroller, programmer, clock_speed / 1000000.0f,
			serial_port, baud_rate, high_fuse, low_fuse);

	gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, status->str);

	g_string_free(status, TRUE);
}

void build_ui(struct options *options)
{
	GtkWidget *vbox;
	GtkWidget *menu_bar;
	GtkWidget *toolbar;
	GtkWidget *scrolled_window;
	GtkWidget *vpaned;

	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new(FALSE, 2);
	vpaned = gtk_vpaned_new();
	notebook = gtk_notebook_new();
	compiler_output = gtk_text_view_new();
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	status_bar = gtk_statusbar_new();
	menu_bar = create_menu_bar();
	toolbar = create_toolbar();

	/* the main window and the top bars */
	if (options->fullscreen)
		gtk_window_fullscreen(GTK_WINDOW(main_window));
	if (options->maximize)
		gtk_window_maximize(GTK_WINDOW(main_window));
	gtk_window_set_title(GTK_WINDOW(main_window), "Microde");
	gtk_container_set_border_width(GTK_CONTAINER(main_window), 8);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 1024, 600);
	gtk_widget_set_size_request(main_window, 320, 180);
	gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);

	/* the text editor and compiler output view */
	int editor_height = (options->editor_height > 0) ? options->editor_height
							 : 600;
	gtk_widget_set_size_request(notebook, 1024, editor_height);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
	gtk_notebook_popup_enable(GTK_NOTEBOOK(notebook));
	gtk_paned_add1(GTK_PANED(vpaned), notebook);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(compiler_output), FALSE);
	gtk_container_add(GTK_CONTAINER(scrolled_window), compiler_output);
	gtk_paned_add2(GTK_PANED(vpaned), scrolled_window);
	gtk_box_pack_start(GTK_BOX(vbox), vpaned, TRUE, TRUE, 0);

	/* the status bar */
	gtk_box_pack_start(GTK_BOX(vbox), status_bar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(main_window), vbox);

	g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(destroy_cb), NULL);
	gtk_widget_show_all(main_window);
}

void set_current_page_name(const char *file_name)
{
	GtkWidget *label = get_current_page_label();

	gtk_label_set_text(GTK_LABEL(label), file_name);
}

int get_current_page_index(void)
{
	return gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
}

int get_num_pages(void)
{
	return gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
}

GtkWidget *get_nth_source_view(int n)
{
	GtkWidget *scrolled_window;
	
	scrolled_window = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), n);

	return gtk_bin_get_child(GTK_BIN(scrolled_window));
}

GtkWidget *get_current_source_view(void)
{
	int page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	GtkWidget *scrolled_window = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page_num);

	return gtk_bin_get_child(GTK_BIN(scrolled_window));
}

GtkWidget *get_current_page_label(void)
{
	int page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	GtkWidget *scrolled_window = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page_num);

	return gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook), scrolled_window);
}

const char *get_current_page_name(void)
{
	GtkWidget *scrolled_window;
	
	int page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	scrolled_window = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page_num);

	return gtk_notebook_get_menu_label_text(GTK_NOTEBOOK(notebook), scrolled_window);
}

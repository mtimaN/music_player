#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>

#include "audio.h"

#define LENGTH 60000.0

extern SDL_AudioDeviceID audio_device;

extern float volume_slider_value;
extern float balance_slider_value;
extern SDL_AudioStream *stream;

typedef struct progress_shower progress_shower;
struct progress_shower {
	GtkWidget *prog_bar;
	GtkWidget *button;
	int pause;
	int length;
};

extern SDL_AudioSpec desired;
GtkWidget *songs_list;
char path[300];

Uint8 *audiobuf = NULL;
Uint32 audiolen = 0;

void myCSS(void);
GtkWidget *new_playlist(const char *name);

GtkWidget *new_song(const char *name)
{
	GtkWidget *n_song, *label;

	n_song = gtk_list_box_row_new();

	char *aux = malloc(100);
	aux = strcpy(aux, name);
	label = gtk_label_new(aux);
	
	gtk_container_add(GTK_CONTAINER(n_song), label);
	
	gtk_widget_set_size_request(GTK_WIDGET(n_song), 150, 50);
	
	gtk_widget_set_name(GTK_WIDGET(n_song), "song");

	return n_song;
}

void on_song_selection(GtkWidget *s_list, GtkListBoxRow *row, gpointer data)
{
	if(path[0] != 0) {
		
		GList *list_itr;
		GtkWidget *label;
		row = gtk_list_box_get_selected_row(GTK_LIST_BOX(songs_list));
		list_itr = gtk_container_get_children(GTK_CONTAINER(row));

		label = GTK_WIDGET(list_itr->data);
		int n = strlen(path);
		strcat(path,"/");
		strcat(path,gtk_label_get_text(GTK_LABEL(label)));

		printf("%s\n", path);
		GtkWidget *prog_bar = ((progress_shower *)data)->prog_bar;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(prog_bar), 0.0);
		((progress_shower *)data)->length = open_new_audio_file(path, &audiobuf, &audiolen) * 1000;
		if (((progress_shower *)data)->length == 0)
			((progress_shower *)data)->length = 1;
		path[n] = '\0';
	}
}

void on_row_selection(GtkWidget *playlists_list, gpointer data)
{	
	const char *home = g_get_home_dir();
	strcpy(path, home);
	strcat(path, "/Music/");

	GtkListBoxRow *row;
	GList *list_itr;
	GtkWidget *label;
	row = gtk_list_box_get_selected_row(GTK_LIST_BOX(playlists_list));
	list_itr = gtk_container_get_children(GTK_CONTAINER(row));

	label = GTK_WIDGET(list_itr->data);
	strcat(path, gtk_label_get_text(GTK_LABEL(label)));
	printf("%s\n", path);
	

	GDir *songs_library = g_dir_open(path, 0, NULL);
	const char *tmp;
	
	tmp = g_dir_read_name(songs_library);
	
	int i = 0;
	if (tmp) {
		
		while(tmp) {

			row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(songs_list), i);
			list_itr = gtk_container_get_children(GTK_CONTAINER(row));

			label = GTK_WIDGET(list_itr->data);
			gtk_label_set_label(GTK_LABEL(label), tmp);

			tmp = g_dir_read_name(songs_library);
			
			i++;
		}
	}
	for (; i < 50; i++) {
		row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(songs_list), i);
		list_itr = gtk_container_get_children(GTK_CONTAINER(row));

		label = GTK_WIDGET(list_itr->data);
		gtk_label_set_label(GTK_LABEL(label), "<Song Slot>");
	}
	
	g_list_free(list_itr);
}

static gboolean inc_progress(gpointer data)
{
	progress_shower *p = data;
	GtkWidget *prog_bar = GTK_WIDGET(p->prog_bar);

	gdouble prog = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(prog_bar));
	if (p->pause) {
		p->pause = -1;
		return FALSE;
	}

	if (prog >= 1.0) {
		prog = 0.0;
		p->pause = -1;
		gtk_button_set_label(GTK_BUTTON(p->button), "Play");
	} else {
		prog += (1000.0/p->length);
	}

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(prog_bar), prog);
}

static void change_main_button_label(GtkWidget *button, gpointer data)
{
	progress_shower *p = data;
	const char *curr_label = gtk_button_get_label(GTK_BUTTON(button));

	if (p->pause) {
		gtk_button_set_label(GTK_BUTTON(button), "Pause");
		g_print("Playing...\n");
		if (p->pause == -1)
			g_timeout_add(1000, inc_progress, p);
		
		SDL_PauseAudioDevice(audio_device, SDL_FALSE);
		Mix_ResumeMusic();
		p->pause = 0;
	} else {
		SDL_PauseAudioDevice(audio_device, SDL_TRUE);
		gtk_button_set_label(GTK_BUTTON(button), "Play");
		Mix_PauseMusic();
		g_print("Paused\n");
		p->pause = 1;
	}
}

void myWindow(GtkWidget **window)
{
	gtk_window_set_default_size(GTK_WINDOW(*window), 1205, 800);

	gtk_window_set_position(GTK_WINDOW(*window), GTK_WIN_POS_CENTER);

	gtk_widget_set_name(GTK_WIDGET(*window), "myWindow");

	gtk_widget_show_all(*window);

	gtk_window_set_title(GTK_WINDOW(*window), "Spotify");

	gtk_window_set_resizable(GTK_WINDOW(*window), FALSE);
}

void myButton(GtkWidget **button)
{
	*button = gtk_button_new_with_label("Play");

	gtk_widget_set_name(GTK_WIDGET(*button), "Play-Pause");

	gtk_widget_set_size_request(GTK_WIDGET(*button), 65, 30);

	gtk_button_set_relief(GTK_BUTTON(*button), GTK_RELIEF_NONE);
}

void mylayout(GtkWidget **fixed, GtkWidget **top_left_box, GtkWidget **left_box, GtkWidget **main_grid) {
	*top_left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	*left_box = gtk_scrolled_window_new(NULL, NULL);
	*main_grid = gtk_scrolled_window_new(NULL, NULL);
	// main list
	gtk_widget_set_size_request(GTK_WIDGET(*main_grid), 985, 650);
	gtk_fixed_put(GTK_FIXED(*fixed), *main_grid, 215, 10);
	gtk_widget_set_name(GTK_WIDGET(*main_grid), "main_grid");
	// top left list
	gtk_widget_set_size_request(GTK_WIDGET(*top_left_box), 200, 100);
	gtk_fixed_put(GTK_FIXED(*fixed), *top_left_box, 5, 10);
	gtk_widget_set_name(GTK_WIDGET(*top_left_box), "top_left_box");
	// left list
	gtk_widget_set_size_request(GTK_WIDGET(*left_box), 200, 540);
	gtk_fixed_put(GTK_FIXED(*fixed), *left_box, 5, 120);
	gtk_widget_set_name(GTK_WIDGET(*left_box), "left_box");
}

void myProgressBar(GtkWidget **prog_bar, GtkWidget **fixed)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(*prog_bar), 0.0);
	gtk_fixed_put(GTK_FIXED(*fixed), *prog_bar, 300, 750);
	gtk_widget_set_size_request(GTK_WIDGET(*prog_bar), 600, 4);
	gtk_widget_set_name(GTK_WIDGET(*prog_bar), "progress_bar");
}

GtkWidget *new_playlist(const char *name)
{
	GtkWidget *new_playlist, *label;

	new_playlist = gtk_list_box_row_new();

	char *aux = malloc(50);
	aux = strcpy(aux, name);
	label = gtk_label_new(aux);
	
	gtk_container_add(GTK_CONTAINER(new_playlist), label);
	
	gtk_widget_set_size_request(GTK_WIDGET(new_playlist), 150, 50)
	gtk_widget_set_name(GTK_WIDGET(new_playlist), "playlist");

	return new_playlist;
}

void detect_playlists(GList *playlists, GtkWidget *playlists_list)
{ 
   	int n = g_list_length(playlists);
	const char *aux;
	GtkWidget *playlist;

	for (int i = 0 ; i < n ; i++) {
		aux = g_list_nth_data(playlists, i);
		playlist = new_playlist(aux);
		
		gtk_container_add(GTK_CONTAINER(playlists_list), playlist);
	}
}

static void activate(GtkApplication *app, gpointer user_data)
{
	// gtk code
	GtkWidget *window, *button, *fixed, *top_left_box, *left_box, *main_grid;
	GtkWidget *prog_bar, *playlists_list, *playlist;
	const char *curr_label;

	progress_shower *p = malloc(sizeof(progress_shower));

	window = gtk_application_window_new(app);
	// main container
	fixed = gtk_fixed_new();

	myButton(&button);

	gtk_fixed_put(GTK_FIXED(fixed), button, 570, 700);

	gtk_container_add(GTK_CONTAINER(window), fixed);

	mylayout(&fixed, &top_left_box, &left_box, &main_grid);

	prog_bar = gtk_progress_bar_new();
	p->prog_bar = prog_bar;
	p->pause = -1;
	p->length = 60000;
	p->button = button;
	myProgressBar(&prog_bar, &fixed);

	playlists_list = gtk_list_box_new();
	gtk_widget_set_name(GTK_WIDGET(playlists_list), "playlists_list");

	gtk_list_box_set_selection_mode(GTK_LIST_BOX(playlists_list), GTK_SELECTION_SINGLE);

	gtk_container_add(GTK_CONTAINER(left_box), playlists_list);

	detect_playlists(user_data, playlists_list);
	GtkWidget *song;
	
	songs_list = gtk_list_box_new();
	gtk_widget_set_name(GTK_WIDGET(songs_list), "songs_list");

	gtk_list_box_set_selection_mode(GTK_LIST_BOX(songs_list), GTK_SELECTION_SINGLE);

	gtk_container_add(GTK_CONTAINER(main_grid), songs_list);
	
	for (int i = 0; i < 50; i++) {
		song = new_song("<Song Slot>");

		gtk_container_add(GTK_CONTAINER(songs_list), song);
	}
	char *path = malloc(300);

	g_signal_connect(button, "clicked", G_CALLBACK(change_main_button_label), p);
	g_signal_connect(playlists_list, "row-selected", G_CALLBACK(on_row_selection), path);
	g_signal_connect(songs_list, "row-selected", G_CALLBACK(on_song_selection), p);

	myWindow(&window);
}

int main(int argc, char **argv)
{	
	init_audio(&audiobuf, &audiolen);
	char *aux = malloc(100);
	const char *tmp;
	strcpy(aux, g_get_home_dir());
	strcat(aux, "/Music");

	GDir *songs_library = g_dir_open(aux, 0, NULL);

	GList *playlists, *new;
	tmp = g_dir_read_name(songs_library);

	while(tmp) {
		aux = strcpy(aux, tmp);

		new = g_list_append(playlists, aux);
		playlists = new;
		aux = malloc(100);
		tmp = g_dir_read_name(songs_library);
	}

	GtkApplication *app;
	
	int ret;
	gtk_init(&argc, &argv);
	myCSS();

	app = gtk_application_new("in.music", G_APPLICATION_FLAGS_NONE);

	g_signal_connect(app, "activate", G_CALLBACK(activate), playlists);
	ret = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);
	deinit_audio(&audiobuf);
	return ret;
}

void myCSS(void){
	GtkCssProvider *provider;
	GdkDisplay *display;
	GdkScreen *screen;

	provider = gtk_css_provider_new ();
	display = gdk_display_get_default ();
	screen = gdk_display_get_default_screen (display);
	gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	const gchar *myCssFile = "data.css";
	GError *error = 0;

	gtk_css_provider_load_from_file(provider, g_file_new_for_path(myCssFile), &error);
	g_object_unref (provider);
}
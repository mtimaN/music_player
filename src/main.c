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
	int pause;
};

void myCSS(void);

static gboolean inc_progress(gpointer data)
{
	progress_shower *p = data;
	GtkWidget *prog_bar = GTK_WIDGET(p->prog_bar);

	gdouble prog = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(prog_bar));
	if (p->pause) {
		p->pause = -1;
		return FALSE;
	}

	if (prog >= 1.0)
		prog = 0.0;
	else
		prog += (1000.0/LENGTH);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(prog_bar), prog);
	printf("11\n");
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
		Mix_HaltMusic();
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
	*main_grid = gtk_grid_new();
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

static void activate(GtkApplication *app, gpointer user_data)
{
	// gtk code
	GtkWidget *window, *button, *fixed, *top_left_box, *left_box, *main_grid;
	GtkWidget *prog_bar, *playlists_list, *playlist1, *playlist2, *playlist3, *playlist4;
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
	myProgressBar(&prog_bar, &fixed);

	playlists_list = gtk_list_box_new();
	gtk_widget_set_name(GTK_WIDGET(playlists_list), "playlists_list");
	// gtk_widget_set_size_request(GTK_WIDGET(playlists_list), 200, 300);

	// gtk_list_box_set_selection_mode(GTK_LIST_BOX(playlists_list), GTK_SELECTION_NONE);

	gtk_container_add(GTK_CONTAINER(left_box), playlists_list);

	playlist2 = gtk_button_new_with_label("2");
	gtk_widget_set_size_request(GTK_WIDGET(playlist2), 150, 200);
	gtk_list_box_insert(GTK_LIST_BOX(playlists_list), playlist2, 0);
	gtk_widget_set_name(GTK_WIDGET(playlist2), "playlist");
	gtk_button_set_relief(GTK_BUTTON(playlist2), GTK_RELIEF_NONE);

	playlist3 = gtk_button_new_with_label("3");
	gtk_widget_set_size_request(GTK_WIDGET(playlist3), 150, 200);
	gtk_list_box_insert(GTK_LIST_BOX(playlists_list), playlist3, 0);
	gtk_widget_set_name(GTK_WIDGET(playlist3), "playlist");
	gtk_button_set_relief(GTK_BUTTON(playlist3), GTK_RELIEF_NONE);

	playlist4 = gtk_button_new_with_label("4");
	gtk_widget_set_size_request(GTK_WIDGET(playlist4), 150, 200);
	gtk_list_box_insert(GTK_LIST_BOX(playlists_list), playlist4, 0);
	gtk_widget_set_name(GTK_WIDGET(playlist4), "playlist");
	gtk_button_set_relief(GTK_BUTTON(playlist4), GTK_RELIEF_NONE);

	playlist1 = gtk_button_new_with_label("1");
	gtk_widget_set_size_request(GTK_WIDGET(playlist1), 150, 200);
	gtk_list_box_insert(GTK_LIST_BOX(playlists_list), playlist1, 0);
	gtk_widget_set_name(GTK_WIDGET(playlist1), "playlist");
	gtk_button_set_relief(GTK_BUTTON(playlist1), GTK_RELIEF_NONE);

	
	// playlists_list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	// gtk_widget_set_name(GTK_WIDGET(playlists_list), "playlists_list");
	// gtk_widget_set_size_request(GTK_WIDGET(playlists_list), 200, 100);

	// gtk_container_add(GTK_CONTAINER(left_box), playlists_list);

	// playlist2 = gtk_button_new_with_label("2");
	// gtk_widget_set_size_request(GTK_WIDGET(playlist2), 200, 50);
	// gtk_box_pack_start(GTK_BOX(playlists_list), playlist2, FALSE, FALSE, 1);

	// playlist3 = gtk_button_new_with_label("3");
	// gtk_widget_set_size_request(GTK_WIDGET(playlist3), 200, 50);
	// gtk_box_pack_start(GTK_BOX(playlists_list), playlist3, FALSE, FALSE, 1);

	// playlist4 = gtk_button_new_with_label("4");
	// gtk_widget_set_size_request(GTK_WIDGET(playlist4), 200, 50);
	// gtk_box_pack_start(GTK_BOX(playlists_list), playlist4, FALSE, FALSE, 1);

	// playlist1 = gtk_button_new_with_label("1");
	// gtk_widget_set_size_request(GTK_WIDGET(playlist1), 200, 50);
	// gtk_box_pack_start(GTK_BOX(playlists_list), playlist1, FALSE, FALSE, 1);

	g_signal_connect(button, "clicked", G_CALLBACK(change_main_button_label), p);

	myWindow(&window);
}

int main(int argc, char **argv)
{
	GDir *test_songs = g_dir_open("../Test_Songs", 0, NULL);
	Uint8 *audiobuf = NULL;
	Uint32 audiolen = 0;
	GtkApplication *app;
	char fname[100] = "../Test_Songs/103-radiohead-subterranean_homesick_alien_(remastered)-faf902be.flac";
	int ret;
	init_everything(&audiobuf, &audiolen, fname);
	gtk_init(&argc, &argv);
	myCSS();

	app = gtk_application_new("in.music", G_APPLICATION_FLAGS_NONE);

	// printf("%s\n", g_dir_read_name(test_songs));

	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	ret = g_application_run(G_APPLICATION(app), argc, argv);

	printf("%s\n", g_dir_read_name(test_songs));
	g_object_unref(app);
	deinit_audio(&audiobuf, strrchr(fname, '.'));
	// gtk_main();
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
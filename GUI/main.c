#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LENGHT 60000.0

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
    // int i = 0;
    gdouble prog = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(prog_bar));
    if (p->pause) {
        p->pause = -1;
        return FALSE;
    }

    if (prog >= 1.0)
        prog = 0.0;
    else
        prog += (1000.0/LENGHT);

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

        p->pause = 0;
    } else {
        gtk_button_set_label(GTK_BUTTON(button), "Play");
        g_print("Paused\n");
        p->pause = 1;
    }
        // g_timeout_add(1000, inc_progress, p);
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
}

void mylayout(GtkWidget **fixed, GtkWidget **top_left_box, GtkWidget **left_box, GtkWidget **main_grid) {
    *top_left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
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

static void activate(GtkApplication *app, gpointer user_data)
{
    // gtk code
    GtkWidget *window, *button, *fixed, *top_left_box, *left_box, *main_grid;
    GtkWidget *prog_bar;
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

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(prog_bar), 0.0);
    gtk_fixed_put(GTK_FIXED(fixed), prog_bar, 300, 750);
    gtk_widget_set_size_request(GTK_WIDGET(prog_bar), 600, 4);
    gtk_widget_set_name(GTK_WIDGET(prog_bar), "progress_bar");

    g_signal_connect(button, "clicked", G_CALLBACK(change_main_button_label), p);

    // const char *curr_label = gtk_button_get_label(GTK_BUTTON(button));
    // if (!strcmp(curr_label, "Play")) {
    //     gtk_button_set_label(GTK_BUTTON(button), "Pause");
        
    // } else {
    //     gtk_button_set_label(GTK_BUTTON(button), "Play");
    //     g_print("Paused\n");
    // }

    myWindow(&window);
}

int main(int argc, char **argv)
{
    GDir *test_songs = g_dir_open("../Test_Songs", 0, NULL);

    GtkApplication *app;
    int ret;
    gtk_init(&argc, &argv);
    myCSS();
    
    app = gtk_application_new("in.music", G_APPLICATION_FLAGS_NONE);

    // printf("%s\n", g_dir_read_name(test_songs));

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    ret = g_application_run(G_APPLICATION(app), argc, argv);

    printf("%s\n", g_dir_read_name(test_songs));
    g_object_unref(app);
    
    // printf("%s\n", g_dir_read_name(test_songs));
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
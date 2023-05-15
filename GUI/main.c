#include <gtk/gtk.h>

void myCSS(void);

static void activate(GtkApplication *app, gpointer user_data)
{
    // gtk code
    GtkWidget *window;

    window = gtk_application_window_new(app);

    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    gtk_widget_set_name(GTK_WIDGET(window), "myWindow");

    gtk_widget_show_all(window);

    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int ret;
    gtk_init(&argc, &argv);
    myCSS();
    
    app = gtk_application_new("in.music", G_APPLICATION_FLAGS_NONE);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    ret = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

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
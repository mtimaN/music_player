#include <gtk/gtk.h>

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    // gtk code comes here

    GtkWidget *window;

    GtkWidget *fixed;

    GtkWidget *button1;
    GtkWidget *button2;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    fixed = gtk_fixed_new();

    gtk_window_set_title(GTK_WINDOW(window), "Spotify");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    
    button1 = gtk_button_new_with_label("button1");
    button2 = gtk_button_new_with_label("button2");

    gtk_widget_set_size_request(GTK_WIDGET(button1), 100, 100);
    
    gtk_fixed_put(GTK_FIXED(fixed), button1, 50, 50);
    gtk_fixed_put(GTK_FIXED(fixed), button2, 100, 50);

    gtk_container_add(GTK_CONTAINER(window), fixed);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main ();

    return 0;
}
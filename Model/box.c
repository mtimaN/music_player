#include <gtk/gtk.h>

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    // gtk code comes here

    GtkWidget *window;

    GtkWidget *box;

    GtkWidget *button1;
    GtkWidget *button2;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0) ;

    gtk_window_set_title(GTK_WINDOW(window), "Spotify");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    button1 = gtk_button_new_with_label("button1");
    gtk_widget_set_size_request(GTK_WIDGET(button1), 100, 100);

    button2 = gtk_button_new_with_label("button2");

    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_box_pack_start(GTK_BOX(box), button1, FALSE, FALSE, 50);
    gtk_box_pack_start(GTK_BOX(box), button2, FALSE, FALSE, 50);

    int is_homo = gtk_box_get_homogeneous(GTK_BOX(box));
    printf("Is homoheneneous? %d\n", is_homo);

    gtk_box_set_homogeneous(GTK_BOX(box), TRUE);

    is_homo = gtk_box_get_homogeneous(GTK_BOX(box));
    printf("Is homoheneneous? %d\n", is_homo);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main ();

    return 0;
}
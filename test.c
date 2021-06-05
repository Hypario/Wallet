#include <stdlib.h>
#include <gtk/gtk.h>

static void cb_ok(GtkWidget *p_wid, gpointer p_data)
{
	GtkWidget *p_dialog = NULL;
	const char *p_text = p_data;

	if (p_text != NULL)
	{
		p_dialog = gtk_message_dialog_new(
			NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			p_text);

		gtk_dialog_run(GTK_DIALOG(p_dialog));
		gtk_widget_destroy(p_dialog);
	}
}

static void cb_quit(GtkWidget *p_wid, gpointer p_data)
{
	gtk_main_quit();
}

int main(int argc, char **argv)
{
	GtkBuilder *p_builder = NULL;
	GError *p_err = NULL;

	/* Initialisation de GTK+ */
	gtk_init(&argc, &argv);

	/* Creation d'un nouveau GtkBuilder */
	p_builder = gtk_builder_new();

	if (p_builder != NULL)
	{
		/* Chargement du XML dans p_builder */
		gtk_builder_add_from_file(p_builder, "test.glade", &p_err);

		if (p_err == NULL)
		{
			/* Recuparation d'un pointeur sur la fenetre. */
			GObject *p_win = gtk_builder_get_object(p_builder, "MainWindow");
			GObject *p_button = gtk_builder_get_object(p_builder, "button");

			g_signal_connect(p_win, "destroy", G_CALLBACK(cb_quit), NULL);

			gtk_widget_show_all(GTK_WIDGET(p_win));
			gtk_main();
		}
		else
		{
			/* Affichage du message d'erreur de GTK+ */
			g_error("%s", p_err->message);
			g_error_free(p_err);
		}
	}

	return EXIT_SUCCESS;
}

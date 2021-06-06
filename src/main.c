#include <stdlib.h>
#include <jansson.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdbool.h>
#include <pthread.h>

GObject *p_builder = NULL;

static GtkWidget *createButton(char *label, GCallback callback, gpointer data)
{
	GtkWidget *buttonBox;
	GtkWidget *button;
	buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);

	button = gtk_button_new_with_label(label);
	g_signal_connect(button, "clicked", callback, data);
	gtk_container_add(GTK_CONTAINER(buttonBox), button);
	return buttonBox;
}

static void change_page(GtkWidget *widget, gpointer page)
{
	GObject *p_notebook = gtk_builder_get_object(p_builder, "Notebook");
	gtk_notebook_set_current_page(p_notebook, gtk_notebook_page_num(p_notebook, page));
}

void setCardPage(GtkWidget *widget, gpointer card)
{
	GObject *p_ImageCard = gtk_builder_get_object(p_builder, "ImageCard");
	GObject *p_LabelCard = gtk_builder_get_object(p_builder, "LabelCard");
	GObject *p_cardPage = gtk_builder_get_object(p_builder, "cardPage");
	char *path = json_string_value(json_object_get(card, "path"));
	char *fullPath = malloc(7 + strlen(path));
	strcpy(fullPath, "Cards/");
	strcat(fullPath, path);
	char *name = json_string_value(json_object_get(card, "name"));

	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(fullPath, NULL);
	pixbuf = gdk_pixbuf_scale_simple(pixbuf, 640, 480, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf(p_ImageCard, pixbuf);
	gtk_label_set_label(p_LabelCard, name);

	change_page(gtk_builder_get_object(p_builder, "Notebook"), p_cardPage);
}

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

void set_navigation()
{
	GObject *p_button_return_card = gtk_builder_get_object(p_builder, "returnButtonCard");
	GObject *p_listCardsPage = gtk_builder_get_object(p_builder, "ListCardsPage");

	g_signal_connect(p_button_return_card, "clicked", G_CALLBACK(change_page), p_listCardsPage);
}

void *handle_fingerprint_detection()
{
	g_print("{'type':'fingerprint', 'action':'scan','value':' '}");
	char *statusmsg = "not ok";
	while (strcmp(statusmsg, "ok") != 0)
	{
		// retry to scan
		g_print("{'type':'fingerprint', 'action':'scan','value':' '}");
		char *statusmsg = "not ok"; // get response
	}
}

void start_fingerprint_scan()
{
	pthread_t fingerprint_th;
	pthread_create(&fingerprint_th, NULL, &handle_fingerprint_detection, NULL);
}

void lock();
static void *handle_registering()
{
	char *status = "fp_release_finger_ok";
	int i = 0;

	while (strcmp(status, "ok") != 0)
	{
		GtkLabel *p_registerLabel = gtk_builder_get_object(p_builder, "registerLabel");
		gtk_label_set_text(p_registerLabel, "Posez votre doigt puis relacher.");
		g_print("posez votre doigt\n");
		if (i < 3)
		{
			i++;
		}
		else
		{
			status = "ok";
		}
	}

	g_print("done\n");

	GtkLabel *p_registerLabel = gtk_builder_get_object(p_builder, "registerLabel");
	gtk_label_set_text(p_registerLabel, "Empreinte enregistré");

	lock();
	return NULL;
}

void lock()
{

	GObject *p_notebook = gtk_builder_get_object(p_builder, "Notebook");
	// check if there's a fingerprint
	// no fingerprint in DB, register one
	// else, ask for a fingerprint

	// lecture
	int response = 0;

	if (response == 0)
	{
		// display register page
		change_page(p_notebook, gtk_builder_get_object(p_builder, "registerPage"));

		g_print("{'type':'fingerprint', 'action':'register','value':' '}");

		// wait for response in another thread to avoid blocking display
		pthread_t register_th;
		pthread_create(&register_th, NULL, &handle_registering, NULL);

		// {"status": true, "message": "[FP] FP Found", "data": 1, "statusmsg":"ok"}

		// {"status": true, "message": "OK. Release finger of reader", "statusmsg":"fp_release_finger_ok"}
	}
	else
	{
		// display unlock page
		change_page(p_notebook, gtk_builder_get_object(p_builder, "unlockPage"));
	}

	start_fingerprint_scan();
}

int main(int argc, char **argv)
{
	GError *p_err = NULL;

	/* Initialisation de GTK+ */
	gdk_threads_init();
	gdk_threads_enter();
	gtk_init(&argc, &argv);

	/* Creation d'un nouveau GtkBuilder */
	p_builder = gtk_builder_new();

	if (p_builder != NULL)
	{
		/* Chargement du XML dans p_builder */
		gtk_builder_add_from_file(p_builder, "views/architecture.glade", &p_err);

		if (p_err == NULL)
		{
			/* Recuperation d'un pointeur sur la fenetre. */
			GObject *p_win = gtk_builder_get_object(p_builder, "MainWindow");
			// TODO : DECOMMENTER
			gtk_window_fullscreen(GTK_WINDOW(p_win));
			GObject *p_ListCards = gtk_builder_get_object(p_builder, "ListCards");

			g_signal_connect(p_win, "destroy", G_CALLBACK(cb_quit), NULL);

			set_navigation();

			json_error_t error;
			size_t index;
			json_t *value;
			json_array_foreach(json_object_get(json_load_file("Cards/cards.json", 0, &error), "all"), index, value)
			{
				gtk_list_box_prepend(p_ListCards, createButton(json_string_value(json_object_get(value, "name")), G_CALLBACK(setCardPage), value));
			};

			lock(); // handle fingerprints

			gtk_widget_show_all(GTK_WIDGET(p_win));
			gtk_main();
		}
		else
		{
			/* Affichage du message d'erreur de GTK+ */
			g_error("%s", p_err->message);
			g_error_free(p_err);
		}

		gdk_threads_leave();
	}

	return EXIT_SUCCESS;
}

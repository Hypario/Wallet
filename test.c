#include <stdlib.h>
#include <jansson.h>
#include <gtk/gtk.h>
#include <stdbool.h>

GObject *p_builder = NULL;

static void click_keyboard(GtkWidget *widget, gpointer key);
static void reset_sensitiveness_keyboard();
static void set_sensitive_key(char key, gboolean sensitive);
static void setCardPage(GtkWidget *widget, gpointer card);
static GtkWidget *createButton(char *label, GCallback callback, gpointer data)
{
	// GtkWidget *buttonBox;
	GtkWidget *button;
	// buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	button = gtk_button_new_with_label(label);

	g_signal_connect(button, "clicked", callback, data);
	// gtk_container_add(GTK_CONTAINER(buttonBox), button);
	// return buttonBox;
	return button;
}
static gboolean list_filter_cards(GtkListBoxRow *row, gpointer len)
{
	if (len == -1)
		return true;
	for (int i = 0; i <= len; i++)
	{
		GObject *p_searchText = gtk_builder_get_object(p_builder, "searchText");
		if (toupper(gtk_button_get_label(gtk_bin_get_child(row))[i]) != gtk_label_get_text(p_searchText)[i])
			return false;
	}
	return true;
}
static void click_keyboard(GtkWidget *widget, gpointer key)
{
	GObject *p_searchText = gtk_builder_get_object(p_builder, "searchText");
	char *str = gtk_label_get_text(p_searchText);
	size_t len = strlen(str);
	char *concat = (char *)malloc(len + 2);
	snprintf(concat, len + 2, "%s%c\0", str, key);
	gtk_label_set_text(p_searchText, concat);
	reset_sensitiveness_keyboard();

	json_error_t error;
	size_t index;
	json_t *value;
	json_array_foreach(json_object_get(json_load_file("Cards/cards.json", 0, &error), "all"), index, value)
	{
		gboolean correct = true;
		for (int i = 0; i <= len; i++)
		{
			if (toupper(json_string_value(json_object_get(value, "name"))[i]) != concat[i])
			{
				correct = false;
				break;
			}
		}
		if (correct)
		{
			set_sensitive_key(json_string_value(json_object_get(value, "name"))[len + 1], TRUE);
		}
	};
	gtk_list_box_set_filter_func(gtk_builder_get_object(p_builder, "ListCards"), (GtkListBoxFilterFunc)list_filter_cards, len, NULL);

	gtk_widget_set_sensitive(gtk_builder_get_object(p_builder, "keyboard_reset"), TRUE);
}
static void click_keyboard_reset()
{
	reset_sensitiveness_keyboard();
	GObject *p_searchText = gtk_builder_get_object(p_builder, "searchText");
	char *str = gtk_label_get_text(p_searchText);
	gtk_label_set_text(p_searchText, "");
	gtk_widget_set_sensitive(gtk_builder_get_object(p_builder, "keyboard_reset"), FALSE);
	json_error_t error;
	size_t index;
	json_t *value;
	json_array_foreach(json_object_get(json_load_file("Cards/cards.json", 0, &error), "all"), index, value)
	{
		set_sensitive_key(json_string_value(json_object_get(value, "name"))[0], TRUE);
	};
	gtk_list_box_set_filter_func(gtk_builder_get_object(p_builder, "ListCards"), (GtkListBoxFilterFunc)list_filter_cards, -1, NULL);
	// faire cet init à chaque switch_page if page = search?
}

static void set_sensitive_key(char key, gboolean sensitive)
{
	char identifier[11] = "keyboard_";
	identifier[strlen(identifier)] = toupper(key);
	GObject *p_button_keyboard_i = gtk_builder_get_object(p_builder, identifier);
	gtk_widget_set_sensitive(p_button_keyboard_i, sensitive);
}
static void reset_sensitiveness_keyboard()
{
	char alphabet[27] = "AZERTYUIOPQSDFGHJKLMWXCVBN";
	for (int i = 0; i < 26; i++)
	{
		set_sensitive_key(alphabet[i], FALSE);
	}
}
static void setupKeyboard()
{
	char alphabet[27] = "AZERTYUIOPQSDFGHJKLMWXCVBN";
	for (int i = 0; i < 26; i++)
	{
		char identifier[11] = "keyboard_";
		identifier[strlen(identifier)] = alphabet[i];
		GObject *p_button_keyboard_i = gtk_builder_get_object(p_builder, identifier);
		g_signal_connect(p_button_keyboard_i, "clicked", G_CALLBACK(click_keyboard), alphabet[i]);
	}
	GObject *p_button_keyboard_reset = gtk_builder_get_object(p_builder, "keyboard_reset");
	g_signal_connect(p_button_keyboard_reset, "clicked", G_CALLBACK(click_keyboard_reset), NULL);
}
static void change_page(GtkWidget *widget, gpointer page)
{
	GObject *p_notebook = gtk_builder_get_object(p_builder, "Notebook");
	gtk_notebook_set_current_page(p_notebook, gtk_notebook_page_num(p_notebook, page));
}

static void setCardPage(GtkWidget *widget, gpointer card)
{
	GObject *p_ImageCard = gtk_builder_get_object(p_builder, "ImageCard");
	GObject *p_LabelCard = gtk_builder_get_object(p_builder, "LabelCard");
	GObject *p_cardPage = gtk_builder_get_object(p_builder, "cardPage");
	char *path = json_string_value(json_object_get(card, "path"));
	char *fullPath = malloc(7 + strlen(path));
	strcpy(fullPath, "Cards/");
	strcat(fullPath, path);
	char *name = json_string_value(json_object_get(card, "name"));

	gtk_image_set_from_file(p_ImageCard, fullPath);
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

int main(int argc, char **argv)
{
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
			/* Recuperation d'un pointeur sur la fenetre. */
			GObject *p_win = gtk_builder_get_object(p_builder, "MainWindow");
			// TODO : DECOMMENTER
			gtk_window_fullscreen(GTK_WINDOW(p_win));
			GObject *p_button_unlock = gtk_builder_get_object(p_builder, "unlock");
			GObject *p_ListCards = gtk_builder_get_object(p_builder, "ListCards");
			GObject *p_button_return_card = gtk_builder_get_object(p_builder, "returnButtonCard");
			GObject *p_listCardsPage = gtk_builder_get_object(p_builder, "ListCardsPage");

			g_signal_connect(p_win, "destroy", G_CALLBACK(cb_quit), NULL);
			g_signal_connect(p_button_unlock, "clicked", G_CALLBACK(change_page), p_listCardsPage);
			g_signal_connect(p_button_return_card, "clicked", G_CALLBACK(change_page), p_listCardsPage);
			setupKeyboard();

			json_error_t error;
			size_t index;
			json_t *value;
			json_array_foreach(json_object_get(json_load_file("Cards/cards.json", 0, &error), "all"), index, value)
			{
				GObject *button = createButton(json_string_value(json_object_get(value, "name")), G_CALLBACK(setCardPage), value);
				gtk_list_box_prepend(p_ListCards, button);
				set_sensitive_key(json_string_value(json_object_get(value, "name"))[0], TRUE);
			};

			change_page(gtk_builder_get_object(p_builder, "Notebook"), gtk_builder_get_object(p_builder, "unlockPage"));
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

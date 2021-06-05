#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <gtk/gtk.h>

static GtkWidget *AddPage(GtkWidget *notebook, GtkWidget *frame, char *szName);

static GtkWidget *createFrameLockScreen();
static GtkWidget *createFrameMenu();
static GtkWidget *createFrameSearch();
void CreateNotebookWindow();
static void CreateNotebook(GtkWidget *window, GtkPositionType pos);
static void unlock_screen(GtkWidget *widget, gpointer data);
static void change_page(GtkWidget *widget, gpointer data);
static void echo_zbeub(GtkWidget *widget, gpointer data);
// static void PageSwitch(GtkWidget *widget, gint page_num); // no use (yet?)

GtkWidget *notebook;
GtkWidget *frameSearch;
GtkWidget *frameLockScreen;
GtkWidget *frameMenu;
// source : https://github.com/steshaw/gtk-examples/blob/master/ch14.clist.tree.tab/notebook/notebook.c
void CreateNotebookWindow(char *szTitle, GtkPositionType pos)
{
    GtkWidget *window;

    /* --- Create the top window --- */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* --- Give the window a title. --- */
    gtk_window_set_title(GTK_WINDOW(window), szTitle);

    /* --- Set the window size. --- */
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 250);

    /* --- Make main page visible --- */
    gtk_widget_show(window);

    /* --- Create notebook --- */
    CreateNotebook(window, pos);
}
// source : https://github.com/steshaw/gtk-examples/blob/master/ch14.clist.tree.tab/notebook/notebook.c
/*
 * CreateNotebook
 *
 * Create a new notebook and add pages to it.
 *
 * window - window to create the notebook in.
 */
static void CreateNotebook(GtkWidget *window,
                           GtkPositionType pos)
{
    GtkWidget *widget;
    GtkWidget *box1;

    /* --- Let us know when getting destroyed. --- */
    g_signal_connect(GTK_CONTAINER(window), "destroy",
                     G_CALLBACK(gtk_widget_destroyed),
                     &window);

    // /* --- Set border width --- */
    // gtk_container_border_width(GTK_CONTAINER(window), 0);

    box1 = gtk_box_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), box1);

    /* --- Create the notebook --- */
    notebook = gtk_notebook_new();

    // no use (yet?)
    // /* --- Listen for the switch page event --- */
    // g_signal_connect(GTK_CONTAINER(notebook), "switch_page",
    //                  G_CALLBACK(PageSwitch), NULL);

    /* --- Make sure tabs are set on correct side --- */
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), pos);

    /* --- Add notebook to vbox --- */
    gtk_box_pack_start(GTK_BOX(box1), notebook, TRUE, TRUE, 0);

    /* --- Add pages to the notebook --- */
    widget = AddPage(notebook, createFrameLockScreen(), "LockScreen");
    widget = AddPage(notebook, createFrameMenu(), "Menu");
    widget = AddPage(notebook, createFrameSearch(), "Recherche");

    /* --- Show everything. --- */
    gtk_widget_show_all(window);
}

// source : https://github.com/steshaw/gtk-examples/blob/master/ch14.clist.tree.tab/notebook/notebook.c
GtkWidget *AddPage(GtkWidget *notebook, GtkWidget *frame, char *szName)
{
    GtkWidget *label;

    /* --- Create a label from the name. --- */
    label = gtk_label_new(szName);
    gtk_widget_show(label);

    /* --- Add a page with the frame and label --- */
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, label);

    return (frame);
}

// no use (yet?)
// static void PageSwitch(GtkWidget *widget,
//                        gint page_num)
// {

//     printf("page switch\n");
// }

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

//// LockScreen
// frame creation

static GtkWidget *createFrameLockScreen()
{
    // GtkWidget *frameLockScreen; // I moved it to global vars
    GtkWidget *buttonBox_Unlock;

    /* --- Create a frame for the page --- */
    frameLockScreen = gtk_frame_new("lockScreen");

    //     //unlock button
    buttonBox_Unlock = createButton("Deverrouiller", G_CALLBACK(unlock_screen), NULL);
    gtk_container_add(GTK_CONTAINER(frameLockScreen), buttonBox_Unlock);
    gtk_widget_show(frameLockScreen);
    return frameLockScreen;
}
// frame callbacks
static void unlock_screen(GtkWidget *widget, gpointer data)
{
    //todo here envoyer acquisition empreinte
    //si NOK try again
    //si OK :
    change_page(NULL, frameMenu);
}

//// Menu
// frame creation

static GtkWidget *createFrameMenu()
{
    // GtkWidget *frameMenu; // i moved it to global vars
    GtkWidget *buttonBox_Search;

    /* --- Create a frame for the page --- */
    frameMenu = gtk_frame_new("lockScreen");

    //     //unlock button
    buttonBox_Search = createButton("Search", G_CALLBACK(change_page), (void *)frameSearch);
    gtk_container_add(GTK_CONTAINER(frameMenu), buttonBox_Search);

    gtk_widget_show(frameMenu);
    return frameMenu;
}

//frame callbacks
static void change_page(GtkWidget *widget, gpointer page)
{
    // todo : here find a way to hide tabs // if no "clean way" is found, remove page?
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), gtk_notebook_page_num(GTK_NOTEBOOK(notebook), page));
}

//// Menu
// frame creation

static GtkWidget *createFrameSearch()
{
    // GtkWidget *frameSearch; // I moved it to global vars
    GtkWidget *buttonBox_Search;

    /* --- Create a frame for the page --- */
    frameSearch = gtk_frame_new("Recherche");

    buttonBox_Search = createButton("Rechercher", G_CALLBACK(echo_zbeub), NULL);
    // todo here : create GtkEntry
    gtk_container_add(GTK_CONTAINER(frameSearch), buttonBox_Search);

    gtk_widget_show(frameSearch);
    return frameSearch;
}

//frame callbacks
static void echo_zbeub(GtkWidget *widget, gpointer data)
{
    g_print("zbeub\n");
}

int main(int argc, char **argv)
{
    // GtkApplication *app;
    int status;
    gtk_init(&argc, &argv);
    CreateNotebookWindow("App", GTK_POS_TOP);

    gtk_main();
    return status;
}
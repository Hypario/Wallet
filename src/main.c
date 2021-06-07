#include <stdlib.h>
#include <jansson.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <stdbool.h>
#include <pthread.h>

// Serial Connectivity 
// 2006-2013, Tod E. Kurt, http://todbot.com/blog/

// Libs Serial 
#include <stdio.h>    // Standard input/output definitions 
#include <unistd.h>   // UNIX standard function definitions 
#include <fcntl.h>    // File control definitions 
#include <errno.h>    // Error number definitions 
#include <termios.h>  // POSIX terminal control definitions 
#include <string.h>   // String function definitions 
#include <sys/ioctl.h>
#include <stdint.h>
int main_fd;
int readcpt = 0;
//  Methods for serial comm up to line 149
int serialport_init(const char* serialport, int baud)
{
    struct termios toptions;
    int fd;
    
    //fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(serialport, O_RDWR | O_NONBLOCK );
    
    if (fd == -1)  {
        perror("serialport_init: Unable to open port ");
        return -1;
    }
    
    //int iflags = TIOCM_DTR;
    //ioctl(fd, TIOCMBIS, &iflags);     // turn on DTR
    //ioctl(fd, TIOCMBIC, &iflags);    // turn off DTR

    if (tcgetattr(fd, &toptions) < 0) {
        perror("serialport_init: Couldn't get term attributes");
        return -1;
    }
    speed_t brate = baud; // let you override switch below if needed
    switch(baud) {
    case 4800:   brate=B4800;   break;
    case 9600:   brate=B9600;   break;
#ifdef B14400
    case 14400:  brate=B14400;  break;
#endif
    case 19200:  brate=B19200;  break;
#ifdef B28800
    case 28800:  brate=B28800;  break;
#endif
    case 38400:  brate=B38400;  break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    //toptions.c_cflag &= ~HUPCL; // disable hang-up-on-close to avoid reset

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 0;
    //toptions.c_cc[VTIME] = 20;
    
    tcsetattr(fd, TCSANOW, &toptions);
    if( tcsetattr(fd, TCSAFLUSH, &toptions) < 0) {
        perror("init_serialport: Couldn't set term attributes");
        return -1;
    }

    return fd;
}

//______________________________________________________________________
int serialport_close( int fd )
{
    return close( fd );
}

//______________________________________________________________________
int serialport_writebyte( int fd, uint8_t b)
{
    int n = write(fd,&b,1);
    if( n!=1)
        return -1;
    return 0;
}

//______________________________________________________________________
int serialport_write(int fd, const char* str)
{
    int len = strlen(str);
    int n = write(fd, str, len);
    if( n!=len ) {
        perror("serialport_write: couldn't write whole string\n");
        return -1;
    }
    return 0;
}

//________________________________________________________________________________
int serialport_read_until(int fd, char* buf, char until, int buf_max, int timeout)
{
    char b[1];  // read expects an array, so we give it a 1-byte array
    int i=0;
    do { 
        int n = read(fd, b, 1);  // read a char at a time
        if( n==-1) return -1;    // couldn't read
        if( n==0 ) {
            usleep( 1 * 1000 );  // wait 1 msec try again
            timeout--;
            if( timeout==0 ) return -2;
            continue;
        }
#ifdef SERIALPORTDEBUG  
        printf("serialport_read_until: i=%d, n=%d b='%c'\n",i,n,b[0]); // debug
#endif
        buf[i] = b[0]; 
        i++;
    } while( b[0] != until && i < buf_max && timeout>0 );

    buf[i] = 0;  // null terminate the string
    return 0;
}

//___________________________________________________________________________
int serialport_flush(int fd)
{
    sleep(2); //required to make flush work, for some reason
    return tcflush(fd, TCIOFLUSH);
}


char * getResponseCode(int fd) {
	int i;
    char buffer[256];	
    //	lecture d'une ligne
	serialport_read_until(fd, buffer, '\n', 255, 10000);
    // suppression de la fin de ligne
    for (i=0 ; buffer[i]!='\n' && i<256 ; i++);
    buffer[i] = 0;

	readcpt+=1;

	if (buffer[0] == '\0') {
		// On recommence la lecture
		return getResponseCode(fd);
	}

	printf("Read %d : %s\n", readcpt, buffer);
    //printf("%s", buffer);
	//printf("%s",buffer);
	char * answer = malloc(sizeof(buffer)*256);
	strncpy(answer,buffer,256);
	
	//return answer;
	return answer;
}






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

void *handle_fingerprint_detection(void * arg)
{
	int fd = (int) arg;

	serialport_write(fd, "{\"type\":\"fingerprint\",\"action\":\"scan\",\"value\":\"\"\n");

	// Message de demande de placement de doigt
	char * msgvfirst =  getResponseCode(fd);
	printf("AIFOJFOEFOID\n");
	printf("\n\n\nscan 1 : %s\n",msgvfirst);
	
	// Vide ?
	//getResponseCode(fd);
	
	// Message OK ou KO
	char * msgsecond = getResponseCode(fd);
	json_error_t error;
    char *statusmsg = json_string_value(json_object_get(json_loads(msgsecond, 0, &error), "statusmsg"));
	
	while (strcmp(statusmsg, "ok") != 0)
	{
		GtkLabel *p_lockLabel = gtk_builder_get_object(p_builder, "lock_label");
		gtk_label_set_text(p_lockLabel, "Empreinte Incorrecte, Merci de réessayer");
		// retry to scan
		serialport_write(fd, "{\"type\":\"fingerprint\",\"action\":\"scan\",\"value\":\"\"\n");

		getResponseCode(fd);
		//getResponseCode(fd);
		char * three = getResponseCode(fd);	



		char *statusmsg = json_string_value(json_object_get(json_loads(msgsecond, 0, &error), "statusmsg"));
	}
	GObject *p_notebook = gtk_builder_get_object(p_builder, "Notebook");
	change_page(p_notebook, gtk_builder_get_object(p_builder, "ListCardsPage"));
	
}

void start_fingerprint_scan(int fd)
{
	pthread_t fingerprint_th;
	pthread_create(&fingerprint_th, NULL, &handle_fingerprint_detection, (void *)fd);
}

void lock();
static void *handle_registering(void * arg)
{
	char *status = "wait";
	int i = 0;

	int fd = (int) arg;
	//printf("%s\n",getResponseCode(fd));

	while (strcmp(status, "ok") != 0)
	{
		GtkLabel *p_registerLabel = gtk_builder_get_object(p_builder, "registerLabel");
		gtk_label_set_text(p_registerLabel, "Posez votre doigt puis relacher.");
		//g_print("posez votre doigt\n");
		sleep(1);
		char * resp = getResponseCode(fd);
		
		json_error_t error;
		char * status =  json_string_value(json_object_get(json_loads(resp, 0, &error), "statusmsg"));
		while (strcmp(status, "ok") != 0) {
			//getResponseCode(fd);
			sleep(1);
			json_error_t error_two;
			char * resptwo = getResponseCode(fd);
			char * statustwo =  json_string_value(json_object_get(json_loads(resptwo, 0, &error_two), "statusmsg"));

			if (strcmp(statustwo, "fp_place_finger") == 0) {
				gtk_label_set_text(p_registerLabel, "Placez votre doigt sur le capteur");	
			} else if (strcmp(statustwo, "fp_release_finger_ok") == 0) {
				gtk_label_set_text(p_registerLabel, "Ôtez votre doigt du capteur");					
			} else if (strcmp(statustwo, "ok") == 0) {
				gtk_label_set_text(p_registerLabel, "OK");
				break;	
			}
		}




	}

	GtkLabel *p_registerLabel = gtk_builder_get_object(p_builder, "registerLabel");
	gtk_label_set_text(p_registerLabel, "Empreinte enregistré");

	//lock();
	return NULL;
}

void lock(int fd)
{

	GObject *p_notebook = gtk_builder_get_object(p_builder, "Notebook");
	// check if there's a fingerprint
	// no fingerprint in DB, register one
	// else, ask for a fingerprint
	serialport_write(fd, "{\"type\":\"fingerprint\",\"action\":\"get_nb\",\"value\":\"\"\n");
	char * rep = getResponseCode(fd);
	printf("%s\n",rep);
	json_error_t error;
	int response = json_object_get(json_loads(rep, 0, &error), "data");


	// lecture

	if (response == 0)
	{
		// display register page
		change_page(p_notebook, gtk_builder_get_object(p_builder, "registerPage"));

		serialport_write(fd, "{\"type\":\"fingerprint\",\"action\":\"register\",\"value\":\"\"}\n");
		// wait for response in another thread to avoid blocking display
		pthread_t register_th;
		pthread_create(&register_th, NULL, &handle_registering, (void *)fd);

		// {"status": true, "message": "[FP] FP Found", "data": 1, "statusmsg":"ok"}

		// {"status": true, "message": "OK. Release finger of reader", "statusmsg":"fp_release_finger_ok"}
	}
	else
	{
		// display unlock page
		change_page(p_notebook, gtk_builder_get_object(p_builder, "unlockPage"));
		start_fingerprint_scan(fd);
	}

	//start_fingerprint_scan(fd);
}

int main(int argc, char **argv)
{
	GError *p_err = NULL;

	// Init Serial
	int fd = serialport_init("/dev/ttyACM0", 9600);
	if (fd == -1) {printf("Unable to start serial !\n"); return -1;}
	// attente, le temps que l'arduino s'init
	sleep(3);
	serialport_flush(fd);

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

			lock(fd); // handle fingerprints

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

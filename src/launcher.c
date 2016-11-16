/*********************************************************************************************************************
/	Autore: 	Carlo Ramponi
/	Data:
/	Consegna:
*********************************************************************************************************************/

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

GtkWidget *window;
GtkWidget *table;
gchar * user1,* user2, * hostn;

void launcher(void);
void opzioni(void);
void utente(GtkWidget*,gpointer);


void callback( GtkWidget *widget, gpointer   data )
{
  /*
    Ovviamente (o almeno lo spero), nella cartella briscola ci saranno i due programmi:
    rete e locale...
  */
  if(strcmp(data,"locale")==0)
  {
    char str[200];
    gtk_main_quit();

    #ifdef _WIN32
      strcpy(str,"cd briscola & start locale.exe ");
      strcat(str,user1);
      strcat(str," ");
      strcat(str,user2);
      system(str);
    #else
      strcpy(str,"cd briscola && ./locale ");
      strcat(str,user1);
      strcat(str," ");
      strcat(str,user2);
      strcat(str," &");
      system(str);
    #endif
  }
	else if(strcmp(data,"rete")==0)
	{
    char str[150];
    #ifdef _WIN32
      strcpy(str,"cd briscola & start client.exe -h ");
      strcat(str,hostn);
      strcat(str," -u ")
      strcat(str,user1);
      system(str);
    #else
      strcpy(str,"cd briscola && ./client -h ");
      strcat(str,hostn);
      strcat(str," -u ");
      strcat(str,user1);
      strcat(str," &");
      system(str);
    #endif
    gtk_main_quit();
	}
  else if(strcmp(data,"opzioni")==0)
  {
    gtk_widget_destroy(table);
    opzioni();
  }
  else
  {
    g_print("Errore...\nUscita...");
    gtk_main_quit();
  }
}

gboolean delete_event( GtkWidget *widget, gpointer   data )
{
	g_print("Uscita...\n");
  gtk_main_quit ();
  return FALSE;
}

void opzioni(void)
{
  g_print("Opzioni...\n");
}

void create_user1(GtkButton *button, gpointer data)
{
  GtkEntry *entry;
  entry=(GtkEntry*)data;
  user1=(gchar*)malloc(strlen(gtk_entry_get_text(entry))*sizeof(gchar));
  strcpy(user1,gtk_entry_get_text(entry));
  //strcpy(user1,"Pippo");
  launcher();
}

void create_user2(GtkButton *button, gpointer data)
{
  GtkEntry *entry;
  entry=(GtkEntry*)data;
  user2=(gchar*)malloc(strlen(gtk_entry_get_text(entry))*sizeof(gchar));
  strcpy(user2,gtk_entry_get_text(entry));
  //strcpy(user1,"Pippo");
  callback(NULL,(gpointer)"locale");
}

void hostname(GtkWidget * widget, gpointer data)
{
  GtkEntry *entry;
  entry=(GtkEntry*)data;
  hostn=(gchar*)malloc(strlen(gtk_entry_get_text(entry))*sizeof(gchar));
  strcpy(hostn,gtk_entry_get_text(entry));
  callback(NULL,(gpointer)"rete");
}

void rete(GtkWidget * widget, gpointer data)
{
  gtk_widget_destroy(table);
  GtkWidget *entry;
  GtkWidget *button;
  GtkWidget *label;
	PangoFontDescription *font_desc;
  table=gtk_table_new(4,2,FALSE);
  gtk_container_add(GTK_CONTAINER(window),table);
  GtkWidget *image;
  label=gtk_label_new("Inserisci l'indirizzo del server:");
  gtk_table_attach(GTK_TABLE(table),label,0,2,2,3,0,0,0,10);
  GdkColor color;
  gdk_color_parse ("white", &color);
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
  font_desc = pango_font_description_from_string("16");
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
  gtk_widget_show(label);
  image=gtk_image_new_from_file("testata/testata (grande).png"); //"testata/testata.png" per l'immagine più piccola
  gtk_table_attach_defaults(GTK_TABLE(table),image,0,2,0,2);
  gtk_widget_show(image);
  entry=gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),entry,0,1,3,4,500,50,5,10);
  button=gtk_button_new_with_label("Gioca!");
  g_signal_connect(button ,"clicked", G_CALLBACK(hostname), (gpointer)entry);
  gtk_table_attach(GTK_TABLE(table),button,1,2,3,4,500,50,5,10);
  gtk_widget_show(button);
	gtk_widget_show(entry);
	gtk_widget_show(table);
}

void launcher(void)
{
  gtk_widget_destroy(table);
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *image;
  image=gtk_image_new_from_file("testata/testata (grande).png"); //"testata/testata.png" per l'immagine più piccola
  table = gtk_table_new (4, 5, FALSE);
  gtk_container_add (GTK_CONTAINER (window), table);
  gtk_table_attach(GTK_TABLE(table),image,0,5,0,2,0,0,0,0);
  gtk_widget_show(image);
  gchar *str;
  gint len=strlen(user1);
  str=(gchar*)malloc((len+6)*sizeof(gchar)); //lunghezza della stringa totale (lunghezza di "ciao, " + lunghezza nome utente)
  strcpy(str,"Ciao, ");
  strcat(str,user1);
  label=gtk_label_new(str);
  PangoFontDescription *font_desc;
  GdkColor color;
  gdk_color_parse ("white", &color);
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
  font_desc = pango_font_description_from_string("16");
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
  gtk_table_attach(GTK_TABLE(table),label,0,5,2,3,0,0,0,5); //mettere 0 come primo numero per far si che la scritta si con allineamento a destra
  gtk_widget_show(label);
  button = gtk_button_new_with_label ("Partita locale");
  g_signal_connect (button, "clicked", GTK_SIGNAL_FUNC(utente), (gpointer)"2");
  gtk_table_attach(GTK_TABLE(table),button,0,2,3,4,500,80,2,2);
  gtk_widget_show (button);
  button = gtk_button_new_with_label ("Partita in rete");
  g_signal_connect (button, "clicked", GTK_SIGNAL_FUNC (rete), NULL);
  gtk_table_attach(GTK_TABLE(table),button,2,4,3,4,500,80,2,2);
  gtk_widget_show (button);
  button = gtk_button_new_with_label ("Opzioni");
  g_signal_connect (button, "clicked", G_CALLBACK (callback),(gpointer) "opzioni");
  gtk_table_attach(GTK_TABLE(table),button,4,5,3,4,100,80,2,2);
  gtk_widget_show (button);
  gtk_widget_show (table);
}


void utente(GtkWidget *widget,gpointer data)
{
  gtk_widget_destroy(table);
  gint n;
  if(strcmp(data,"1")==0)
    n=1;
  else
    n=2;
  GtkWidget *entry;
  GtkWidget *button;
  GtkWidget *label;
	PangoFontDescription *font_desc;
  table=gtk_table_new(4,2,FALSE);
  gtk_container_add(GTK_CONTAINER(window),table);
  GtkWidget *image;
  if(n==1)
    label=gtk_label_new("Inserisci il tuo nome utente:");
  else
    label=gtk_label_new("Inserisci il nome utente del secondo giocatore:");
  gtk_table_attach(GTK_TABLE(table),label,0,2,2,3,0,0,0,10);
  GdkColor color;
  gdk_color_parse ("white", &color);
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
  font_desc = pango_font_description_from_string("16");
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
  gtk_widget_show(label);
  image=gtk_image_new_from_file("testata/testata (grande).png"); //"testata/testata.png" per l'immagine più piccola
  gtk_table_attach_defaults(GTK_TABLE(table),image,0,2,0,2);
  gtk_widget_show(image);
  entry=gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),entry,0,1,3,4,500,50,5,10);
  button=gtk_button_new_with_label("Gioca!");
  if(n==1)
    g_signal_connect(button ,"clicked", G_CALLBACK(create_user1), (gpointer)entry);
  else
    g_signal_connect(button ,"clicked", G_CALLBACK(create_user2), (gpointer)entry);
  gtk_table_attach(GTK_TABLE(table),button,1,2,3,4,500,50,5,10);
  gtk_widget_show(button);
	gtk_widget_show(entry);
	gtk_widget_show(table);
}

int main( int   argc, char *argv[] )
{
	gtk_init(&argc, &argv);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Briscola launcher");
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
  utente(NULL,(gpointer)"1");
  gtk_widget_show (window);
  gtk_main();
  return 0;
}

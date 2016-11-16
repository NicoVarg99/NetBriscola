/*******************************************************************************
 *  Autori: Ramponi Carlo
 *          Salsotto Nicola
 *
 *  Data inizio progetto: 19/04/2016
 *
 *  Nome: NetBriscola
*******************************************************************************/

#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
	#define CLEAR "cls"

#elif __APPLE__
	#define CLEAR "clear"

#elif __linux__
	#define CLEAR "clear"

#endif


GtkWidget * window;
GtkWidget * table;


struct carta{
	int seme;	//seme della carta
	int val;	//numero della carta. Asso=1, 2=2, Tre=3, ecc..
	int eval;	//valore della carta. Asso=11, 2=0, Tre=10 ecc...
	int index;	//ordine della carta nel mazzo. Esempio: Asso di bastoni=16, 2 di coppe=9...
	int win;	//ordine delle carte. Esempio: 1,2,4,5,6,7,fante,cavallo,re,3,asso
};

struct giocatore{
	int punteggio;	//punti totalizzati nella partita attuale
	int vittorie;	//partite vinte in totale
	char nome[20];
	struct carta mano[3];
	int inmano;		//numero di carte che il giocatore ha in mano
};

struct giocatore p[2];
int n; //numero di carte attualmente nel mazzo
struct carta campo[2];		//carte in campo
struct carta scarti[2][40];	//mazzo di carte vinte da ogni giocatore
int scartate[2];	//numero di carte vinte da ogni giocatore
struct carta vuota;
int winner;
struct carta briscola;
struct carta *mazzo;
int primo; //primo giocatore della mano
int turno=0;
int stato=0;	//stato della partita. 1=in corso, 0=finita

void initDeck();
void shufDeck();
void loop(GtkWidget*,gpointer);
void printDeckByIndex(struct carta *);
char *getStringBySuit(int);
char *getStringByCard(int);
int getSuitByIndex(struct carta *,int);
int getCardByIndex(struct carta *,int);
int inverti();
gboolean delete_event( GtkWidget *widget, gpointer   data );
void fine_mano(void);
void pesca(void);
void togli_carta(int);
void inizio(GtkWidget*widget,gpointer data);
void choice(GtkWidget* widget,gpointer data);
void finestra(void);
void startup_msg(GtkWidget*widget,gpointer data);
void endMsg(void);

gboolean delete_event( GtkWidget *widget, gpointer data )
{
	g_print("Uscita...\n");
  gtk_main_quit ();
  return FALSE;
}

int main(int argc, char * argv[])
{
	gtk_init(&argc, &argv);
	vuota.val=-1;

	if(argc<=2)
	{
		printf("\nUso: ./briscola giocatore1 giocatore2\n\n");
		return 0;
	}

	strcpy(p[0].nome,argv[1]);
	strcpy(p[1].nome,argv[2]);




	p[0].vittorie=0;

	p[1].vittorie=0;

	printf("Briscola!\n");
	startup_msg(NULL,NULL);
	gtk_main();

	return 0;
}

void startup_msg(GtkWidget*widget,gpointer data)
{
	stato=inverti(stato);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	gtk_widget_destroy(window);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Briscola");
	//gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);
	//gtk_window_set_default_size(GTK_WINDOW(window),800,500);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	PangoFontDescription *font_desc;
  gdk_color_parse ("white", &color);
  font_desc = pango_font_description_from_string("16");
	GtkWidget *label,*button;
	srand(time(NULL));
	turno=rand()%2;//sceglie casualmente chi deve iniziare la partita
	table=gtk_table_new(4,1,FALSE);
	gtk_container_add(GTK_CONTAINER(window),table);
	label=gtk_label_new("Poiché due giocatori giocheranno sullo stesso PC,");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);
	label=gtk_label_new("è importante che solo il giocatore di turno possa vederlo!");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);
	char *str;
	str=(char*)malloc(sizeof(char)*(strlen(p[turno].nome+10)));
	strcpy(str,"Inizia ");
	strcat(str,p[turno].nome);
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);
	gtk_widget_show(label);
	button=gtk_button_new_with_label("OK");
	gtk_table_attach_defaults(GTK_TABLE(table),button,0,1,3,4);
	g_signal_connect(button,"clicked",GTK_SIGNAL_FUNC(inizio),NULL);
	gtk_widget_show(button);
	gtk_widget_show(table);
}

void endMsg(void)
{
	int i,j;


	//conta i punti
	for(j=0;j<2;j++)
		for(i=0;i<scartate[j];i++)
			p[j].punteggio+=scarti[j][i].eval;

	if(p[0].punteggio>p[1].punteggio)
		winner=0;
	if(p[0].punteggio<p[1].punteggio)
		winner=1;
	if(p[0].punteggio==p[1].punteggio)
		winner=-1;


	//decide il vincitore
	if(winner!=-1)
	{
		p[winner].vittorie++;
	}
	else
	{
		p[1].vittorie++;
		p[0].vittorie++;
	}

	gtk_widget_destroy(window);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Briscola - Fine Partita");
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (window), 20);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_widget_show (window);
	PangoFontDescription *font_desc;
	gdk_color_parse ("white", &color);
	font_desc = pango_font_description_from_string("16");
	GtkWidget *label,*button,*separator;
	GtkWidget *table2;
	table2=gtk_table_new(4,2,FALSE);
	table=gtk_table_new(5,8,TRUE);
	gtk_container_add(GTK_CONTAINER(window),table2);
	gtk_table_attach(GTK_TABLE(table2),table,0,2,1,2,GTK_FILL,GTK_FILL,0,15);
	gtk_widget_show(table2);


	label=gtk_label_new("PARTITA TERMINATA");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach(GTK_TABLE(table2),label,0,2,0,1,GTK_FILL,GTK_FILL,0,0);
	gtk_widget_show(label);

	char *str;

	j=3;
	for(i=0;i<2;i++)
	{
		str=(char*)malloc(sizeof(char)*(strlen(p[i].nome)));
		strcpy(str,p[i].nome);
		label=gtk_label_new(str);
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,j,j+2,0,1);
		gtk_widget_show(label);

		sprintf(str,"%d",p[i].punteggio);
		label=gtk_label_new(str);
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,j,j+2,2,3);
		gtk_widget_show(label);

		sprintf(str,"%d",p[i].vittorie);
		label=gtk_label_new(str);
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,j,j+2,4,5);
		gtk_widget_show(label);

		j+=3;
	}

	label=gtk_label_new("Punti:");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,2,2,3);
	gtk_widget_show(label);

	label=gtk_label_new("Vittorie:");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,2,4,5);
	gtk_widget_show(label);


	if(winner!=-1)
	{
		str=(char*)realloc(str,sizeof(char)*(strlen(p[winner].nome)+30));
		strcpy(str,"Vincitore della partita: ");
		strcat(str,p[winner].nome);
		label=gtk_label_new(str);
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach(GTK_TABLE(table2),label,0,2,2,3,GTK_FILL,GTK_FILL,0,0);
		gtk_widget_show(label);
	}
	else
	{
		label=gtk_label_new("Pareggio!");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach(GTK_TABLE(table2),label,0,2,2,3,GTK_FILL,GTK_FILL,0,0);
		gtk_widget_show(label);
	}

	j=2;
	for(i=0;i<2;i++)
	{
		separator=gtk_vseparator_new();
		gtk_table_attach(GTK_TABLE(table),separator,j,j+1,0,5,GTK_FILL,GTK_FILL,0,0);
		gtk_widget_show(separator);

		j+=3;
	}

	j=1;
	for(i=0;i<2;i++)
	{
		separator=gtk_hseparator_new();
		gtk_table_attach_defaults(GTK_TABLE(table),separator,0,8,j,j+1);
		gtk_widget_show(separator);

		j+=2;
	}

	button=gtk_button_new_with_label("ESCI");
	gtk_table_attach(GTK_TABLE(table2),button,0,1,3,4,GTK_FILL,GTK_FILL,5,15);
	g_signal_connect(GTK_WIDGET(button),"clicked",GTK_SIGNAL_FUNC(delete_event),NULL);
	gtk_widget_show(button);

	button=gtk_button_new_with_label("RIGIOCA");
	gtk_table_attach(GTK_TABLE(table2),button,1,2,3,4,GTK_FILL,GTK_FILL,5,15);
	g_signal_connect(GTK_WIDGET(button),"clicked",GTK_SIGNAL_FUNC(startup_msg),NULL);
	gtk_widget_show(button);

	gtk_widget_show(table);
}

void intraMsg(void)
{
	gtk_widget_destroy(window);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title (GTK_WINDOW (window), "Briscola");
	//gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);
	//gtk_window_set_default_size(GTK_WINDOW(window),800,500);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	PangoFontDescription *font_desc;
	gdk_color_parse ("white", &color);
	font_desc = pango_font_description_from_string("16");
	GtkWidget *label,*button,*image;
	table=gtk_table_new(5,2,FALSE);
	gtk_container_add(GTK_CONTAINER(window),table);
	label=gtk_label_new("Carta giocata");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);
	char *str;
	str=(char*)malloc(sizeof(char)*(strlen(p[turno].nome+5)));
	strcpy(str,"da ");
	strcat(str,p[turno].nome);
	strcat(str,":");
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	str=(char*)malloc(sizeof(char)*(strlen(p[turno].nome+20)));
	strcpy(str,"Passa il PC a ");
	strcat(str,p[inverti(turno)].nome);
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,2,3,4);
	gtk_widget_show(label);

	label=gtk_label_new("Briscola:");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,0,2);
	gtk_widget_show(label);

	button=gtk_button_new_with_label("OK");
	gtk_table_attach(GTK_TABLE(table),button,0,2,4,5,GTK_FILL,0,5,10);
	g_signal_connect(button,"clicked",GTK_SIGNAL_FUNC(loop),NULL);
	gtk_widget_show(button);

	char buffer[5];

	strcpy(str,"../carte/");
	strcat(str,getStringBySuit(briscola.seme));
	strcat(str,"/");
	sprintf(buffer, "%d", briscola.val);
	strcat(str,buffer);
	strcat(str,".png");
	image=gtk_image_new_from_file(str);

	button=gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(button),image);
	gtk_table_attach(GTK_TABLE(table),button,1,2,2,3,0,0,15,10);
	gtk_widget_show(button);

	strcpy(str,"../carte/");
	strcat(str,getStringBySuit(campo[0].seme));
	strcat(str,"/");
	sprintf(buffer, "%d", campo[0].val);
	strcat(str,buffer);
	strcat(str,".png");
	image=gtk_image_new_from_file(str);
	button=gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(button),image);
	gtk_table_attach(GTK_TABLE(table),button,0,1,2,3,0,0,15,10);
	gtk_widget_show(button);

	gtk_widget_show(table);
}

void loop(GtkWidget*widget,gpointer data)
{
	if(stato)
		finestra();
	else
		endMsg();
}

void choice(GtkWidget* widget,gpointer data)
{
	int scelta;
	if(strcmp(data,"0")==0)
		scelta=1;
	else if(strcmp(data,"1")==0)
		scelta=2;
	else if(strcmp(data,"2")==0)
		scelta=3;
	togli_carta(scelta);
}

void finestra(void)
{
	gtk_widget_destroy(window);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title (GTK_WINDOW (window), "Briscola");
	//gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	//gtk_window_set_default_size(GTK_WINDOW(window),800,500);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	table=gtk_table_new(8,5,FALSE);
	gtk_container_add(GTK_CONTAINER(window),table);
	GtkWidget *label,*image,*button;

	PangoFontDescription *font_desc;
  gdk_color_parse ("white", &color);
  font_desc = pango_font_description_from_string("16");

	label=gtk_label_new("Briscola:");
	//gtk_misc_set_alignment(GTK_MISC(label),0,1);
	gtk_table_attach(GTK_TABLE(table),label,0,1,4,5,0,0,0,30);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_widget_show(label);

	label=gtk_label_new("Clicca sulla carta da giocare!");
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,4,5,6);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_widget_show(label);

	if(turno!=primo)
	{
		label=gtk_label_new("Carta in gioco:");
		gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,0,0,10,0);
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_widget_show(label);
	}

	char *str;
	str=(char*)malloc(sizeof(char)*100);
	strcpy(str,"Carte nel mazzo: ");
	char buffer[5];
	sprintf(buffer, "%d", n);
	strcat(str,buffer);

	label=gtk_label_new(str);
	gtk_table_attach(GTK_TABLE(table),label,4,5,2,3,0,0,10,0);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_widget_show(label);

	strcpy(str,"Turno di ");
	strcat(str,p[turno].nome);

	label=gtk_label_new(str);
	gtk_table_attach_defaults(GTK_TABLE(table),label,4,5,0,1);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_widget_show(label);

	if(n>35)
		image=gtk_image_new_from_file("../mazzo/normale/mazzo8.png");
	else if(n>30)
		image=gtk_image_new_from_file("../mazzo/normale/mazzo7.png");
	else if(n>25)
		image=gtk_image_new_from_file("../mazzo/normale/mazzo6.png");
	else if(n>20)
		image=gtk_image_new_from_file("../mazzo/normale/mazzo5.png");
	else if(n>15)
		image=gtk_image_new_from_file("../mazzo/normale/mazzo4.png");
	else if(n>10)
		image=gtk_image_new_from_file("../mazzo/normale/mazzo3.png");
	else if(n>5)
		image=gtk_image_new_from_file("../mazzo/normale/mazzo2.png");
	else if(n>0)
		image=gtk_image_new_from_file("../mazzo/normale/mazzo1.png");
	else
		image=gtk_image_new_from_file("../mazzo/normale/mazzo0.png");

	gtk_table_attach(GTK_TABLE(table),image,4,5,3,6,0,0,0,0);
	gtk_widget_show(image);

	strcpy(str,"../carte/");
	strcat(str,getStringBySuit(briscola.seme));
	strcat(str,"/");
	sprintf(buffer, "%d", briscola.val);
	strcat(str,buffer);
	strcat(str,".png");

	image=gtk_image_new_from_file(str);
	button=gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(button),image);
	gtk_table_attach(GTK_TABLE(table),button,0,1,5,7,0,0,20,0);
	//gtk_widget_set_sensitive(button,FALSE);	//lo commento perchè se non è abilitato è brutto da vedere
	gtk_widget_show(button);

	int i;
	for(i=0;i<p[inverti(turno)].inmano;i++)
	{
		image=gtk_image_new_from_file("../mazzo/dorso.png");
		gtk_table_attach_defaults(GTK_TABLE(table),image,i+1,i+2,0,3);
		gtk_widget_show(image);
	}

	if(turno!=primo)
	{
		strcpy(str,"../carte/");
		strcat(str,getStringBySuit(campo[0].seme));
		strcat(str,"/");
		sprintf(buffer, "%d", campo[0].val);
		strcat(str,buffer);
		strcat(str,".png");
		image=gtk_image_new_from_file(str);
		button=gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(button),image);
		//gtk_widget_set_sensitive(button,FALSE);
		gtk_table_attach(GTK_TABLE(table),button,0,1,1,4,0,0,0,0);
		gtk_widget_show(button);
	}

	for(i=0;i<p[turno].inmano;i++)
	{
		strcpy(str,"../carte/");
		strcat(str,getStringBySuit(p[turno].mano[i].seme));
		strcat(str,"/");
		sprintf(buffer, "%d", p[turno].mano[i].val);
		strcat(str,buffer);
		strcat(str,".png");
		image=gtk_image_new_from_file(str);
		button=gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(button),image);
		gtk_table_attach(GTK_TABLE(table),button,i+1,i+2,6,8,0,0,0,0);
		char * data;
		switch (i)
		{
			case 0:
			{
				data="0";
				break;
			}
			case 1:
			{
				data="1";
				break;
			}
			case 2:
			{
				data="2";
				break;
			}
		}
		g_signal_connect(button,"clicked",GTK_SIGNAL_FUNC(choice),data);
		gtk_widget_show(button);
	}

	gtk_widget_show(table);
}

void inizio(GtkWidget*widget,gpointer data)
{
	n=34;
	//dichiara il mazzo
	mazzo=(struct carta*)malloc(sizeof(struct carta)*40);
	p[0].punteggio=0;
	p[1].punteggio=0;
	//inizializza il mazzo
	initDeck();

	//mescola il mazzo
	shufDeck();

	//stampa il mazzo in ordine
	//printDeckByIndex(mazzo);

	//inizia una nuova partita
	/*
	printf("Inserisci il nome del giocatore 1: ");
	fgets(p[0].nome,20,stdin);
	strtok(p[0].nome,"\n");
	p[0].punteggio=0;
	p[0].vittorie=0;

	do
	{
		printf("Inserisci il nome del giocatore 2: ");
		fgets(p[1].nome,20,stdin);	printf("Vittorie:\r\t\t%d\r\t\t\t\t\t%d\n",p[0].vittorie,p[1].vittorie);

		strtok(p[1].nome,"\n");
		p[1].punteggio=0;
		p[1].vittorie=0;
		if(strcmp(p[0].nome,p[1].nome)==0)
			printf("Questo nome è già in uso dal giocatore 1.\n");

	}while(strcmp(p[0].nome,p[1].nome)==0);*/



	briscola=mazzo[39];	//sceglie come briscola l'ultima carta del mazzo

	//mette in mano ai due giocatori le 6 carte in cima al mazzo
	p[0].mano[0]=mazzo[0];
	p[0].mano[1]=mazzo[1];
	p[0].mano[2]=mazzo[2];
	p[1].mano[0]=mazzo[3];
	p[1].mano[1]=mazzo[4];
	p[1].mano[2]=mazzo[5];
	p[0].inmano=3;
	p[1].inmano=3;

	int i;
	for(i=0;i<n;i++)           //sposto tutte le carte indietro di 6 posizioni
			mazzo[i]=mazzo[i+6];

	mazzo=(struct carta*)realloc(mazzo,sizeof(struct carta)*n);    //realloco la memoria del mazzo più corto

	//pone a 0 il numero di carte vinte da ciascun giocatore
	scartate[0]=0;
	scartate[1]=0;


	//printf("%d",turno);
	primo=turno;
	system(CLEAR);
	loop(NULL,NULL);
	system(CLEAR);
	//vincitore(NULL,NULL);
}

void pesca(void)
{
	int i;
	if(n!=0)
	{
		p[winner].mano[2]=mazzo[0];
		p[inverti(winner)].mano[2]=mazzo[1];
		p[winner].inmano++;
		p[inverti(winner)].inmano++;
		//toglie dal mazzo le carte pescate
		n-=2;
		for(i=0;i<n;i++)           //sposto tutte le carte indietro di 2 posizioni
			mazzo[i]=mazzo[i+2];
		if(n==0)
			free(mazzo);
		else
			mazzo=(struct carta*)realloc(mazzo,sizeof(struct carta)*n);    //realloco la memoria del mazzo più corto
	}


	//se carte in mano=0 allora finisce la partita
	if(p[0].inmano==0)
	{
		stato=inverti(stato);
		loop(NULL,NULL);
	}
	else	//altrimenti stampa il messaggio di cambio giocatore.
	{
		//cambio_giocatore();
		fine_mano();
	}

	//passa il turno al vincitore
	primo=winner;
	turno=primo;
}

void fine_mano(void)
{
	gtk_widget_destroy(window);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Briscola");
	//gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	//gtk_window_set_default_size(GTK_WINDOW(window),800,500);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	PangoFontDescription *font_desc;
	gdk_color_parse ("white", &color);
	font_desc = pango_font_description_from_string("16");
	GtkWidget *label,*button,*image;
	table=gtk_table_new(6,3,FALSE);
	gtk_container_add(GTK_CONTAINER(window),table);

	label=gtk_label_new("Carta giocata");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);
	char *str;
	str=(char*)malloc(sizeof(char)*150);
	strcpy(str,"da ");
	strcat(str,p[inverti(turno)].nome);
	strcat(str,":");
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	label=gtk_label_new("Carta giocata");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,0,1);
	gtk_widget_show(label);
	strcpy(str,"da ");
	strcat(str,p[turno].nome);
	strcat(str,":");
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);
	gtk_widget_show(label);

	strcpy(str,"Vincitore della mano: ");
	strcat(str,p[winner].nome);
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,3,3,4);
	gtk_widget_show(label);

	label=gtk_label_new("Premi OK per iniziare il suo turno");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,3,4,5);
	gtk_widget_show(label);

	label=gtk_label_new("Briscola:");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,2,3,0,2);
	gtk_widget_show(label);

	button=gtk_button_new_with_label("OK");
	gtk_table_attach(GTK_TABLE(table),button,0,3,5,6,GTK_FILL,0,5,10);
	g_signal_connect(button,"clicked",GTK_SIGNAL_FUNC(loop),NULL);
	gtk_widget_show(button);

	char buffer[5];

	strcpy(str,"../carte/");
	strcat(str,getStringBySuit(briscola.seme));
	strcat(str,"/");
	sprintf(buffer, "%d", briscola.val);
	strcat(str,buffer);
	strcat(str,".png");
	image=gtk_image_new_from_file(str);

	button=gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(button),image);
	gtk_table_attach(GTK_TABLE(table),button,2,3,2,3,0,0,15,10);
	gtk_widget_show(button);

	strcpy(str,"../carte/");
	strcat(str,getStringBySuit(campo[0].seme));
	strcat(str,"/");
	sprintf(buffer, "%d", campo[0].val);
	strcat(str,buffer);
	strcat(str,".png");
	image=gtk_image_new_from_file(str);
	button=gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(button),image);
	gtk_table_attach(GTK_TABLE(table),button,0,1,2,3,0,0,15,10);
	gtk_widget_show(button);

	strcpy(str,"../carte/");
	strcat(str,getStringBySuit(campo[1].seme));
	strcat(str,"/");
	sprintf(buffer, "%d", campo[1].val);
	strcat(str,buffer);
	strcat(str,".png");
	image=gtk_image_new_from_file(str);
	button=gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(button),image);
	gtk_table_attach(GTK_TABLE(table),button,1,2,2,3,0,0,15,10);
	gtk_widget_show(button);

	gtk_widget_show(table);
}

void controlla_vincitore(void)
{
	//controlla il vincitore
	//se il segno è uguale
	if(campo[0].seme==campo[1].seme)
	{
		//vince la carta con win>
		if(campo[0].win>campo[1].win)
			winner=primo;
		else
			winner=inverti(primo);
	}
	//se il segno è diverso
	else
	{
		//se uno dei due segni è briscola vince quello
		if(campo[0].seme==briscola.seme)
			winner=primo;
		else if(campo[1].seme==briscola.seme)
			winner=inverti(primo);
		//se nessuno è briscola
		else	//vince la prima carta messa
			winner=primo;
	}
	//aggiunge alle carte vinte da winner le carte in campo
	scarti[winner][scartate[winner]++]=campo[0];
	scarti[winner][scartate[winner]++]=campo[1];
}



void togli_carta(int scelta)
{
	if(turno==primo)
		campo[0]=p[turno].mano[scelta-1];
	else
		campo[1]=p[turno].mano[scelta-1];

	//toglie dalla mano del giocatore la carta giocata
	p[turno].inmano--;
	if(scelta-1==0)
	{
		p[turno].mano[0]=p[turno].mano[1];
		p[turno].mano[1]=p[turno].mano[2];
		p[turno].mano[2]=vuota;
	}
	if(scelta-1==1)
	{
		p[turno].mano[1]=p[turno].mano[2];
		p[turno].mano[2]=vuota;
	}
	if(scelta-1==2)
	{
		p[turno].mano[2]=vuota;
	}
	if(turno!=primo)
	{
		controlla_vincitore();
		//fa pescare le carte ai 2 giocatori (se il mazzo non è vuoto)
		pesca();
		//printf("\n\nVincitore della mano: %d (%s)\n\n",winner,p[winner].nome);	//stampa il vincitore

	}
	else
	{
		intraMsg();
		turno=inverti(turno);
	}
}

void initDeck()
{
	//carica il mazzo con i valori di ogni carta
	int i,j;

	for(i=0;i<4;i++)
	{
		for(j=1;j<=10;j++)
		{
			mazzo[10*i+j-1].seme=i;
			mazzo[10*i+j-1].index=-1;
			mazzo[10*i+j-1].val=j;
			mazzo[10*i+j-1].eval=0;

			switch (j)
			{
				case 1:		//asso
					mazzo[10*i+j-1].eval=11;
					mazzo[10*i+j-1].win=9;
					break;
				case 2:
					mazzo[10*i+j-1].win=0;
					break;
				case 3:		//tre
					mazzo[10*i+j-1].eval=10;
					mazzo[10*i+j-1].win=8;
					break;
				case 4:
					mazzo[10*i+j-1].win=1;
					break;
				case 5:
					mazzo[10*i+j-1].win=2;
					break;
				case 6:
					mazzo[10*i+j-1].win=3;
					break;
				case 7:
					mazzo[10*i+j-1].win=4;
					break;
				case 8:		//fante
					mazzo[10*i+j-1].eval=2;
					mazzo[10*i+j-1].win=5;
					break;
				case 9:		//cavallo
					mazzo[10*i+j-1].eval=3;
					mazzo[10*i+j-1].win=6;
					break;
				case 10:	//re
					mazzo[10*i+j-1].eval=4;
					mazzo[10*i+j-1].win=7;
					break;
			}
			//printf("Carta: %d\tOrdine: %d\tSeme: %d\tNumero: %d\tValore punti: %d\n",10*i+j,mazzo[10*i+j-1].index,mazzo[10*i+j-1].seme,mazzo[10*i+j-1].val,mazzo[10*i+j-1].eval);
		}
	}

}

int inverti(int a)
{
	//restituisce 0 se a=1
	//restutuisce 1 se a=0
	if(a==0)
		return 1;
	return 0;
}

void shufDeck()
{
	struct carta temp[40];
	int i,j,ii,jj,random,ext,k;
	srand(time(NULL));
	//int random;

	//scorre seme per seme
	for(i=0;i<4;i++)
	{
		//scorre carta per carta
		for(j=1;j<=10;j++)
		{

						do{
										random=rand()%40; //estrae un numero casuale da 0 a 39
							ext=0;
									//scorre seme per seme II
									for(ii=0;ii<4;ii++)
									{
										//scorre carta per carta II
										for(jj=1;jj<=10;jj++)
										{
											if(random==mazzo[10*ii+jj-1].index)
												ext=1;

										}
									}
						}
						while(ext==1);

						mazzo[10*i+j-1].index=random;
			//printf("\nSeme: %d\tCarta: %d",mazzo[10*i+j-1].seme,mazzo[10*i+j-1].val);

		}
	}


	for(k=0;k<40;k++)
		for(i=0;i<4;i++)
			for(j=1;j<=10;j++)
				if(mazzo[10*i+j-1].index==k)
						temp[k]=mazzo[10*i+j-1];

	for(k=0;k<40;k++)
		mazzo[k]=temp[k];
}

void printDeckByIndex(struct carta mazzo[])
{
	int i,j,k;

	//stampa il mazzo ordinato
	printf("\n\nMazzo ordinato");
	for(k=0;k<40;k++)
		for(i=0;i<4;i++)
			for(j=1;j<=10;j++)
				if(mazzo[10*i+j-1].index==k)
						printf("\nSeme: %d\tCarta: %d\tIndice: %d",mazzo[10*i+j-1].seme,mazzo[10*i+j-1].val,mazzo[10*i+j-1].index);
}

char *getStringBySuit(int suit)
{
	//char string[20];

	switch (suit)
	{
		case 0:
			return "bastoni";

		case 1:
			return "coppe";

		case 2:
			return "denari";

		case 3:
			return "spade";
		default:
			printf("%d",suit);
	}
	return "-";
}

char *getStringByCard(int card)
{
	//char string[20];

	switch (card)
	{
		case 1:
			return "asso";

		case 2:
			return "2";

		case 3:
			return "3";

		case 4:
			return "4";

		case 5:
			return "5";

		case 6:
			return "6";

		case 7:
			return "7";

		case 8:
			return "fante";

		case 9:
			return "cavallo";

		case 10:
			return "re";

		default:
			printf("%d",card);
	}
	return "-";
}

int getSuitByIndex(struct carta mazzo[],int index)
{
	int i,j;
	for(i=0;i<4;i++)
		for(j=1;j<=10;j++)
			if(mazzo[10*i+j-1].index==index)
					return mazzo[10*i+j-1].seme;

	return -1;
}

int getCardByIndex(struct carta mazzo[],int index)
{
	int i,j;

		for(i=0;i<4;i++)
			for(j=1;j<=10;j++)
				if(mazzo[10*i+j-1].index==index)
						return mazzo[10*i+j-1].val;
	return -1;
}

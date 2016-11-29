#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



//#define PORT 3490
int PORT;
#define MAXSIZE 1024
#define SLEEP 0.2

//sequenze di escape per l'output colorato
char *rosso="\033[1;31m";
char *verde="\033[1;32m";
char *reset="\033[0m";
char *grassetto="\033[0m";


struct carta{
	int seme;	//seme della carta
	int val;	//numero della carta. Asso=1, 2=2, Tre=3, ecc..
	int eval;	//valore della carta. Asso=11, 2=0, Tre=10 ecc...
	int index;	//ordine della carta nel mazzo. Esempio: Asso di bastoni=16, 2 di coppe=9...
	int win;	//ordine delle carte in base alla più potente. Esempio: 1,2,4,5,6,7,fante,cavallo,re,3,asso
	char *strcarta;
}mano[3];

struct sockaddr_in server_info;
struct hostent *he;
int socket_fd,num;
char buffer[1024],txmsg[1024];
char n;	//numero di carte nel mazzo
int inmano=3;//TODO
struct carta briscola;
//char buff[1024];
int mioid,turno;
int stmano=0;	//stato della mano. 0=primo giocatore 1=secondo
int scelta;	//carta giocata dal giocatore corrente
int inattesa=0;	// 1 se il client sta aspettando per inviare il messaggio al server
struct carta giocata;
struct carta campo[2];
int vincitoremano=0,vincitoremanoold=0;
char nomeavversario[24];
int punt;

int argcc;
char ** argvv;

char myUsername[20];
char *hostn;
int statopartita=0; //0=richiesta di una nuova partita, 1=in attesa di un avversario, 2=in gioco
int autoplay=1; //gioca automaticamente la prima carta in mano. Utile durante il testing del programma

GtkWidget * window, * table, * window2;

char *getStringBySuit(int);
char *getStringByCard(int);
void intraMsg(int);
void finestra(int);
void fine(int);
void delete_event( GtkWidget *widget, gpointer data );
void startup_msg();
int inverti(int);
void choice(GtkWidget * window, gpointer data);
void invalid_host();

int main(int argc, char *argv[])
{
		inattesa=0;
		statopartita=0;
		stmano=0;
		gtk_init(&argc, &argv);
		argcc=argc;
		argvv=(char**)malloc(sizeof(char*)*argc);
		int i,u=0,h=0;
		for(i=0;i<argc;i++)
		{
			argvv[i]=(char*)malloc(sizeof(char)*strlen(argv[i]));
			strcpy(argvv[i],argv[i]);
		}
		PORT=3490;
		for(i=1;i<argc;i++)
		{
			if(argv[i][0]=='-')
			{
				switch(argv[i][1])
				{
					case 'h':
					{
						printf("Hostname:  %s\n",argv[i+1]);
						hostn=argv[i+1];
						h++;
						break;
					}
					case 'u':
					{
						printf("Username: %s\n",argv[i+1]);
						strcpy(myUsername,argv[i+1]);
						u++;
						break;
					}
					case 'p':
					{
						printf("Porta: %s\n",argv[i+1]);
						PORT=atoi(argv[i+1]);
						printf("Porta: %d\n",PORT);
						break;
					}
					default:
					{
						printf("%s\"%s\" - Opzione non valida%s\n",rosso,argv[i],reset);
						break;
					}
				}
			}
		}

		if(u==0)
		{
			printf("%sERRORE FATALE!!!\nUsername non fornito%s\nUso: %s [-h hostname] [-p porta] -u nome_utente\n",rosso,reset,argv[0]);
			return -1;
		}

		if(h==0)
		{
			printf("%sHostname non fornito, verrà usato localhost:%d%s\n",rosso,PORT,reset);
			hostn="localhost";
		}

	//chiede di inserire l'username
	/*printf("Inserisci il tuo nome utente: ");
	fgets(myUsername,20,stdin);
	strtok(myUsername,"\n");*/
	/* //prima connessione che crasha il programma
    if((he = gethostbyname(hostn))==NULL)
    {
        fprintf(stderr, "Cannot get host name\n");
        exit(1);
    }

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        fprintf(stderr, "Socket Failure!!\n");
        exit(1);
    }

    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(PORT);
    server_info.sin_addr = *((struct in_addr *)he->h_addr);

    if (connect(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr))<0) {
        //fprintf(stderr, "Connection Failure\n");

    //buffer = "Hello World!! Lets have fun\n";
    //memset(buffer, 0 , sizeof(buffer));
    //while(1)
    //{
		//printf("Inserisci il messaggio da inviare: ");
		//fgets(buffer,MAXSIZE-1,stdin);
		//strtok(buffer,"\n");//elimina il \n in fondo al messaggio

		printf("Chiedo di iniziare una nuova partita...");
		strcpy(buffer,"NewGame");
		strcat(buffer,":");
		strcat(buffer,myUsername);

		if(send(socket_fd,buffer, strlen(buffer),0)== -1)
		{
			fprintf(stderr, "Failure Sending Message\n");
			close(socket_fd);
			exit(1);
		}
		else
		{
			//printf("Client:Message being sent: %s\n",buffer);
			num=recv(socket_fd, buffer, sizeof(buffer),0); //riceve la risposta dal server
			if(num<=0)
				printf("Either Connection Closed or Error\n");
				//break;	//Esce dal ciclo While
			else
				//buff[num] = '\0';
				printf("Messaggio ricevuto: %s\n",buffer);

        }
    //}


}



    close(socket_fd);
	*/
	printf("Briscola!\n");
	startup_msg();
	gtk_main();
	return 0;
}//Fine di main

void inizio(GtkWidget * widget, gpointer data)
{
	main(argcc,argvv);
}

void loop(GtkWidget * widget, gpointer data)
{
	int famosa;
	while(statopartita!=-1)
	{
		famosa=0;
		if((he = gethostbyname(hostn))==NULL)
		{
			fprintf(stderr, "Hostname non valido\n");
			famosa=6;
		}
		else
		{
			if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1)
			{
				fprintf(stderr, "Socket Failure!!\n");
				exit(1);
			}

			memset(&server_info, 0, sizeof(server_info));
			server_info.sin_family = AF_INET;
			server_info.sin_port = htons(PORT);
			server_info.sin_addr = *((struct in_addr *)he->h_addr);



			if (connect(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr))<0) {
				//fprintf(stderr, "Connection Failure\n");
				//perror("connect");

					//printf("Aspetto 5 secondi e riprovo...\n");
							close(socket_fd);
							sleep(SLEEP);

				//exit(1);
			}

			//buffer = "Hello World!!\n";
			//memset(buffer, 0 , sizeof(buffer));
			//while(1)
			//{
				//printf("Inserisci il messaggio da inviare: ");
				//fgets(buffer,MAXSIZE-1,stdin);
				//strtok(buffer,"\n");//elimina il \n in fondo al messaggio

				if(statopartita==0)
				{
					printf("Chiedo di iniziare una nuova partita...");
					strcpy(buffer,"NewGame");
					strcat(buffer,":");
					strcat(buffer,myUsername);
				}

				if(statopartita==1)
				{
					printf("Chiedo di avviare il gioco...");
					strcpy(buffer,"Start");
					strcat(buffer,":");
					strcat(buffer,myUsername);
				}

				if(statopartita==2)
				{
					//printf("Chiedo di avviare il gioco...");
					strcpy(buffer,"Update");
				}

				if(statopartita==2&&inattesa==1)
				{
					//printf("Chiedo di avviare il gioco...");
					inattesa=0;
					strcpy(buffer,txmsg);
				}






				if(send(socket_fd,buffer, strlen(buffer),0)== -1)
				{
					//fprintf(stderr, "Failure Sending Message\n");

					//printf("Aspetto 5 secondi e riprovo...\n");
							close(socket_fd);
							sleep(SLEEP);

					//exit(1);
				}
				else
				{
					//printf("Client:Message being sent: %s\n",buffer);
					num=recv(socket_fd, buffer, sizeof(buffer),0); //riceve la risposta dal server
					if(num<=0)
					{
						//printf("Either Connection Closed or Error\n");
						//break;	//Esce dal ciclo While
							//printf("Aspetto 5 secondi e riprovo...\n");
							close(socket_fd);
							sleep(SLEEP);
					}
					else
					{
						//buff[num] = '\0';
						//printf("Messaggio ricevuto: %s\n",buffer);


						int i;
						const char sep[2] = ":";//usato per la divisione del messaggio
						char *token;
						char mess[7][1024];

						token = strtok(buffer, sep);//ottiene il primo elemento
						for(i=0;token != NULL ;i++) //cicla gli altri elementi
						{
						  //printf( " %s\n", token );
							strcpy(mess[i],token);
						  token = strtok(NULL, sep);
						}
						//mess[1] dovrebbe contenere le carte


						if(strcmp(buffer,"okNewGame")==0)
						{
							printf("Connesso! Aspetto un avversario...\n");
							close(socket_fd);
							statopartita=1;
							sleep(SLEEP);
						}

						if(strcmp(mess[0],"NewPort")==0)
						{
							close(socket_fd);
							PORT=atoi(mess[1]);
							printf("Mi sposto sulla nuova porta (%d)\n",PORT);
							sleep(SLEEP);
						}

						if(strcmp(buffer,"wait")==0)
						{
							//printf("Aspetto 5 secondi e riprovo...\n");
							close(socket_fd);
							sleep(SLEEP);
						}

						if(strcmp(buffer,"waitopn")==0)
						{
							printf("In attesa di un avversario...\n");
							close(socket_fd);
							sleep(SLEEP);
						}

						if(strcmp(buffer,"close")==0)
						{
							//printf("Server occupato. Aspetto 5 secondi...\n");
							close(socket_fd);
							sleep(SLEEP);
						}

						if(strcmp(buffer,"error")==0)
						{
							printf("Errore. Connessione chiusa.\n");
							statopartita=-1;
						}

						//TODO controlla se il messaggio inizia con "cards"
						//in quel caso copia le carte in mano e inizia la partita

						if(strcmp(mess[0],"Cards")==0)
						{
							//esempio	Cards:12-34-0x:12:X:010:21-24
							//			Cards:carta1-carta2-carta3:briscola:cartenelmazzo:idturnostatovincitore:campo0-campo1
							//printf("Carte ricevute.\n");
							statopartita=2;//avvia la partita vera e propria


							mano[0].seme=(int)(mess[1][0]-'0');
							mano[0].val=(int)(mess[1][1]-'0');
							mano[1].seme=(int)(mess[1][3]-'0');
							mano[1].val=(int)(mess[1][4]-'0');
							mano[2].seme=(int)(mess[1][6]-'0');
							mano[2].val=(int)(mess[1][7]-'0');

							//trasforma eventuali 'x' contenute nel messaggio in 10
							if(mano[0].val=='x'-'0')
								mano[0].val=10;
							if(mano[1].val=='x'-'0')
								mano[1].val=10;
							if(mano[2].val=='x'-'0')
								mano[2].val=10;

							//mess[1] dovrebbe contenere le carte,mess[2] la briscola,mess[3] il numero di carte

							briscola.seme=(int)(mess[2][0]-'0');
							briscola.val=(int)(mess[2][1]-'0');
							if(briscola.val=='x'-'0')
								briscola.val=10;

							n=(int)(mess[3][0])-'<';//riceve il numero di carte nel mazzo

							//riceve l'id del giocatore, il turno e lo stato della mano
							mioid=(int)(mess[4][0]-'0');//id
							turno=(int)(mess[4][1]-'0');//turno
							stmano=(int)(mess[4][2]-'0');//stato
							if(n==34)
								vincitoremanoold=turno;
							else
								vincitoremanoold=vincitoremano;
							vincitoremano=(int)(mess[4][3]-'0');//vincitore


							//riceve le carte in campo
							campo[0].seme=(int)(mess[5][0]-'0');
							campo[0].val=(int)(mess[5][1]-'0');
							campo[1].seme=(int)(mess[5][3]-'0');
							campo[1].val=(int)(mess[5][4]-'0');

							if(campo[0].val=='H')
								campo[0].val=10;
							if(campo[1].val=='H')
								campo[1].val=10;

							/*
							printf("Carte in mano: %d-%d %d-%d %d-%d\n",mano[0].seme,mano[0].val,mano[1].seme,mano[1].val,mano[2].seme,mano[2].val);
							printf("Briscola: %d-%d (%s di %s)\n",briscola.seme,briscola.val,getStringByCard(briscola.val),getStringBySuit(briscola.seme));
							printf("Numero di carte nel mazzo: %d\n",n);
							printf("Tuo id: %d\n",mioid);
							printf("Turno di: %d\n",turno);
							printf("Stato della mano: %d\n",stmano);
							printf("Vincitore della mano: %d\n",vincitoremano);
							* */
						}



						if(strcmp(mess[0],"Winner")==0)
						{
							famosa=3;
							printf("Hai vinto!\n");
							close(socket_fd);
							punt=120-(buffer[7]-'!');
							//printf("BUFFER[7]=%d",buffer[7]);
							printf("Il tuo punteggio: %d\n",punt);
							printf("Punteggio dell'avversario: %d\n",120-punt);
							statopartita=-1;
						}


						if(strcmp(mess[0],"Loser")==0)
						{
							famosa=4;
							printf("Hai perso!\n");
							close(socket_fd);
							punt=buffer[6]-'!';
							//printf("BUFFER[6]=%d",buffer[6]);
							printf("Il tuo punteggio: %d\n",punt);
							printf("Punteggio dell'avversario: %d\n",120-punt);
							statopartita=-1;
						}


						if(strcmp(mess[0],"Draw")==0)
						{
							famosa=5;
							printf("Pareggio!\n");
							close(socket_fd);
							statopartita=-1;
						}


						if(strcmp(mess[0],"0trn")==0)//primo turno del giocatore corrente: gli fa scegliere quale carta inviare
						{
							if(n==34)
								vincitoremanoold=mioid;
							famosa=1;
							printf("È il tuo turno.\n");
							close(socket_fd);

							strcpy(nomeavversario,mess[2]);

							printf("Nome avversario: %s\n",nomeavversario);
							printf("***È il tuo turno***\n");
							printf("PRIMA MANO\n");
							printf("Carte nel mazzo: %d/40\n\n",n);
							printf("Briscola: %s di %s.\n\n\n",getStringByCard(briscola.val),getStringBySuit(briscola.seme));

							//stampa le carte in mano al giocatore corrente
							printf("\nLa tua mano:\n");
							for(i=0;i<inmano;i++)
								if(mano[i].val!=15)
									printf("%d) %s di %s\n",i+1,getStringByCard(mano[i].val),getStringBySuit(mano[i].seme));

							inattesa=1;
						}

						if(strcmp(mess[0],"1trn")==0)//secondo turno del giocatore corrente: gli fa scegliere quale carta inviare, mostrando anche la carta giocata dall'avversario.
						{
							famosa=2;
							giocata.seme=(int)mess[1][0]-'0';
							giocata.val=(int)mess[1][1]-'0';
							if(giocata.val=='x')
								giocata.val=10;

							strcpy(nomeavversario,mess[2]);
							printf("Nome avversario: %s\n",nomeavversario);

							printf("SECONDA MANO\n");
							printf("Carte nel mazzo: %d/40\n\n",n);
							printf("Briscola: %s di %s.\n\n\n",getStringByCard(briscola.val),getStringBySuit(briscola.seme));

							printf("Carta giocata dal tuo avversario: %s di %s\n",getStringByCard(giocata.val),getStringBySuit(giocata.seme));
							close(socket_fd);


							//stampa le carte in mano al giocatore corrente
							printf("\nLa tua mano:\n");

							for(i=0;i<inmano;i++)
								if(mano[i].val!=15)
									printf("%d) %s di %s\n",i+1,getStringByCard(mano[i].val),getStringBySuit(mano[i].seme));

							if(autoplay)
							{
								scelta=1;
								printf("\nAUTOPLAY: Gioco '1'\n");
								sleep(SLEEP);
							}
							else
							{
								printf("\nChe carta vuoi giocare? ");
								scanf("%d",&scelta);
							}


							inattesa=1;
							char carattere[2];
							carattere[0]=scelta-1+'0';
							carattere[1]='\0';
							strcpy(txmsg,carattere);
						}
						close(socket_fd);
					}

			}
		}
		if(famosa!=0)
		break;
	}
	switch(famosa)	//in base al valore della variabile, creerà la finestra
	{
		case 1:	//primo turno
		{
			printf("\n\n\nPrimo turno\n");
			finestra(1);
			if(n!=34)
				intraMsg(vincitoremanoold);
			break;
		}
		case 2:	//secondo turno
		{
			printf("\n\n\nSecondo turno\n");
			finestra(2);
			if(n!=34)
				intraMsg(vincitoremanoold);
			break;
		}
		case 3:	//fine partita - vincita
		{
			printf("\n\n\nHai vinto\n");
			fine(1);
			break;
		}
		case 4:	//fine partita - perdita
		{
			printf("\n\n\nHai perso\n");
			fine(2);
			break;
		}
		case 5:	//fine partita - pareggio
		{
			printf("\n\n\nPareggio\n");
			fine(3);
			break;
		}
		case 6:
		{
			printf("\n\nHostname non valiso!!!\n");
			invalid_host();
			break;
		}
	}
}

void fine(int m)
{
	int i,j;
	gtk_widget_destroy(window);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "NetBriscola - Fine Partita");
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
	table=gtk_table_new(4,8,TRUE);
	gtk_container_add(GTK_CONTAINER(window),table2);
	gtk_table_attach(GTK_TABLE(table2),table,0,2,1,2,GTK_FILL,GTK_FILL,0,15);
	gtk_widget_show(table2);


	label=gtk_label_new("PARTITA TERMINATA!");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach(GTK_TABLE(table2),label,0,2,0,1,GTK_FILL,GTK_FILL,0,0);
	gtk_widget_show(label);

	char *str;

	if(m==1){
		str=(char*)malloc(sizeof(char)*(strlen(myUsername)));
		strcpy(str,myUsername);
	}
	else{
		str=(char*)malloc(sizeof(char)*(strlen(nomeavversario)));
		strcpy(str,nomeavversario);
	}

	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,3,5,0,1);
	gtk_widget_show(label);


	sprintf(str,"%d",120-punt);
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,3,5,2,3);
	gtk_widget_show(label);

	if(m!=1){
		str=(char*)malloc(sizeof(char)*(strlen(nomeavversario)));
		strcpy(str,nomeavversario);
	}
	else{
		str=(char*)malloc(sizeof(char)*(strlen(myUsername)));
		strcpy(str,myUsername);
	}
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,6,8,0,1);
	gtk_widget_show(label);



	sprintf(str,"%d",punt);
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,6,8,2,3);
	gtk_widget_show(label);


	label=gtk_label_new("Punti:");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,2,2,3);
	gtk_widget_show(label);



	if(m==1)
		strcpy(str,"Hai Vinto!");
	else if(m==2)
		strcpy(str,"Hai Perso!");
	else
		strcpy(str,"Pareggio!, Wow!");
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach(GTK_TABLE(table2),label,0,2,2,3,GTK_FILL,GTK_FILL,0,0);
	gtk_widget_show(label);

	j=2;
	for(i=0;i<2;i++)
	{
		separator=gtk_vseparator_new();
		gtk_table_attach(GTK_TABLE(table),separator,j,j+1,0,4,GTK_FILL,GTK_FILL,0,0);
		gtk_widget_show(separator);
		j+=3;
	}

	j=1;
	separator=gtk_hseparator_new();
	gtk_table_attach_defaults(GTK_TABLE(table),separator,0,8,j,j+1);
	gtk_widget_show(separator);

	button=gtk_button_new_with_label("ESCI");
	gtk_table_attach(GTK_TABLE(table2),button,0,1,3,4,GTK_FILL,GTK_FILL,5,15);
	g_signal_connect(GTK_WIDGET(button),"clicked",GTK_SIGNAL_FUNC(delete_event),NULL);
	gtk_widget_show(button);

	button=gtk_button_new_with_label("RIGIOCA");
	gtk_table_attach(GTK_TABLE(table2),button,1,2,3,4,GTK_FILL,GTK_FILL,5,15);
	g_signal_connect(GTK_WIDGET(button),"clicked",GTK_SIGNAL_FUNC(inizio),NULL);
	gtk_widget_show(button);

	gtk_widget_show(table);
}

void intra(GtkWidget * widget, gpointer data)
{
	gtk_widget_set_sensitive(window,TRUE);
	gtk_widget_destroy(window2);
}

void intraMsg(int m)
{
	gtk_widget_set_sensitive(window,FALSE);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	window2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window2), "NetBriscola");
	//gtk_window_set_resizable (GTK_WINDOW(window2), FALSE);
	g_signal_connect (window2, "delete-event", G_CALLBACK (intra), NULL);
	gtk_window_set_position(GTK_WINDOW(window2),GTK_WIN_POS_CENTER);
	//gtk_window_set_default_size(GTK_WINDOW(window2),800,500);
	gtk_container_set_border_width (GTK_CONTAINER (window2), 5);
	gtk_widget_modify_bg(window2, GTK_STATE_NORMAL, &color);
	gtk_window_set_position(GTK_WINDOW(window2),GTK_WIN_POS_CENTER);
	PangoFontDescription *font_desc;
	gdk_color_parse ("white", &color);
	font_desc = pango_font_description_from_string("16");
	GtkWidget *label,*button,*image,*table;
	table=gtk_table_new(6,3,FALSE);
	gtk_container_add(GTK_CONTAINER(window2),table);

	if(vincitoremanoold==mioid)
	{
		label=gtk_label_new("Carta giocata");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
		gtk_widget_show(label);
		label=gtk_label_new("da te");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
		gtk_widget_show(label);

		label=gtk_label_new("Carta giocata");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,0,1);
		gtk_widget_show(label);
		label=gtk_label_new("dall'avversario");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);
		gtk_widget_show(label);
	}
	else
	{
		label=gtk_label_new("Carta giocata");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
		gtk_widget_show(label);
		label=gtk_label_new("dall'avversario");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
		gtk_widget_show(label);

		label=gtk_label_new("Carta giocata");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,0,1);
		gtk_widget_show(label);
		label=gtk_label_new("da te");
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);
		gtk_widget_show(label);
	}



	char str[200];

	strcpy(str,"Vincitore della mano: ");
	if(mioid==vincitoremano)
	  strcat(str,"TU");
	else
	  strcat(str,nomeavversario);
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,3,3,4);
	gtk_widget_show(label);

	label=gtk_label_new("Premi OK per iniziare il turno");
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
	g_signal_connect(button,"clicked",GTK_SIGNAL_FUNC(intra),NULL);
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
	gtk_widget_show(window2);
}

int inverti(int n)
{
	if(n)
		return 1;
	return 0;
}

void finestra(int t)
{
	gtk_widget_destroy(table);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
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

	char *str;
	str=(char*)malloc(sizeof(char)*100);

	strcpy(str,"Nome dell'avversario:  ");
	strcat(str,nomeavversario);

	label=gtk_label_new(str);
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,4,3,4);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_widget_show(label);


	label=gtk_label_new("Clicca sulla carta da giocare!");
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,4,5,6);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_widget_show(label);

	if(t!=1)
	{
		label=gtk_label_new("Carta in gioco:");
		gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,0,0,10,0);
		gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
		gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
		gtk_widget_show(label);
	}

	strcpy(str,"Carte nel mazzo: ");
	char buffer[5];
	sprintf(buffer, "%d", n);
	strcat(str,buffer);

	label=gtk_label_new(str);
	gtk_table_attach(GTK_TABLE(table),label,4,5,2,3,0,0,10,0);
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

	if(t!=1)
	{
		strcpy(str,"../carte/");
		strcat(str,getStringBySuit(giocata.seme));
		strcat(str,"/");
		sprintf(buffer, "%d", giocata.val);
		strcat(str,buffer);
		strcat(str,".png");
		image=gtk_image_new_from_file(str);
		button=gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(button),image);
		//gtk_widget_set_sensitive(button,FALSE);
		gtk_table_attach(GTK_TABLE(table),button,0,1,1,4,0,0,0,0);
		gtk_widget_show(button);
	}

	int tmp=0,i;
	for(i=0;i<3;i++)
	{
		if(mano[i].val!=15)
		{
			tmp++;
			strcpy(str,"../carte/");
			strcat(str,getStringBySuit(mano[i].seme));
			strcat(str,"/");
			sprintf(buffer, "%d", mano[i].val);
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
	}
	if(t==2)
		tmp--;
	for(i=0;i<tmp;i++)
	{
		image=gtk_image_new_from_file("../mazzo/dorso.png");
		gtk_table_attach_defaults(GTK_TABLE(table),image,i+1,i+2,0,3);
		gtk_widget_show(image);
	}

	gtk_widget_show(table);
}

void choice(GtkWidget * window, gpointer data)
{
	char str[50];
	if(vincitoremano==mioid||n==34)
	{
		#ifdef _WIN32
			strcpy(str,"start popup.exe");
		#else
			strcpy(str,"./popup &");
		#endif
		system(str);
	}

	strcpy(txmsg,data);
	loop(NULL,NULL);
}

void delete_event( GtkWidget *widget, gpointer data )
{
	g_print("Uscita...\n");
  gtk_main_quit();
	exit(0);
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

void startup_msg()
{
	gtk_widget_destroy(window);
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	GtkWidget * image;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "NetBriscola");
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
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
	table=gtk_table_new(3,1,FALSE);
  image=gtk_image_new_from_file("../testata/testata (grande).png"); //"testata/testata.png" per l'immagine più piccola
	gtk_table_attach_defaults(GTK_TABLE(table),image,0,1,0,1);
	gtk_widget_show(image);
	gtk_container_add(GTK_CONTAINER(window),table);
	char str[200];
	strcpy(str,"Premi \"Connetti\" per connetterti al server: ");
	strcat(str,hostn);
	label=gtk_label_new(str);
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);
	label=gtk_label_new("Se non succede nulla, è perché sta aspettando un avversario");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);
	gtk_widget_show(label);
	button=gtk_button_new_with_label("Connetti");
	gtk_table_attach_defaults(GTK_TABLE(table),button,0,1,3,4);
	g_signal_connect(button,"clicked",GTK_SIGNAL_FUNC(loop),NULL);
	gtk_widget_show(button);
	gtk_widget_show(table);
}

void invalid_host()
{
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	gtk_widget_destroy(window);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "NetBriscola");
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
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
	table=gtk_table_new(2,1,FALSE);
	gtk_container_add(GTK_CONTAINER(window),table);
	label=gtk_label_new("Hostname non valido");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);
	gtk_widget_show(label);
	button=gtk_button_new_with_label("Esci");
	gtk_table_attach_defaults(GTK_TABLE(table),button,0,1,3,4);
	g_signal_connect(button,"clicked",GTK_SIGNAL_FUNC(delete_event),NULL);
	gtk_widget_show(button);
	gtk_widget_show(table);
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
		case 72:
			return "re";

		default:
			printf("%d",card);
	}
	return "-";
}

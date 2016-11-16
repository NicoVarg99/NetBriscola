/*
 * Questo server si occupa di gestire una singola partita.
 * Alla fine del gioco viene spento e la porta liberata.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#ifdef _WIN32
	#define CLEAR "cls"

#elif __APPLE__
	#define CLEAR "clear"

#elif __linux__
	#define CLEAR "clear"

#endif


#define BACKLOG 10

//sequenze di escape per l'output colorato
char *rosso="\033[1;31m";
char *verde="\033[1;32m";
char *reset="\033[0m";
char *grassetto="\033[1m";


struct carta{
	int seme;	//seme della carta
	int val;	//numero della carta. Asso=1, 2=2, Tre=3, ecc..
	int eval;	//valore della carta. Asso=11, 2=0, Tre=10 ecc...
	int index;	//ordine della carta nel mazzo. Esempio: Asso di bastoni=16, 2 di coppe=9...
	int win;	//ordine delle carte in base alla più potente. Esempio: 1,2,4,5,6,7,fante,cavallo,re,3,asso
};

struct giocatore{
	int punteggio;	//punti totalizzati nella partita attuale
	int vittorie;	//partite vinte in totale
	char nome[20];
	struct carta mano[3];
	int inmano;		//numero di carte che il giocatore ha in mano
	char hostname[20];
};

struct serv{
	int gmstato;
		//stato della partita. 0=da iniziare, 1=in corso.
}srv;

struct giocatore p[2];	//crea due giocatori
int txbytes=0;
int rxbytes=0;

void initDeck(struct carta *);
void shufDeck(struct carta *);
void printDeckByIndex(struct carta *);
char *getStringBySuit(int);
char *getStringByCard(int);
int getSuitByIndex(struct carta *,int);
int getCardByIndex(struct carta *,int);
int inverti(int);
char buffer[1024];
int PORT=7500;

void trasmetti(int clientid,char messaggio[])
{
	//clientid: 0 o 1 a seconda del giocatore a cui inviare il messaggio
	//messaggio[]: messaggio da inviare al client
	
	char txmsg[1024];
	char buffer[1024];
	int socket_fd, client_fd,num;
	socklen_t size;
	int yes=1;    
    struct sockaddr_in server;
    struct sockaddr_in dest;
	int inattesa=1;	//variabile che segna se il programma è in attesa del client
	strcpy(txmsg,messaggio);//imposta come messaggio da trasmettere quello passato come argomento
	
	//printf("******inizio trasmetti********\n");
	//printf("Messaggio da inviare: %s\n",txmsg);
	
	
	
    if ((socket_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1)
    {
        perror("setsockopt");
        exit(1);
    }
    
    memset(&server, 0, sizeof(server));
    memset(&dest,0,sizeof(dest));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY; 
    
    if ((bind(socket_fd,(struct sockaddr *)&server,sizeof(struct sockaddr)))==-1)
    {//sizeof(struct sockaddr) 
        printf("%sErrore sul binding%s\nPorta in uso da un altro programma?",rosso,reset);
        exit(1);
    }

    if ((listen(socket_fd, BACKLOG))== -1)
    {
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }
    
    //printf("%sServer in ascolto sulla porta %d%s\n",verde,PORT,reset);
    //printf("Aspetto una connessione da %s...\n",p[clientid].hostname);
    
    while(inattesa) //ciclo che viene ripetuto mentre il server aspetta una connessione dal client
	{
			size=sizeof(struct sockaddr_in);
			
			if((client_fd=accept(socket_fd, (struct sockaddr *)&dest, &size))==-1)
			{
				perror("accept");
				exit(1);
			}
			
			//printf("Stabilita connessione con %s...",inet_ntoa(dest.sin_addr));
			if(strcmp(inet_ntoa(dest.sin_addr),p[clientid].hostname)==0)			
			{//controlla se il client è il destinatario del messaggio
				//printf("La accetto\n");
				strcpy(txmsg,messaggio);//imposta come messaggio da trasmettere quello passato come argomento
				while(1)
				{
					if((num=recv(client_fd,buffer,1024,0))==-1)
					{
							perror("recv");
							exit(1);
					}
					else if(num==0)
					{
						//printf("Connessione chiusa.\n\n");
						//printf("Connessione chiusa. Stat RX:%db TX:%db\r\t\t\t\t\t\t\t\t|\n",rxbytes,txbytes);
						break;//esce dal while interno, per aspettare un nuovo client
					}
					
					buffer[num]='\0';
					//strcpy(txmsg,"error");

					//printf("Ricevuto: %s\n",buffer);
					rxbytes+=strlen(buffer);//aggiorna le statistiche sull'uso della rete
					//printf("| Dimensione: %ld bytes.\r\t\t\t\t\t\t\t\t|\n",strlen(buffer));
					
					
					//controlla se viene ricevuto NewGame: inizia una nuova partita
					//if(strcmp(buffer,"NewGame")==0&&srv.gmstato==0)
				
					if(send(client_fd,txmsg,strlen(txmsg)+1,0)==-1) //invia la risposta
					{
						 fprintf(stderr,"Failure Sending Message\n");
						 close(client_fd);
						 break;
					}

					//printf("Invio...: %s\n",txmsg);
					txbytes+=strlen(txmsg);//aggiorna le statistiche dell'uso della rete.
					//printf("+–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––+\n");
					
				} //End of Inner While...
				inattesa=0;
			}
			else
			{//se il client non è il destinatario gli chiede di chiudere la connessione
				//printf("La rifiuto\n");
					if((num=recv(client_fd,buffer,1024,0))==-1)
					{
							perror("recv");
							exit(1);
					}
					else if(num==0)
					{
						//printf("Connessione chiusa.\n\n");
						break;//esce dal while interno, per aspettare un nuovo client
					}
					
					buffer[num]='\0';
					
					//printf("Ricevuto: %s\n",buffer);
					rxbytes+=strlen(buffer);//aggiorna le statistiche sull'uso della rete
					strcpy(txmsg,"close");
					if(send(client_fd,txmsg,strlen(txmsg)+1,0)==-1) //invia una risposta
					{
						 fprintf(stderr,"Failure Sending Message\n");
						 close(client_fd);
						 break;
					}

					//printf("Invio...: %s\n",txmsg);
					txbytes+=strlen(txmsg);//aggiorna le statistiche sull'uso della rete.
			}
		
			close(client_fd);//Close Connection Socket
	
	}//Fine While esterno

    close(socket_fd);
	//printf("************Fine trasmetti**************\n\n");
}//fine trasmetti




char *ricevi(int clientid)
{
	//clientid: 0 o 1 a seconda del giocatore a cui inviare il messaggio
	//messaggio[]: messaggio da inviare al client
	
	char txmsg[1024];
	
	int socket_fd, client_fd,num;
	socklen_t size;
	int yes=1;    
    struct sockaddr_in server;
    struct sockaddr_in dest;
	int inattesa=1;	//variabile che segna se il programma è in attesa del client
	strcpy(txmsg,"rx");//imposta come messaggio da trasmettere rx
	
	//printf("******inizio ricevi********\n");
	//printf("Messaggio da inviare: %s\n",txmsg);
	
	
    if ((socket_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1)
    {
        perror("setsockopt");
        exit(1);
    }
    
    memset(&server, 0, sizeof(server));
    memset(&dest,0,sizeof(dest));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY; 
    
    if ((bind(socket_fd,(struct sockaddr *)&server,sizeof(struct sockaddr)))==-1)
    {//sizeof(struct sockaddr) 
        printf("%sErrore sul binding%s\nPorta in uso da un altro programma?",rosso,reset);
        exit(1);
    }

    if ((listen(socket_fd, BACKLOG))== -1)
    {
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }
    
    //printf("%sServer in ascolto sulla porta %d%s\n",verde,PORT,reset);
    //printf("Aspetto una connessione da %s...\n",p[clientid].hostname);
    
    while(inattesa) //ciclo che viene ripetuto mentre il server aspetta una connessione dal client
	{
			size=sizeof(struct sockaddr_in);
			
			if((client_fd=accept(socket_fd, (struct sockaddr *)&dest, &size))==-1)
			{
				perror("accept");
				exit(1);
			}
			
			//printf("Stabilita connessione con %s...",inet_ntoa(dest.sin_addr));
			if(strcmp(inet_ntoa(dest.sin_addr),p[clientid].hostname)==0)			
			{//controlla se il client è il mittente del messaggio
				//printf("La accetto\n");
				strcpy(txmsg,"rx");//imposta come messaggio da trasmettere quello passato come argomento
				while(1)
				{
					if((num=recv(client_fd,buffer,1024,0))==-1)
					{
							perror("recv");
							exit(1);
					}
					else if(num==0)
					{
						//printf("Connessione chiusa.\n\n");
						//printf("Connessione chiusa. Stat RX:%db TX:%db\r\t\t\t\t\t\t\t\t|\n",rxbytes,txbytes);
						break;//esce dal while interno, per aspettare un nuovo client
					}
					
					buffer[num]='\0';
					//strcpy(txmsg,"error");

					//printf("Ricevuto: %s\n",buffer);
					rxbytes+=strlen(buffer);//aggiorna le statistiche sull'uso della rete
					//printf("| Dimensione: %ld bytes.\r\t\t\t\t\t\t\t\t|\n",strlen(buffer));
					
					
					//controlla se viene ricevuto NewGame: inizia una nuova partita
					//if(strcmp(buffer,"NewGame")==0&&srv.gmstato==0)
				
					if(send(client_fd,txmsg,strlen(txmsg)+1,0)==-1) //invia la risposta
					{
						 fprintf(stderr,"Failure Sending Message\n");
						 close(client_fd);
						 break;
					}

					//printf("Invio...: %s\n",txmsg);
					txbytes+=strlen(txmsg);//aggiorna le statistiche dell'uso della rete.
					//printf("+–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––+\n");
					
				} //End of Inner While...
				inattesa=0;
			}
			else
			{//se il client non è il mittente gli chiede di chiudere la connessione
				//printf("La rifiuto\n");
					if((num=recv(client_fd,buffer,1024,0))==-1)
					{
							perror("recv");
							exit(1);
					}
					else if(num==0)
					{
						//printf("Connessione chiusa.\n\n");
						break;//esce dal while interno, per aspettare un nuovo client
					}
					
					buffer[num]='\0';
					
					//printf("Ricevuto: %s\n",buffer);
					rxbytes+=strlen(buffer);//aggiorna le statistiche sull'uso della rete
					strcpy(txmsg,"close");
					if(send(client_fd,txmsg,strlen(txmsg)+1,0)==-1) //invia una risposta
					{
						 fprintf(stderr,"Failure Sending Message\n");
						 close(client_fd);
						 break;
					}

					//printf("Invio...: %s\n",txmsg);
					txbytes+=strlen(txmsg);//aggiorna le statistiche sull'uso della rete.
			}
		
			close(client_fd);//Close Connection Socket
	
	}//Fine While esterno

    close(socket_fd);
	//printf("************Fine ricevi**************\n\n");
	
	return buffer;
}//fine ricevi



void netPartita(struct carta mazzo[])
{
	printf("Nuova partita iniziata.\n");
	printf("Giocatore 1: %s\n",p[0].hostname);
	printf("Giocatore 2: %s\n\n",p[1].hostname);
	
	srand(time(NULL));
	
	struct carta campo[2];		//carte in campo
	struct carta scarti[2][40];	//mazzo di carte vinte da ogni giocatore
	int scartate[2];	//numero di carte vinte da ogni giocatore
	struct carta vuota;
	vuota.val=15;
	vuota.seme=15;
	struct carta briscola;
	int turno,scelta;
	int winner;	//vincitore della mano
	int primo; //primo giocatore della mano
	int stato=1;	//stato della partita. 1=in corso, 0=finita
	int stmano=0;
	char txmsg[1024];

	p[0].punteggio=0;
	p[0].vittorie=0;

	p[1].punteggio=0;
	p[1].vittorie=0;

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

	int n=34; //numero di carte attualmente nel mazzo

	int i;
	for(i=0;i<n;i++)           //sposto tutte le carte indietro di 6 posizioni
	    mazzo[i]=mazzo[i+6];

	mazzo=(struct carta*)realloc(mazzo,sizeof(struct carta)*n);//realloco la memoria del mazzo più corto


	turno=rand()%2;//sceglie casualmente chi deve iniziare la partita
	
	char temp[20];
	for(i=0;i<2;i++)
	{
		strcpy(txmsg,"Cards:");
		
		temp[0]=(char)p[i].mano[0].seme+'0';
		temp[1]=(char)p[i].mano[0].val+'0';
		temp[2]='-';
		temp[3]=(char)p[i].mano[1].seme+'0';
		temp[4]=(char)p[i].mano[1].val+'0';
		temp[5]='-';
		temp[6]=(char)p[i].mano[2].seme+'0';
		temp[7]=(char)p[i].mano[2].val+'0';
		
		temp[8]='\0';

		//sostituisce eventuali : (messi al posto di 10) con x
		if(temp[1]==':')
			temp[1]='x';
		if(temp[4]==':')
			temp[4]='x';
		if(temp[7]==':')
			temp[7]='x';
		
		/*
		//da eliminare
		printf("\n\nCarte di giocatore %d: %c%c%c%c%c%c\n",i,(char)p[i].mano[0].seme+'0',(char)p[i].mano[0].val+'0',(char)p[i].mano[1].seme+'0',(char)p[i].mano[1].val+'0',(char)p[i].mano[2].seme+'0',(char)p[i].mano[2].val+'0');
		printf("TEMP: %s\n\n",temp);
		*/
		
		strcat(txmsg,temp);
		
		//accoda alle carte la briscola
		temp[0]=':';
		temp[1]=(char)briscola.seme+'0';
		temp[2]=(char)briscola.val+'0';
		if(temp[2]==':')
			temp[2]='x';
		temp[3]='\0';
		strcat(txmsg,temp);
		
		//accoda il numero di carte nel mazzo
		temp[0]=':';
		temp[1]=n+'<';
		temp[2]='\0';
		strcat(txmsg,temp);
		
		//accoda l'id del giocatore attuale e del prossimo
		temp[0]=':';
		temp[1]=i+'0';		//id
		temp[2]=turno+'0';	//turno
		temp[3]=stmano+'0';		//stato della mano
		temp[4]='5';		//vincitore. 5=ingioco 1=1 0=0
		temp[5]='\0';
		strcat(txmsg,temp);
		printf("TX: %s\n",txmsg);
		trasmetti(i,txmsg);
	}
	
	
	//pone a 0 il numero di carte vinte da ciascun giocatore
	scartate[0]=0;
	scartate[1]=0;
	
	//printf("%d",turno);
	primo=turno;
    //printf("È il turno di %s.\n",p[turno].nome);
	//printf("Premi [Invio] per iniziare la partita.");
	//getchar();//		
    //system(CLEAR);
	while(stato)
	{
		if(turno==primo)
		{
			stmano=0;
			
			strcpy(buffer,"0trn:nn:");
			strcat(buffer,p[inverti(turno)].nome);
			
			trasmetti(turno,buffer);	//fa iniziare il turno al primo giocatore, trasmettendogli "0trn"
			/*
			printf("***Turno di %s***\n",p[turno].nome);
			printf("Carte nel mazzo: %d/40\n\n",n);
			printf("Briscola: %s di %s.\n\n\n",getStringByCard(briscola.val),getStringBySuit(briscola.seme));
			*/
			//stampa le carte in mano al giocatore corrente
			/*
			printf("\nLa tua mano:\n");
			for(i=0;i<p[turno].inmano;i++)
				printf("%d) %s di %s\n",i+1,getStringByCard(p[turno].mano[i].val),getStringBySuit(p[turno].mano[i].seme));
			*/
			
			scelta=ricevi(turno)[0]-'0';//riceve la scelta del primo giocatore
			
			//printf("Scelta del giocatore: %d (%s di %s)\n",scelta,getStringByCard(p[turno].mano[scelta].val),getStringBySuit(p[turno].mano[scelta].seme));
				
			//TODO controllare se la scelta è valida
			//printf("\nChe carta vuoi giocare? ");
			//scanf("%d",&scelta);
			//getchar();
			campo[0]=p[turno].mano[scelta];

			//toglie dalla mano del giocatore la carta giocata
			p[turno].inmano--;
			if(scelta==0)
			{
				p[turno].mano[0]=p[turno].mano[1];
				p[turno].mano[1]=p[turno].mano[2];
				p[turno].mano[2]=vuota;
			}
			if(scelta==1)
			{
				p[turno].mano[1]=p[turno].mano[2];
				p[turno].mano[2]=vuota;
			}
			if(scelta==2)
			{
				p[turno].mano[2]=vuota;
			}

			//passa il turno all'avversario
			//system(CLEAR);
			//printf("Carta giocata da %s: %s di %s\n\n\n\n\n",p[primo].nome,getStringByCard(campo[0].val),getStringBySuit(campo[0].seme));
			//printf("Premi [Invio] per iniziare il turno di %s.",p[inverti(primo)].nome);
			//getchar();

			turno=inverti(turno);
			//system(CLEAR);
		}
		else
		{
			stmano=1;
			
			strcpy(txmsg,"1trn:");
			txmsg[5]=(char)campo[0].seme+'0';
			txmsg[6]=(char)campo[0].val+'0';
			if(txmsg[6]==':')
				txmsg[6]='x';
			txmsg[7]=':';
			txmsg[8]='\0';
			strcat(txmsg,p[inverti(turno)].nome);
			//getchar();
			
			
			
			trasmetti(turno,txmsg);	//fa iniziare il turno al secondo giocatore, trasmettendogli "1trn:giocata"
			
			//TODO ricevere la scelta
			scelta=ricevi(turno)[0]-'0';//riceve la scelta del secondo giocatore
			
			/*
			printf("***Turno di %s***\n",p[turno].nome);
			printf("Carte nel mazzo: %d/40\n\n",n);
			printf("Briscola: %s di %s.\n\n\n",getStringByCard(briscola.val),getStringBySuit(briscola.seme));

			printf("Carta giocata da %s: %s di %s\n\n",p[primo].nome,getStringByCard(campo[0].val),getStringBySuit(campo[0].seme));
			*/
			
			/*
			//stampa le carte in mano al giocatore corrente
			printf("\nLa tua mano:\n");
			for(i=0;i<p[turno].inmano;i++)
				printf("%d) %s di %s\n",i+1,getStringByCard(p[turno].mano[i].val),getStringBySuit(p[turno].mano[i].seme));
			*/
			
			
			//scelta=ricevi(turno)[0]-'0';//riceve la scelta del primo giocatore
			campo[1]=p[turno].mano[scelta];
			//printf("Scelta del giocatore: %d (%s di %s)\n",scelta,getStringByCard(p[turno].mano[scelta].val),getStringBySuit(p[turno].mano[scelta].seme));
				
				
				
    		//toglie dalla mano del giocatore la carta appena giocata
			p[turno].inmano--;
			if(scelta==0)
			{
				p[turno].mano[0]=p[turno].mano[1];
				p[turno].mano[1]=p[turno].mano[2];
				p[turno].mano[2]=vuota;
			}
			if(scelta==1)
			{
				p[turno].mano[1]=p[turno].mano[2];
				p[turno].mano[2]=vuota;
			}
			if(scelta==2)
			{
				p[turno].mano[2]=vuota;
			}


			//system(CLEAR);

			//controlla il vincitore della mano
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


			//fa pescare le carte ai 2 giocatori (se il mazzo non è vuoto)
			if(n!=0)
			{
				p[winner].mano[2]=mazzo[0];
				p[inverti(winner)].mano[2]=mazzo[1];
				p[winner].inmano++;
				p[inverti(winner)].inmano++;

				//toglie dal mazzo le carte pescate
				n-=2;
				for(i=0;i<n;i++)           //sposto tutte le carte indietro di 6 posizioni
					mazzo[i]=mazzo[i+2];

				if(n==0)
					free(mazzo);
				else
					mazzo=(struct carta*)realloc(mazzo,sizeof(struct carta)*n);    //realloco la memoria del mazzo
			}


			//passa il turno al vincitore
			primo=winner;
			turno=primo;

			//se carte in mano=0 allora finisce la partita
			if(p[0].inmano==0)
				stato=inverti(stato);
			else	//altrimenti stampa il messaggio di cambio giocatore.
			{
				//system(CLEAR);
				/*
				printf("Briscola: %s di %s.\n\n\n",getStringByCard(getCardByIndex(mazzo,39)),getStringBySuit(getSuitByIndex(mazzo,39)));
				printf("Carta giocata da %s: %s di %s\n",p[primo].nome,getStringByCard(campo[0].val),getStringBySuit(campo[0].seme));
				printf("Carta giocata da %s: %s di %s\n\n",p[inverti(primo)].nome,getStringByCard(campo[1].val),getStringBySuit(campo[1].seme));
				printf("Vincitore della mano: %s\nInizio il suo turno.\n",p[winner].nome);

				
				
				//printf("Premi [Invio] per iniziare il turno di %s.",p[winner].nome);
				//getchar();
				//system(CLEAR);
				*/
				
				//TODO invia ai giocatori le carte e il vincitore					
				for(i=0;i<2;i++)
				{
					strcpy(txmsg,"Cards:");

					temp[0]=(char)p[i].mano[0].seme+'0';
					temp[1]=(char)p[i].mano[0].val+'0';
					temp[2]='-';
					temp[3]=(char)p[i].mano[1].seme+'0';
					temp[4]=(char)p[i].mano[1].val+'0';
					temp[5]='-';
					temp[6]=(char)p[i].mano[2].seme+'0';
					temp[7]=(char)p[i].mano[2].val+'0';

					temp[8]='\0';

					//sostituisce eventuali : (messi al posto di 10) con x
					if(temp[1]==':')
					temp[1]='x';
					if(temp[4]==':')
					temp[4]='x';
					if(temp[7]==':')
					temp[7]='x';


					strcat(txmsg,temp);

					//accoda alle carte la briscola
					temp[0]=':';
					temp[1]=(char)briscola.seme+'0';
					temp[2]=(char)briscola.val+'0';
					if(temp[2]==':')
					temp[2]='x';
					temp[3]='\0';
					strcat(txmsg,temp);

					//accoda il numero di carte nel mazzo
					temp[0]=':';
					temp[1]=n+'<';
					temp[2]='\0';
					strcat(txmsg,temp);

					//accoda l'id del giocatore attuale e del prossimo
					temp[0]=':';
					temp[1]=i+'0';		//id
					temp[2]=turno+'0';	//turno
					temp[3]=stmano+'0';		//stato della mano
					temp[4]=winner+'0';		//vincitore. 5=ingioco 1=1 0=0
					
					
					temp[5]=':';//TEST
					temp[6]='\0';
					strcat(txmsg,temp);

					//accoda le carte in campo
					temp[0]=campo[0].seme+'0';
					temp[1]=campo[0].val+'0';
					if(temp[1]==':')
						temp[1]='x';
					temp[2]='-';
					temp[3]=campo[1].seme+'0';
					temp[4]=campo[1].val+'0';
					if(temp[4]==':')
						temp[4]='x';
					temp[5]='\0';
					strcat(txmsg,temp);
					printf("TX: %s\n",txmsg);
					trasmetti(i,txmsg);
					
					
				}

				

			}
			//printf("\n\nVincitore della mano: %d (%s)\n\n",winner,p[winner].nome);	//stampa il vincitore
		}
	}
	//system(CLEAR);

	//conta i punti
	int j;
	for(j=0;j<2;j++)
		for(i=0;i<scartate[j];i++)
			p[j].punteggio+=scarti[j][i].eval;

	if(p[0].punteggio>p[1].punteggio)
		winner=0;
	if(p[0].punteggio<p[1].punteggio)
		winner=1;
	if(p[0].punteggio==p[1].punteggio)
		winner=-1;

	printf("Partita finita\n\n");

	//decide il vincitore
	if(winner!=-1)
	{
		printf("Vincitore: %s\n\n",p[winner].nome);
		p[winner].vittorie++;
		
		strcpy(txmsg,"Winner:");
		buffer[0]=p[inverti(winner)].punteggio+'!';
		buffer[1]='\0';
		strcat(txmsg,buffer);
		trasmetti(winner,txmsg);
		
		strcpy(txmsg,"Loser:");
		buffer[0]=p[inverti(winner)].punteggio+'!';
		buffer[1]='\0';
		strcat(txmsg,buffer);
		trasmetti(inverti(winner),txmsg);
		
		
	}
	else
	{
		printf("Pareggio!\n\n");
		p[1].vittorie++;
		p[0].vittorie++;
		
		strcpy(txmsg,"Draw");
		trasmetti(winner,txmsg);
		trasmetti(inverti(winner),txmsg);
		
	}

	printf("Giocatore:\r\t\t%s\r\t\t\t\t\t%s\n",p[0].nome,p[1].nome);
	printf("Punteggio:\r\t\t%d\r\t\t\t\t\t%d\n",p[0].punteggio,p[1].punteggio);
	printf("Vittorie:\r\t\t%d\r\t\t\t\t\t%d\n",p[0].vittorie,p[1].vittorie);
	printf("\nUtilizzo della rete\nByte ricevuti: %d\nByte inviati: %d",rxbytes,txbytes);
	//getchar();

}


int main(int argc, char *argv[])
{
	/*
	 * Accetta due giocatori, dopodichè inizia una partita.
	 * NOTA: In caso di un qualsiasi errore, tutta la partita viene persa.
	 */
	 
    struct sockaddr_in server;
    struct sockaddr_in dest;
    int socket_fd, client_fd,num;
    //int status;
    socklen_t size;
	char *token;
    char buffer[1024];
    char txmsg[1024];
    const char s[2] = ":";	//carattere usato per dividere le varie parti del messaggio
    //char *buff;
//  memset(buffer,0,sizeof(buffer));
    int yes =1;
	int registra=0;//id del giocatore da registrare. Verrà usato per associare un indirizzo IP ad un giocatore.
	char percorso[20];

    if(argc!=2)
        {
			printf("Uso: gameserver porta\n%sParametro porta non fornito, verrà usato %d%s\n",rosso,PORT,reset);   
		}
    else
		{			
			PORT=atoi(argv[1]);	
			
			
			strcpy(percorso,"srv/");
			strcat(percorso,argv[1]);
			FILE *fp;
			fp = fopen(percorso, "w+");
			fprintf(fp, "Gameserver\n"); 
			fclose(fp);
		}


    if ((socket_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1) {
        perror("setsockopt");
        exit(1);
    }
    memset(&server, 0, sizeof(server));
    memset(&dest,0,sizeof(dest));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY; 
    if ((bind(socket_fd,(struct sockaddr *)&server,sizeof(struct sockaddr)))==-1)
    {//sizeof(struct sockaddr) 
        printf("%sErrore sul binding%s\nPorta in uso da un altro programma?",rosso,reset);
        exit(1);
    }

    if ((listen(socket_fd, BACKLOG))== -1){
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }
    printf("%sGameserver in ascolto sulla porta %d%s\n",verde,PORT,reset);


	while(srv.gmstato==0)
	{

			size=sizeof(struct sockaddr_in);

			if((client_fd=accept(socket_fd, (struct sockaddr *)&dest, &size))==-1)
			{
				perror("accept");
				exit(1);
			}
			
			printf("Stabilita connessione con %s\n",inet_ntoa(dest.sin_addr));
			
			while(1)
			{
                if((num=recv(client_fd,buffer,1024,0))==-1)
                {
                        perror("recv");
                        exit(1);
                }
                else if(num==0)
				{
					//printf("Connessione chiusa.\n");
					//printf("Connessione chiusa. Stat RX:%db TX:%db\r\t\t\t\t\t\t\t\t|\n",rxbytes,txbytes);
                    break;	//esce dal while interno, per aspettare un nuovo client
                }
                
                buffer[num]='\0';
                //txmsg[num]='\0';
                strcpy(txmsg,"error");

                printf("Ricevuto: %s\n",buffer);
                rxbytes+=strlen(buffer);//aggiorna le statistiche sull'uso della rete
                //printf("| Dimensione: %ld bytes.\r\t\t\t\t\t\t\t\t|\n",strlen(buffer));
                
                
				
				//salva i parametri ricevuti per messaggio
				token = strtok(buffer, s);//legge il primo token
				int i;//cicla gli altri token
				char ricevuto[10][1024];
				for(i=0;token!=NULL;i++)//Originale: while( token != NULL )
				{
					strcpy(ricevuto[i],token);
					strcpy(ricevuto[i],token);
					printf( "%s\n", ricevuto[i]);
					token = strtok(NULL, s);
				}
				
                //controlla se viene ricevuto NewGame: inizia una nuova partita e salva i nomi dei giocatori
                if(strcmp(ricevuto[0],"NewGame")==0&&srv.gmstato==0)
                {
					if(registra==0)//se non è mai stato registrato un giocatore:
					{
						printf("Giocatore 1 connesso: %s%s%s\n",grassetto,inet_ntoa(dest.sin_addr),reset);
						strcpy(p[0].hostname,inet_ntoa(dest.sin_addr)); //salva il suo hostname
						
						strcpy(p[0].nome,ricevuto[1]);//salva il suo nome
						registra=1;
						strcpy(txmsg,"okNewGame");//cambia il messaggio
					}
					else if(strcmp(p[0].hostname,inet_ntoa(dest.sin_addr))!=0)//se il giocatore da registrare è diverso dal giocatore 1
					{
						printf("Giocatore 2 connesso: %s%s%s\n",grassetto,inet_ntoa(dest.sin_addr),reset);
						strcpy(p[1].hostname,inet_ntoa(dest.sin_addr));
						registra=2;
						strcpy(p[1].nome,ricevuto[1]);//salva il suo nome
						srv.gmstato=1;//entrambi i giocatori sono connessi, quindi inizia la partita.
						strcpy(txmsg,"okNewGame");//cambia il messaggio
					}
					else
					{
						printf("Giocatore già registrato! Invio \"waitopn\"\n");
						strcpy(txmsg,"waitopn");//cambia il messaggio
					}			
				}
				
				if(strcmp(ricevuto[0],"Start")==0&&srv.gmstato==0)
                {
					if(registra==1)//se è già stato registrato un giocatore:
					{
							strcpy(txmsg,"waitopn");//cambia il messaggio: in attesa dell'avversario
					}			
				}
                
                if(send(client_fd,txmsg,strlen(txmsg)+1,0)==-1) //manda il messaggio
                {
                     fprintf(stderr,"Failure Sending Message\n");
                     close(client_fd);
                     break;
                }

                printf("Invio...: %s\n",txmsg);
                txbytes+=strlen(txmsg);//aggiorna le statistiche dell'uso della rete.
				//printf("+–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––+\n");
				
			} //End of Inner While...
			
			close(client_fd);//Close Connection Socket
	}//Fine While esterno
    close(socket_fd);
    
    if(srv.gmstato==1)
    {
		//dichiara il mazzo
		struct carta *mazzo;
		mazzo=(struct carta*)malloc(sizeof(struct carta)*40);

		//inizializza il mazzo
		initDeck(mazzo);

		//mescola il mazzo
		shufDeck(mazzo);

		//stampa il mazzo in ordine
		//printDeckByIndex(mazzo);

		//inizia una nuova partita
		netPartita(mazzo);
	}
	

	remove(percorso);
   //int status = remove(percorso);
	/*
   if( status == 0 )
      printf("File %s eliminato.\n",percorso);
   else
   {
      printf("Impossibile eliminare il file\n");
   }
    */
    
    return 0;
} //Fine di main


void initDeck(struct carta mazzo[])
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

void shufDeck(struct carta mazzo[])
{
	//mescola casualmente il mazzo di carte
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

						do
						{
							random=rand()%40; //estrae un numero casuale da 0 a 39
							ext=0;

							//scorre seme per seme II
							for(ii=0;ii<4;ii++)
								//scorre carta per carta II
								for(jj=1;jj<=10;jj++)
									if(random==mazzo[10*ii+jj-1].index)
										ext=1;
						}while(ext==1);

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

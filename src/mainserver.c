/*
 * Questo server si occupa di gestire le nuove connessioni ed associare
 * fra loro i vari giocatori in attesa. Per ogni coppia di giocatori
 * viene avviato un 'gameserver' che si chiuderà autmoaticamente alla
 * fine della partita, liberando la porta che gli era stata riservata
 * da questo programma.
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


#define BACKLOG 10

char *rosso="\033[1;31m";
char *verde="\033[1;32m";
char *reset="\033[0m";
char *grassetto="\033[1m";


int main(int argc, char *argv[])
{
    struct sockaddr_in server;
    struct sockaddr_in dest;
    int socket_fd, client_fd,num;
    //int status;
    socklen_t size;
	int txbytes=0;
	int rxbytes=0;
    char buffer[1024];
    char fname[20];
    //char *buff;
//  memset(buffer,0,sizeof(buffer));
    int yes=1;
	int iwait=0;		//1 o 0 a seconda se è presente o meno un gameserver in attesa
	int portwait;	//porta dell'eventuale gameserver in attesa
	int PORT=3490;
    if(argc!=2)
        printf("Uso: mainserver PORTA\nNumero di porta non fornito, verrà usata la porta %d\n",PORT);
    else
		PORT=atoi(argv[1]);




    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
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

    if ((listen(socket_fd, BACKLOG))==-1){
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }
    printf("%sServer in ascolto sulla porta %d%s\n",verde,PORT,reset);


	while(1)
	{

			size=sizeof(struct sockaddr_in);

			if((client_fd=accept(socket_fd, (struct sockaddr *)&dest, &size))==-1)
			{
				perror("accept");
				exit(1);
			}

			printf("+–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––+\n");
			printf("| Stabilita connessione con %s%s%s\r\t\t\t\t\t\t\t\t|\n",grassetto,inet_ntoa(dest.sin_addr),reset);
			//printf("+–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––+\n");

			while(1)
			{
                if((num=recv(client_fd,buffer,1024,0))==-1)
                {
                        perror("recv");
                        exit(1);
                }
                else if(num==0)
				{

					printf("| Connessione chiusa. Stat RX:%db TX:%db\r\t\t\t\t\t\t\t\t|\n",rxbytes,txbytes);
					printf("+–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––+\n\n");

                    //So I can now wait for another client
                    break;
                }

                buffer[num]='\0';
                printf("| Ricevuto: %s\r\t\t\t\t\t\t\t\t|\n",buffer);
                //printf("| Dimensione: %ld bytes.\r\t\t\t\t\t\t\t\t|\n",strlen(buffer));
                rxbytes+=strlen(buffer);
                printf("+–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––+\n");


					const char sep[2] = ":";//usato per la divisione del messaggio
					char *token;
					char mess[7][1024];
					int i;
					token = strtok(buffer, sep);//ottiene il primo elemento
					for(i=0;token != NULL ;i++) //cicla gli altri elementi
					{
					  //printf( " %s\n", token );
						strcpy(mess[i],token);
					  token = strtok(NULL, sep);
					}

                strcpy(buffer,"error");	//cambia il messaggio da inviare

                char chporta[6];
                if(strcmp(mess[0],"NewGame")==0)
                {
					//printf("Richiesta di una nuova partita!");
					//controlla se c'è un giocatore in attesa
					if(iwait)
					{
						snprintf(chporta, 6, "%d", portwait);
						strcpy(buffer,"NewPort:");	//cambia il messaggio da inviare
						strcat(buffer,chporta);		//accoda al messaggio la porta del gameserver
						iwait=0;
					}
					else	//altrimenti avvia un gameserver
					{
						printf("| Avvio un nuovo Gameserver\r\t\t\t\t\t\t\t\t|\n");
						for(i=7500;iwait==0;i++)
						{

							snprintf(fname, 20, "srv/%d", i);
							snprintf(chporta, 6, "%d", i);

							if( access( fname, F_OK ) != -1 )
							{
								printf("| Porta %s occupata.\r\t\t\t\t\t\t\t\t|\n",chporta);
							}
							else
							{
								printf("| Porta %s libera.\r\t\t\t\t\t\t\t\t|\n",chporta);
								//avvia game server sulla porta i
								char comando[50];

								//strcpy(comando,"gnome-terminal -x ./gameserver ");
                                strcpy(comando,"./gameserver ");
								strcat(comando,chporta);
								system(comando);
								iwait=1;	//segnala che il gameserver è in attesa di un secondo giocatore
								portwait=i;
								strcpy(buffer,"NewPort:");	//cambia il messaggio da inviare
								strcat(buffer,chporta);		//accoda al messaggio la porta del gameserver
							}
						}
					}
					sleep(2);
				}



                if(send(client_fd,buffer,strlen(buffer),0)==-1) //manda il messaggio
                {
                     fprintf(stderr,"Failure Sending Message\n");
                     close(client_fd);
                     break;
                }

                printf("| Invio...: %s\r\t\t\t\t\t\t\t\t|\n",buffer);
                //printf("| Dimensione: %ld bytes.\r\t\t\t\t\t\t\t\t|\n",strlen(buffer));
                txbytes+=strlen(buffer);
				printf("+–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––+\n");

			} //End of Inner While...
			close(client_fd);//Close Connection Socket
	}//Fine While esterno

    close(socket_fd);
    return 0;
} //End of main

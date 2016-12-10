#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#define true 1
#define false 0
#define COD_WHO 1
#define COD_INFO 2
#define OK 3
#define COD_QUIT 5
#define COD_CON_REQ 8
#define COD_CON_REF 9
#define COD_CON_ACC 10
#define COD_CON_REF_OCC 11
#define COD_CON_REFUSED 12
#define DISCONNECT 13
#define SHOT_DIM 3
#define YOU_WON 14
#define COD_SHOT 15
#define COD_HIT 16

#define USERNAME_LEN 30
#define COD_FAST_QUIT 7
#define IP_LEN 16
#define N_NAVI 7
#define SQUARE_DIM 3
#define DIM_TABELLA 36
//DEVI RICAVARE L?IP PER POI MANDARE UDP E POI FARE LA INETPTON

enum StatoCasella{OCCUPATA,COLPITA,MANCATA,VUOTO};
// nave posizionata o //nave non colpita ~ // nave colpita     x	// Vuoto ?
int sd,sudp;
char username[USERNAME_LEN];
int waitingConnect;
int inGame;
int myturn;
int naviRimaste;
//int naviDaPosizionare;
enum StatoCasella tabella[DIM_TABELLA];
enum StatoCasella tabellaAvversaria[DIM_TABELLA];
struct rival opponent;//=(struct rival*)malloc(sizeof(struct rival));

struct rival{
	char user[USERNAME_LEN];
	int udp;
	char ip[IP_LEN];
};
int insert(char*buf,int dim){
    char *foo; 
    memset(buf,0,dim);
    scanf("%ms", &foo);
    if(strlen(foo)>dim){
    	free(foo);
    	return -1;
    }
    strncpy(buf,foo,dim);
    free(foo);
    return 1;
}

void controllaReceive(int ret){
	if(ret==-1){
		perror("Errore nella recv");
		exit(1);
	}	
}
void stampa(enum StatoCasella *b){
	int i=0;
	int riga=0;
	printf("  A B C D E F");
	for(;i<36;i++){
		if((i%6)==0)
			printf("\n%d ",++riga);
		switch(b[i]){
			case OCCUPATA:
				printf("o ");
				break;
			case MANCATA: 
				printf("~ ");
				break;			
			case COLPITA:
				printf("x ");
				break;
			case VUOTO:
				printf("? ");
				break;
		}
	}
	printf("\n");
	printf("\n");
}
int ottieniX(char *buf){
	if(strcmp("A",buf)==0)
		return 1;
	if(strcmp("B",buf)==0)
		return 2;
	if(strcmp("C",buf)==0)
		return 3;
	if(strcmp("D",buf)==0)
		return 4;
	if(strcmp("E",buf)==0)
		return 5;
	if(strcmp("F",buf)==0)
		return 6;
	return -1;
}
void resettaGriglia(enum StatoCasella*b){
	int i;
	for ( i = 0; i < 36; ++i)
	{
		b[i]=VUOTO;
	}
}
int controllaCasella(char *buf,enum StatoCasella s,enum StatoCasella *b,int*xx,int*yy){
	int y=atoi((const char*)&buf[2]);
	char *p=strtok(buf,",");
	if(p==NULL){
		printf("casella non valida p=null \n");
		return true;
	}
	int x=ottieniX(p);
	if(x==-1 || y>6 || y<1){
		printf("casella non valida \n");
		return true;
	}
	if(	b[x-1+(y-1)*6]!=VUOTO&& s==VUOTO){
		printf("nave già posizionata in questa casella \n");
		return true;
	}
	if(	b[x-1+(y-1)*6]!=VUOTO&& s==VUOTO){
		printf("nave già posizionata in questa casella \n");
		return true;
	}
	if(	b[x-1+(y-1)*6]!=VUOTO&& s==MANCATA){
		printf("hai già colpito questa casella \n");
		return true;
	}
	*xx=x;
	*yy=y;
	return false;
}
void posizionaNavi(enum StatoCasella *b){
	resettaGriglia(b);
	int i=0;
	int x,y;
	char buf[4];
	printf("\rInserire le coordinate in cui posizionare le %d navi indicando le caselle [A-F],[1-6] separate da virgola\n",N_NAVI);
	int ret;
	while (i<N_NAVI){
		ret=insert(buf,SQUARE_DIM);
		if(ret==-1){
			printf("casella non valida\n");
			continue;
		}
		if(controllaCasella(buf,VUOTO,b,&x,&y))
			continue;
		b[x-1+(y-1)*6]=OCCUPATA;
		i++;
	}
	stampa(b);
}
void inviaInt(int sd, int msgl){
	int ret;
	int msg=htonl(msgl);
	ret=send(sd,(void*)&msg,sizeof(int),0);
	if(ret==-1){
		perror("Errore nell'invio dell'intero al client");
		exit(1);
	}
}
void inviaIntUdp(int sd, int msgl,struct sockaddr_in*cl){
	int ret;
	int msg=htonl(msgl);
	ret=sendto(sd,(void*)&msg,sizeof(int),0,(struct sockaddr*)cl,sizeof(*cl));
	if(ret==-1){
		perror("Errore nell'invio dell'intero al client");
		exit(1);
	}
}
void initializeSockaddr(char * ip, int p,struct sockaddr_in*cl){
	memset(cl,0,sizeof(*cl));

	cl->sin_family=AF_INET;
	cl->sin_port=htons(p);
	inet_pton(AF_INET,ip,&cl->sin_addr);
}
void help(){
	printf("\nSono disponibili i seguenti comandi:\n!help --> mostra l'elenco dei comandi disponibili \n!who --> mostra l'elenco dei client connessi al server\n!connect username --> avvia una partita con l'utente username\n!quit --> disconnette il client dal server\n");	
}
void helpGame(){
	printf("\nSono disponibili i seguenti comandi:\n!help --> mostra l'elenco dei comandi disponibili \n!disconnect --> disconnette il client dall'attuale partita\n!shot square --> fai un tentativo con la casella square\n!show --> visualizza la griglia di gioco\n");	
}
int quantiByte(int i){
	int ret;
	uint32_t dimMsg;
	ret=recv(i,(void*)&dimMsg,sizeof(int),0);
	controllaReceive(ret);
	int dimMsg2=(int)ntohl(dimMsg);
	printf("\rbyte ricevuti: %d \n",ret);
	if(ret==0){
		printf("Il server si è disconnesso\n");
		close(sd);
		exit(1);
	}
	printf("\ril server vuole mandare %d byte\n",dimMsg2 );
	return dimMsg2;
}
int quantiByteUdp(int i,struct sockaddr_in*cl){
	int ret;
	uint32_t dimMsg;
	socklen_t addrlen=sizeof(*cl);
	ret=recvfrom(i,(void*)&dimMsg,sizeof(int),0,(struct sockaddr*)cl,&addrlen);
	controllaReceive(ret);
	int dimMsg2=(int)ntohl(dimMsg);
	printf("\rbyte ricevuti: %d \n",ret);
	if(ret==0){
		printf("Il server si è disconnesso\n");
		close(sd);
		exit(1);
	}
	printf("\ril server vuole mandare %d byte\n",dimMsg2 );
	return dimMsg2;
}
void riceviByte(int i, void*buf,int dimMsg){
	int ret;
	ret=recv(i,(void*)buf,dimMsg,0);
	controllaReceive(ret);
	printf("\rbyte ricevuti: %d \n \n",ret);
}
void inviaByte(int sd, int dim, void * msg){
	int ret;
	inviaInt(sd,dim);
	ret=send(sd,msg,dim,0);
	if(ret==-1){
		perror("Errore nell'invio delle informazioni al server");
		exit(1);
	}
}
int insertUsername(char*user){
    scanf("%s",username);
    int dimMsg=strlen(username);
    if(dimMsg>=USERNAME_LEN){
        dimMsg=USERNAME_LEN;
        username[USERNAME_LEN-1]='\0';
        printf("username troppo lungo verrà inviato %s\n",user);
    }
    else
        dimMsg++;
    return dimMsg;
}
void who(int sd){
	printf("provo invio al server il codice %d \n",COD_WHO);
	inviaInt(sd,COD_WHO);
}
void quit(int sd,char*username){
	printf("provo invio al server il codice %d \n",COD_QUIT);
	inviaInt(sd,COD_QUIT);
	printf("Mi disconnetto dal server:\n");
	close(sd);
}
void connectUser(int sd,struct rival*opp){
	char user[USERNAME_LEN];
	int dimMsg;
	scanf("%30s",user);
	dimMsg=strlen(user)+1;
	printf("%d\n",dimMsg );
	if(dimMsg>=USERNAME_LEN){
		printf("comando non valido lo username è troppo lungo\n");
		return;
	}	
	printf("ti vuoi connettere a %s\n", user);
	if(waitingConnect==true){
		printf("non puoi fare una connect sei già in attesa\n");
		return;
	}
	if(strcmp(user,username)==0){
		printf("non puoi fare una connect a te stesso\n");
		return;
	}
	strcpy(opp->user,user);
	waitingConnect=true;
	inviaInt(sd,COD_CON_REQ);
	printf("sto per mandare al server %s\n", user);
	inviaByte(sd,dimMsg,user);
}
void disconnect(int sd){
	myturn=false;
	inGame=false;
	inviaInt(sd,DISCONNECT);
	printf("Ti sei disconnesso correttamente\n");
	resettaGriglia(tabellaAvversaria);
}
void shot(struct sockaddr_in*cl,struct rival*opp,int *x,int *y){
	char buf[5];
	int ret=insert(buf,SQUARE_DIM);
	if(ret==-1){
		printf("casella non valida\n");
		return;
	}
	printf("invio la casella %s\n",buf);
	if(controllaCasella(buf,MANCATA,tabellaAvversaria,x,y)){
		printf("casella non valida o hai già colpito sulla suddetta\n");
		return;
	}
	inviaIntUdp(sudp,COD_SHOT,cl);
	inviaIntUdp(sudp,SHOT_DIM,cl);
	ret=sendto(sudp,buf,SHOT_DIM,0,(struct sockaddr*)cl,sizeof(*cl));
	printf("il valore di ritorno della shot %d\n",ret );
	if(ret==-1){
		perror("sendto");
		exit(1);
	}
	myturn=false; 
}
void inserisciComando(int sd,char *buf,char*username,struct rival*opp,enum StatoCasella *b,struct sockaddr_in*cl,int *x,int*y){
		scanf("%19s",buf); ///scanf/"%ms",&buf delego al SO
		printf("%s\n",buf );
		if(strcmp("!quit",buf)==0&&inGame==false){
			quit(sd,username);
			exit(0);
		}
		if(strcmp("!help",buf)==0&&inGame==false){
			help();
			return;
		}
		if(strcmp("!help",buf)==0&&inGame==true){
			helpGame();
			return;
		}
		if(strcmp("!disconnect",buf)==0&&inGame==true){
			naviRimaste=N_NAVI;
			disconnect(sd);
			printf("TI SEI ARRESO\n");
			return;
		}
		if(strcmp("!connect",buf)==0&&inGame==false){ 
			connectUser(sd,opp);
			printf("ritorno da connectUser\n");
			return;
		}
		if(strcmp("!who",buf)==0&&inGame==false){
			printf("compare riconosciuto who\n");
			who(sd);
			return;
		}
		if((strcmp("y",buf)==0)&&(opp->udp!=0)){
			printf("Hai accettato la partita con %s\n",opp->user);
			printf("sulla porta: %d\n",opp->udp );
			initializeSockaddr(opp->ip, opp->udp,cl);
			inviaInt(sd,COD_CON_ACC);
			waitingConnect=false;
			myturn=false;
			inGame=true;
			posizionaNavi(b);   
			helpGame();
			return;
		}
		if((strcmp("n",buf)==0)&&(opp->udp!=0)){
			printf("Hai rifiutato la partita con %s\n",opp->user);
			opp->udp=0;
			waitingConnect=false;
			inviaInt(sd,COD_CON_REF);
			return;
		}
		if((strcmp("!show",buf)==0)&&(inGame)){
			printf("Stampo la tua griglia\n");
			stampa(tabella);
			printf("Stampo la griglia avversaria\n");
			stampa(tabellaAvversaria);
			return;
		}
		if((strcmp("!shot",buf)==0)&&(myturn)){
			printf("sparo\n");
			shot(cl,opp,x,y);
			return;
		}
		if((strcmp("!shot",buf)==0)&&(!myturn)){
			printf("non puoi sparare non è il tuo turno\n");
			return;
		}
		printf("Comando digitato non riconosciuto, riprovare\n");
}
void inserisciPorta(int * portaAscolto){
	while(1){
		printf("Inserisci la tua porta UDP di ascolto: ");
		scanf("%d",portaAscolto);
		if(*portaAscolto<1025||*portaAscolto>0xFFFF){
			printf("Porta indicata non valida, RIPROVARE\n");
			continue;
		}
		break;
	}
}

int inviaInfo(int sd,char *username){
	int portaAscolto;
	while(1){
		printf("Inserisci il tuo nome client: ");
		int dimMsg=insertUsername(username);
		inserisciPorta(&portaAscolto);
		inviaInt(sd,COD_INFO);
		inviaInt(sd,portaAscolto);
		printf("invio :%s\n",username );
		inviaByte(sd,dimMsg,username);
		int p=quantiByte(sd);
		if(p==OK){
			printf("Username accettato dal server\n");
			break;
		}
		printf("codice di ritorno %d\n",p );
		printf("username già presente sul server: %s\n", username );
	}
	return portaAscolto;
}
void mysigint(){
	printf("entro nella mysigint\n");
	if(strcmp(username,"")==0){
		printf("esco senza cancellarmi\n");
		inviaInt(sd,COD_FAST_QUIT);
		close(sd);
		exit(0);
	}
	if(inGame==true){
		disconnect(sd);
		printf("TI SEI ARRESO\n");
	}
	quit(sd,username);
	exit(0);
}
void recvWho(int sd){
	int dimMsg=quantiByte(sd);
	char buf[dimMsg];
	riceviByte(sd,buf,dimMsg);
	printf("Clienti connessi al server:\n");
	printf("%s\n",buf );
}

void connectRequest(int sd,struct rival*opp,struct sockaddr_in*cl){
	int dim=quantiByte(sd);
	char user[dim];
	riceviByte(sd,user,dim);
	strcpy(opp->user,user);
	opp->udp=quantiByte(sd); //ricevo la porta udp
	dim=quantiByte(sd);
	riceviByte(sd,opp->ip,dim);
	initializeSockaddr(opp->ip, opp->udp,cl);
	printf("Ti vuoi connettere con il client %s y/n?\n",user);
}

void decripta(int cod, int sd,struct rival *opp,enum StatoCasella *b,struct sockaddr_in*cl){
	printf("sto decriptando\n");
	switch(cod){
		case COD_WHO:
			printf("ricevuta una who\n");
			recvWho(sd);
			break;
		case COD_CON_REF:
			opp->udp=0;
			printf("connect rifiutata utente inesistente\n");
         	waitingConnect=false;
			break;
		case COD_CON_REFUSED:
			opp->udp=0;
			printf("partita rifiutata da: %s\n",opp->user);
         	waitingConnect=false;
			break;
		case COD_CON_REF_OCC:
			opp->udp=0;
			printf("connect rifiutata utente occupato\n");
         	waitingConnect=false;
			break;
		case COD_CON_REQ:            //richiesta di connessione di un altro socket
			printf("richiesta di connessione\n");
			connectRequest(sd,opp,cl);
			break;
		case COD_CON_ACC:       
			printf("richiesta di connessione accettata con: %s\n",opp->user);
			opp->udp=quantiByte(sd);
			int dim=quantiByte(sd);
			riceviByte(sd,opp->ip,dim);
			initializeSockaddr(opp->ip, opp->udp,cl);
			inGame=true;   
			waitingConnect=false;  
			posizionaNavi(b);
			helpGame();
			myturn=true;
			break;
		case DISCONNECT:
			if(inGame==true){
				naviRimaste=N_NAVI;
				inGame=false;
				resettaGriglia(tabellaAvversaria);
				printf("HAI VINTO!!! %s si è arreso\n",opp->user);
				opp->udp=0;
				strcpy(opp->ip,"");
			}
			break;
		printf("codifica non riconosciuta\n");
		break;
	}
}
void riceviUdp(struct sockaddr_in*cl){
	char buf[4];
	int ret;
	socklen_t addrlen=sizeof(*cl);
	int dim =quantiByteUdp(sudp,cl);
	ret=recvfrom(sudp,(void*)buf,dim,0,(struct sockaddr*)cl,&addrlen);
	printf("l'avversario ha sparato in %s\n", buf);
	if(ret==-1){
		printf("errore recvfrom\n");
		exit(1);
	}
	int y=atoi((const char*)&buf[2]);
	char *p=strtok(buf,",");
	int x=ottieniX(p);
	myturn=true;
	inviaIntUdp(sudp,COD_HIT,cl);
	if(tabella[x-1+(y-1)*6]==VUOTO){
		inviaIntUdp(sudp,false,cl);
		tabella[x-1+(y-1)*6]=MANCATA;
		return;
	}
	printf("HAi perrso una nave\n");
	naviRimaste--;
	tabella[x-1+(y-1)*6]=COLPITA;
	if(naviRimaste==0){
		naviRimaste=N_NAVI;
		inviaIntUdp(sudp,YOU_WON,cl);
		printf("HAI PERSO :( :( :( \n");
		inGame=false;
		resettaGriglia(tabellaAvversaria);
		opponent.udp=0;
		strcpy(opponent.ip,"");
		disconnect(sd);
		return;
	}
	inviaIntUdp(sudp,true,cl);

}

void decriptaUdp (int cod,struct sockaddr_in*cl,int *x,int *y){
	printf("sto decriptando\n");
	switch(cod){
		case COD_SHOT:
			printf("ricevuta una shot\n");
			riceviUdp(cl);
			break;
		case true:
			printf("Hai colpito l'avversario\n");
			tabellaAvversaria[*x-1+(*y-1)*6]=COLPITA;
			break;
		case false:
			printf("Hai mancato l'avversario\n");
			tabellaAvversaria[*x-1+(*y-1)*6]=MANCATA;
			break;
		case YOU_WON:
			printf("HAI VINTO!!!!!!!!!!!!\n");
			naviRimaste=N_NAVI;
			inGame=false;
			resettaGriglia(tabellaAvversaria);
			opponent.udp=0;
			strcpy(opponent.ip,"");
			break;
		printf("codifica non riconosciuta\n");
		break;
	}
}
int main(int argc,char* argv[]) {
	naviRimaste=N_NAVI;
	opponent.udp=0;
	strcpy(opponent.ip,"");
    strcpy(username,"");
    waitingConnect=false;
    resettaGriglia (tabellaAvversaria);
   	inGame=false;
    if (signal(SIGINT, mysigint) == SIG_ERR)
        printf("Cannot handle SIGINT!\n");
    if(argc<2){
        printf("Errore passaggio argomenti al server\n");
        exit(1);
    }
    int porta=atoi((const char*)argv[2]);
    char * ip=argv[1];
    int ret;
    sd=socket(AF_INET,SOCK_STREAM,0);
    if(sd==-1){
        perror("Errore nella creazione del socket ");
        exit(1);
    }
    struct sockaddr_in server;
    memset(&server,0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(porta);
    inet_pton(AF_INET,ip,&server.sin_addr);

    ret=connect(sd,(struct sockaddr*)&server,sizeof(server));
    if(ret==-1){
        printf("connessione al server %s (porta %d) fallita \n",ip,porta);
        perror("Errore nella connessione ");
        exit(1);
    }
    printf("connessione al server %s (porta %d) riuscita\n",ip,porta);
    help();
    int udp=inviaInfo(sd,username);
    char buf[20];
    fd_set master;
    fd_set read;
    int fdmax;
    FD_ZERO(&master);
    FD_ZERO(&read);
    
    sudp= socket(AF_INET,SOCK_DGRAM, 0);	
   struct sockaddr_in my_addr,client;
    memset(&server,0,sizeof(my_addr));
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(udp);
    my_addr.sin_addr.s_addr=INADDR_ANY;

    ret=bind(sudp,(struct sockaddr*)&my_addr,sizeof(my_addr));
    if(ret==-1){
    	perror("bind ");
    	exit(1);
    }
    FD_SET(sudp,&master);
    FD_SET(sd,&master);
    FD_SET(0,&master);
    if(sd>sudp)
    	fdmax=sd;
    else
    	fdmax=sudp;
    int i;
    struct timeval timeout;
    timeout.tv_sec=20;
    timeout.tv_usec=0;
    int retval;
    for(;;){
    	if(inGame==false){
	    	printf("> ");
	        fflush(stdout);
	    }else{   
	       	if(myturn){
	       		printf("è il tuo turno\n");
	       		printf("Navi rimaste: %d\n", naviRimaste);
	       	}
	        printf("# ");
	        fflush(stdout);	
	    }
	    int x,y;
        read=master;
        retval=select(fdmax+1,&read,NULL,NULL,&timeout);
        if(retval==0){
        	timeout.tv_sec=20;
        	if(inGame){
	        	printf("timeout scaduto\n");
	        	if(myturn){
	        		//myturn=false;
	        		disconnect(sd);
					printf("TI SEI ARRESO\n");
	        	}
	        }
        	continue;
        }
        for(i=0;i<=fdmax;i++){
            if(FD_ISSET(i,&read)){
                if(i==sd){
                	printf("richiesta dal server\n");
                	int cod=quantiByte(sd);
                	printf("%d\n",cod );
                	decripta(cod,i,&opponent,tabella,&client);
                }else if (i==0){
                    inserisciComando(sd,buf,username,&opponent,tabella,&client,&x,&y);
                }else if(i==sudp){
                	int cod=quantiByteUdp(i,&client);
                	decriptaUdp(cod,&client,&x,&y);
                }
            }
    
        }
     

    }
    return 0;
}

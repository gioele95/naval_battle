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
#include "../costants.h"

//DEVI RICAVARE L?IP PER POI MANDARE UDP E POI FARE LA INETPTON

enum StatoCasella{OCCUPATA,COLPITA,MANCATA,VUOTO};
// nave posizionata o //nave non colpita ~ // nave colpita     x	// Vuoto ?
int sd,sudp;
char username[USERNAME_LEN];
int waitingConnect;
int inGame;
int myturn;
int infoInviate;
int naviRimaste;
//int naviDaPosizionare;
enum StatoCasella tabella[TABLE_SIZE];
enum StatoCasella tabellaAvversaria[TABLE_SIZE];
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
		perror("receive error");
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
		printf("invalid cell\n");
		return true;
	}
	int x=ottieniX(p);
	if(x==-1 || y>6 || y<1){
		printf("invalid cell\n");
		return true;
	}
	if(	b[x-1+(y-1)*6]!=VUOTO&& s==VUOTO){
		printf("ship already positioned in this cell \n");
		return true;
	}
	if(	b[x-1+(y-1)*6]!=VUOTO&& s==VUOTO){
		printf("ship already positioned in this cell \n");
		return true;
	}
	if(	b[x-1+(y-1)*6]!=VUOTO&& s==MANCATA){
		printf("You've already shot in this cell \n");
		return true;
	}
	*xx=x;
	*yy=y;
	return false;
}
void inviaInt(int sd, int msgl){
	int ret;
	int msg=htonl(msgl);
	ret=send(sd,(void*)&msg,sizeof(int),0);
	if(ret==-1){
		perror("Error in sending the integer to the client");
		exit(1);
	}
}
int quantiByte(int i){
	int ret;
	uint32_t dimMsg;
	ret=recv(i,(void*)&dimMsg,sizeof(int),0);
	controllaReceive(ret);
	int dimMsg2=(int)ntohl(dimMsg);
	if(ret==0){
		printf("The server has been disconnected\n");
		close(sd);
		exit(1);
	}
	return dimMsg2;
}
void disconnect(int sd){
	myturn=false;
	inGame=false;
	inviaInt(sd,DISCONNECT);
	printf("You have successfully disconnected\n");
	resettaGriglia(tabellaAvversaria);
}
int posizionaNavi(enum StatoCasella *b,int t){
	naviRimaste=N_SHIPS;
	resettaGriglia(b);
	int j,i=0;
	int x,y,retval;
	char buf[4];
	printf("\r Insert the coordinates of the %d ships by specifying the cells [A-F],[1-6] divided by comma e.g. D,5\n",N_SHIPS);
	int ret;
	fd_set master;
    fd_set read;
    int fdmax=sd;
    FD_ZERO(&master);
    FD_ZERO(&read);
    FD_SET(0,&master);
    FD_SET(sd,&master);
    char *foo; 
 //   char nl[1];
    struct timeval timeout;
    timeout.tv_sec=TIMEOUT_SHIPS+t;
    timeout.tv_usec=0;
	while (i<N_SHIPS){
		read=master;
        retval=select(fdmax+1,&read,NULL,NULL,&timeout);
        timeout.tv_sec=TIMEOUT_SHIPS;
    //    printf("retval: %d inGame:%d\n", retval,inGame);
        if(retval==0){
        	if(inGame){
	        //	printf("timeout scaduto\n");
	        	disconnect(sd);
				printf("YOU GAVE UP\n");	
	        }
	        return false;
        } 
        for(j=0;j<=fdmax;j++){
            if(FD_ISSET(j,&read)){
            	if(j==0){
            		//while (scanf("%ms", &foo){
            			scanf("%ms", &foo);
            		///	printf("inserisco nave\n");
				   		memset(buf,0,SQUARE_DIM);				   
					    if(strlen(foo)>SQUARE_DIM){
					    	free(foo);
					    	printf("invalid cell\n");
							continue;
					    }
				    	strncpy(buf,foo,SQUARE_DIM);
				    	free(foo);
            		//	printf("nave: %s\n",buf );
            			if(controllaCasella(buf,VUOTO,b,&x,&y))
							continue;
						b[x-1+(y-1)*6]=OCCUPATA;
						i++;
					//	if(i>=N_SHIPS)
					//		break;
            		//}
				}
				else if (j==sd){
					ret=quantiByte(sd);
					if(ret==DISCONNECT){
						inGame=false;
						myturn=false;
						printf("YOU WON, the other client has disconnected\n");
						return false;
					}
				}
			}
		}
	}
	stampa(b);
	return true;
}

void inviaIntUdp(int sd, int msgl,struct sockaddr_in*cl){
	int ret;
	int msg=htonl(msgl);
	ret=sendto(sd,(void*)&msg,sizeof(int),0,(struct sockaddr*)cl,sizeof(*cl));
	if(ret==-1){
		perror("Error in sending the integer to the client");
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
	printf("\nThe following commands are available:\n!help --> displays the list of possible commands \n!who --> displays the list of clients connected to the server\n!connect username --> starts up a game with user 'username'\n!quit --> disconnects the client from the server\n");	
}
void helpGame(){
	help();
	printf("\n!shot square --> make an attempt to cell 'square'\n!show --> show the game grid\n");	
}

int quantiByteUdp(int i,struct sockaddr_in*cl){
	int ret;
	uint32_t dimMsg;
	socklen_t addrlen=sizeof(*cl);
	ret=recvfrom(i,(void*)&dimMsg,sizeof(int),0,(struct sockaddr*)cl,&addrlen);
	controllaReceive(ret);
	int dimMsg2=(int)ntohl(dimMsg);
	return dimMsg2;
}
void riceviByte(int i, void*buf,int dimMsg){
	int ret;
	ret=recv(i,(void*)buf,dimMsg,0);
	controllaReceive(ret);
	//printf("\rbyte ricevuti: %d \n \n",ret);
}
void inviaByte(int sd, int dim, void * msg){
	int ret;
	inviaInt(sd,dim);
	ret=send(sd,msg,dim,0);
	if(ret==-1){
		perror("Error in sending the info to the server");
		exit(1);
	}
}
int insertUsername(char*user){
    scanf("%s",username);
    int dimMsg=strlen(username);
    if(dimMsg>=USERNAME_LEN){
        dimMsg=USERNAME_LEN;
        username[USERNAME_LEN-1]='\0';
        printf("username is too long, you will be asssigned: %s\n",user);
    }
    else
        dimMsg++;
    return dimMsg;
}
void who(int sd){
	//printf("provo invio al server il codice %d \n",COD_WHO);
	inviaInt(sd,COD_WHO);
}
void quit(int sd){
	//printf("provo invio al server il codice %d \n",COD_QUIT);
	inviaInt(sd,COD_QUIT);
	printf("Disconnecting from server\n");
	close(sd);
	exit(0);
}
void connectUser(int sd,struct rival*opp){
	char user[USERNAME_LEN];
	int dimMsg;
	scanf("%30s",user);
	dimMsg=strlen(user)+1;
	//	printf("%d\n",dimMsg );
	if(dimMsg>=USERNAME_LEN){
		printf("invalid command the username is too long\n");
		return;
	}	
	printf("you want to connect to: %s\n", user);
	if(waitingConnect==true){
		printf("If you're already waiting for a response you cannot ask for another connect\n");
		return;
	}
	if(strcmp(user,username)==0){
		printf("you cannot connect with yourself\n");
		return;
	}
	strcpy(opp->user,user);
	waitingConnect=true;
	inviaInt(sd,COD_CON_REQ);
	//printf("sto per mandare al server %s\n", user);
	inviaByte(sd,dimMsg,user);
}

void shot(struct sockaddr_in*cl,struct rival*opp,int *x,int *y){
	char buf[5];
	int ret=insert(buf,SQUARE_DIM);
	if(ret==-1){
		printf("invalid cell\n");
		return;
	}
	//printf("invio la casella %s\n",buf);
	if(controllaCasella(buf,MANCATA,tabellaAvversaria,x,y)){
		printf("invalid or already shot cell \n");
		return;
	}
	inviaIntUdp(sudp,COD_SHOT,cl);
	inviaIntUdp(sudp,SHOT_DIM,cl);
	ret=sendto(sudp,buf,SHOT_DIM,0,(struct sockaddr*)cl,sizeof(*cl));
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
			quit(sd);
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
			naviRimaste=N_SHIPS;
			disconnect(sd);
			printf("YOU GAVE UP :(");
			return;
		}
		if(strcmp("!connect",buf)==0&&inGame==false){ 
			connectUser(sd,opp);
		//	printf("ritorno da connectUser\n");
			return;
		}
		if(strcmp("!who",buf)==0&&inGame==false){
			//printf("compare riconosciuto who\n");
			who(sd);
			return;
		}
		if((strcmp("y",buf)==0)&&(inGame)&&(opp->udp==-1)){
			printf("You have accepted the game with %s\n",opp->user);
			inviaInt(sd,COD_CON_ACC);
			return;
		}
		if((strcmp("n",buf)==0)&&(inGame)&&(opp->udp==-1)) {
			printf("You have refused the game with %s\n",opp->user);
			opp->udp=-1;
			waitingConnect=false;
			inGame=false;
			inviaInt(sd,COD_CON_REF);
			return;
		}
		if((strcmp("!show",buf)==0)&&(inGame)){
			printf("Printing your grid\n");
			stampa(tabella);
			printf("Printing your opponents grid\n");
			stampa(tabellaAvversaria);
			return;
		}
		if((strcmp("!shot",buf)==0)&&(myturn)){
			//printf("sparo\n");
			shot(cl,opp,x,y);
			return;
		}
		if((strcmp("!shot",buf)==0)&&(!myturn)){
 			printf("Wait for your turn\n");
			return;
		}
		printf("Command has not been recognised, try again\n");
}
void inserisciPorta(int * portaAscolto){
	char buf[10];
	while(1){
		printf("Insert your UDP listening port ");
		scanf("%s",buf);
		*portaAscolto=atoi((const char*)buf);
		if(*portaAscolto<1025||*portaAscolto>0xFFFF){
			printf("Invalid port, try again\n");
			continue;
		}
		break;
	}
}

int inviaInfo(int sd,char *username){
	int portaAscolto;
	while(1){
		printf("Insert your username: ");
		int dimMsg=insertUsername(username);
		inserisciPorta(&portaAscolto);
		inviaInt(sd,COD_INFO);
		inviaInt(sd,portaAscolto);
		//printf("invio :%s\n",username );
		inviaByte(sd,dimMsg,username);
		int p=quantiByte(sd);
		if(p==OK){
			printf("Username accepted from the server\n");
			break;
		}
		///printf("codice di ritorno %d\n",p );
		printf("username already exists on the server: %s\n", username );
	}
	return portaAscolto;
}
void mysigint(){
	if(inGame==true){
		disconnect(sd);
		printf("YOU GAVE UP :(\n");
		exit(0);
	}else{
		quit(sd);
	}
}
void recvWho(int sd){
	int dimMsg=quantiByte(sd);
	char buf[dimMsg];
	riceviByte(sd,buf,dimMsg);
	printf("Clients connected to the server:\n");
	printf("%s\n",buf );
}

void connectRequest(int sd,struct rival*opp,struct sockaddr_in*cl){
	int dim=quantiByte(sd);
	char user[dim];
	riceviByte(sd,user,dim);
	strcpy(opp->user,user);
	opp->udp=-1;
	inGame=true;
	printf("Do you want to with client %s y/n?\n",user);
}
void connectionInfo(struct sockaddr_in*cl){
	int dim;
	opponent.udp=quantiByte(sd); //ricevo la porta udp
	dim=quantiByte(sd);
	riceviByte(sd,opponent.ip,dim);
	initializeSockaddr(opponent.ip, opponent.udp,cl);
	waitingConnect=false;
	myturn=false;
	inGame=true;
	int app=posizionaNavi(tabella,1);   
	if(app){
		helpGame();
		printf("Wait for your turn\n");
	}
	return;
}
void decripta(int cod, int sd,struct rival *opp,enum StatoCasella *b,struct sockaddr_in*cl){
	//printf("sto decriptando\n");
	switch(cod){
		case CONN_INFO:
			connectionInfo(cl);
			break;
		case COD_WHO:
			//printf("ricevuta una who\n");
			recvWho(sd);
			break;
		case COD_CON_REF:
			opp->udp=-1;
			printf("connection refused user does not exist\n");
         	waitingConnect=false;
			break;
		case COD_CON_REFUSED:
			opp->udp=-1;
			printf("game refused by: %s\n",opp->user);
         	waitingConnect=false;
			break;
		case COD_CON_REF_OCC:
			opp->udp=-1;
			printf("connection refused user busy\n");
         	waitingConnect=false;
			break;
		case COD_CON_REQ:            //richiesta di connessione di un altro socket
			//printf("richiesta di connessione\n");
			connectRequest(sd,opp,cl);
			break;
		case COD_CON_ACC:       
			printf("connection request accepted with: %s\n",opp->user);
			opp->udp=quantiByte(sd);
			int dim=quantiByte(sd);
			riceviByte(sd,opp->ip,dim);
			initializeSockaddr(opp->ip, opp->udp,cl);
			inGame=true;   
			waitingConnect=false;  
			int app=posizionaNavi(b,0);
			if (app){
				helpGame();
				myturn=true;
			}
			break;
		case DISCONNECT:
			if(inGame==true){
				naviRimaste=N_SHIPS;
				inGame=false;
				resettaGriglia(tabellaAvversaria);
				printf("YOU WON!!! %s gave up\n",opp->user);
				opp->udp=-1;
				strcpy(opp->ip,"");
			}
			else{
				waitingConnect=false;
				resettaGriglia(tabellaAvversaria);
				printf(" %s has disconnected\n",opp->user);
			}
			break;
		printf("encoding not recognised\n");
		break;
	}
}
void riceviUdp(struct sockaddr_in*cl){
	char buf[4];
	int ret;
	socklen_t addrlen=sizeof(*cl);
	int dim =quantiByteUdp(sudp,cl);
	ret=recvfrom(sudp,(void*)buf,dim,0,(struct sockaddr*)cl,&addrlen);
	printf("opponent has shot in  %s\n", buf);
	if(ret==-1){
		printf("error recvfrom\n");
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
	printf("You lost a ship\n");
	naviRimaste--;
	tabella[x-1+(y-1)*6]=COLPITA;
	if(naviRimaste==0){
		naviRimaste=N_SHIPS;
		inviaIntUdp(sudp,YOU_WON,cl);
		printf("YOU LOST :( :( :( \n");
		inGame=false;
		resettaGriglia(tabellaAvversaria);
		opponent.udp=-1;
		strcpy(opponent.ip,"");
		disconnect(sd);
		return;
	}
	inviaIntUdp(sudp,true,cl);

}

void decriptaUdp (int cod,struct sockaddr_in*cl,int *x,int *y){
	switch(cod){
		case COD_SHOT:
			if(inGame){
			printf("received a shot\n");
			riceviUdp(cl);}
			break;
		case true:
			printf("You hit\n");
			tabellaAvversaria[*x-1+(*y-1)*6]=COLPITA;
			break;
		case false:
			printf("You missed\n");
			tabellaAvversaria[*x-1+(*y-1)*6]=MANCATA;
			break;
		case YOU_WON:
			printf("YOU WON!!!!!!!!!!!!\n");
			naviRimaste=N_SHIPS;
			inGame=false;
			resettaGriglia(tabellaAvversaria);
			opponent.udp=-1;
			strcpy(opponent.ip,"");
			break;
		printf("encoding not recognised\n");
		break;
	}
}
int main(int argc,char* argv[]) {
	naviRimaste=N_SHIPS;
	infoInviate=false;
	opponent.udp=-1;
	strcpy(opponent.ip,"");
    strcpy(username,"");
    waitingConnect=false;
    resettaGriglia (tabellaAvversaria);
   	inGame=false;
    if (signal(SIGINT, mysigint) == SIG_ERR)
        printf("Cannot handle SIGINT!\n");
    if(argc!=3){
        printf("Error passing arguments to the main\n");
        exit(1);
    }
    int porta=atoi((const char*)argv[2]);
    char * ip=argv[1];
    int ret;
    sd=socket(AF_INET,SOCK_STREAM,0);
    if(sd==-1){
        perror("Error in creating socket ");
        exit(1);
    }
    struct sockaddr_in server;
    memset(&server,0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(porta);
    inet_pton(AF_INET,ip,&server.sin_addr);

    ret=connect(sd,(struct sockaddr*)&server,sizeof(server));
    if(ret==-1){
        printf("connection at server %s (port %d) failed \n",ip,porta);
        perror("Error during connection ");
        exit(1);
    }
    printf("connection at server %s (port %d) successful\n",ip,porta);
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
    memset(&my_addr,0,sizeof(my_addr));
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(udp);
    my_addr.sin_addr.s_addr=INADDR_ANY;

    ret=bind(sudp,(struct sockaddr*)&my_addr,sizeof(my_addr));
    if(ret==-1){
    	perror("bind ");
    	quit(sd);
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
    timeout.tv_sec=TIMEOUT;
    timeout.tv_usec=0;
    int retval;
    for(;;){
    	if(inGame==false){
	    	printf("\r> ");
	        fflush(stdout);
	    }else{   
	       	if(myturn){
	       		printf("Is your turn\n");
	       		printf("Remaining ships: %d\n", naviRimaste);
	       	}
	        printf("\r# ");
	        fflush(stdout);	
	    }
	    int x,y;
        read=master;
       // while(1){
	        retval=select(fdmax+1,&read,NULL,NULL,&timeout);
	        timeout.tv_sec=TIMEOUT;
	        if(retval==0){
	        //	printf("timeout scaduto %d\n",inGame);
	        //	printf("opp.udp %d\n", opponent.udp);
	        	if(inGame){
		        	if(myturn){
		        		disconnect(sd);
						printf("YOU GAVE UP\n");
		        	}
		        	if(opponent.udp==-1){
		        		waitingConnect=false;
						inGame=false;
						inviaInt(sd,COD_CON_REF);
						printf("You did not give an answer in the required time\n");
		        	}
		    //    	break;
		        }
	        }
	      /*  else
	        	break;
	    }*/
        for(i=0;i<=fdmax;i++){
            if(FD_ISSET(i,&read)){
                if(i==sd){
                	//printf("richiesta dal server\n");
                	int cod=quantiByte(sd);
                	printf("%d\n",cod );
                	decripta(cod,i,&opponent,tabella,&client);
                }else if (i==0){
                	printf("waiting for commands\n");
                    inserisciComando(sd,buf,username,&opponent,tabella,&client,&x,&y);
                }else if(i==sudp){
                	printf("udp\n");
                	int cod=quantiByteUdp(i,&client);
                	decriptaUdp(cod,&client,&x,&y);
                }
            }
    
        }
     

    }
    return 0;
}

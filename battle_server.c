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

#define INT_DIM 2
#define COD_WHO 1
#define COD_INFO 2
#define OK 3
#define COD_QUIT 5
#define ERRORE 6
#define true 1
#define false 0
#define COD_FAST_QUIT 7
#define USERNAME_LEN 30
#define COD_CON_REQ 8
#define COD_CON_REF 9
#define COD_CON_ACC 10
#define COD_CON_REF_OCC 11
#define COD_CON_REFUSED 12
#define DISCONNECT 13

#define QUEUE_LEN 10
#define IP_LEN 16

//DISCONNETTITI CON IL I


int n_client=0;// variabile usata per stimare la dimensione del buffer per lawho

enum statoClient{LIBERO,OCCUPATO};

struct listaClient{
	int socket;
	struct listaClient * next; 
	int porta; 
	char username[USERNAME_LEN];
	char ip[IP_LEN];
	enum statoClient stato;
	char rival[USERNAME_LEN];
};
void controllaReceive(int ret,int i){
	if(ret==-1){
		perror("Errore nella recv");
	//	exit(1);
	}
	if(ret==0){
		printf("Client disconnesso \n");
		close(i);
//		exit(0);
	}	
}
void inviaInt(int sd, int msgl){
	int ret;
	int msg=htonl(msgl);
	printf("intero inviato %d\n",ntohl(msg ));
	ret=send(sd,(void*)&msg,sizeof(int),0);
	printf("%d\n",ret );
	if(ret==-1){
		perror("Errore nell'invio dell'intero al client");
		//exit(1);
	}
}
void stampaStato(enum statoClient s){
	if(s==LIBERO)
		printf("libero)\n");
	else
		printf("occupato)\n");
}
int quantiByte(int i){
	int ret;
	uint32_t dimMsg;
	ret=recv(i,(void*)&dimMsg,sizeof(int),0);
	controllaReceive(ret,i);
	int dimMsg2=(int)ntohl(dimMsg);
	printf("byte ricevuti: %d \n",ret);
	printf("il client vuole mandare %d byte\n",dimMsg2 );
	return dimMsg2;
}
void riceviByte(int i, void*buf,int dimMsg){
	int ret;
	ret=recv(i,(void*)buf,dimMsg,0);
	controllaReceive(ret,i);
	printf("byte ricevuti: %d \n \n",ret);
}
void inserimentoLista(struct listaClient ** testa, struct listaClient *p){
	p->next=*testa;
	*testa=p;
}
void inserimentoDatiLista(int i,char*buf,int udp, struct listaClient *p){
	//effettua un controllo fai inserimentoLista in coda;
	while(1 ){
		if(p->socket==i)
			break;
		p=p->next;
	}
	p->stato=LIBERO;
	strcpy(p->username,buf);
	p->porta=udp;
}
void stampaLista(struct listaClient * p){
	while(p!=NULL){
		if(p->porta!=0){
			printf("%s(",p->username );
			stampaStato(p->stato);
			printf("porta: %d\n",p->porta );
			printf("ip: %s\n\n",p->ip );
		}
		p=p->next;
	}
}
void inserisciPorta(int * portaAscolto){
	printf("Porta di ascolto passata al main non valida ");
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
int lunghezzaStato(enum statoClient s){
	if(s==LIBERO)
		return 6;
	return 8;
}
void inviaByte(int sd, int dim, void * msg){
	int ret;
	inviaInt(sd,dim);
	ret=send(sd,msg,dim,0);
	if(ret==-1){
		perror("Errore nell'invio delle informazioni al client");
		exit(1);
	}
}
void who(int i,struct listaClient * p){//faccio una stima di 20 char per client
	int dim=n_client*20;
	char*app;
	char *buf=(char*)malloc(dim*sizeof(char));
	int d=0,ii=1;
	while(p!=NULL){
		if(p->porta!=0){
			d=strlen(p->username)+2+lunghezzaStato(p->stato);
			dim-=d;
			if(dim>=0){
				strcat(buf,p->username);
				if(p->stato==LIBERO)
					strcat(buf,"(libero)");
				else
					strcat(buf,"(occupato)");
				strcat(buf,"\n");
			}else{
				dim=n_client*(20+5*ii);
				app=(char*)malloc(dim*sizeof(char));
				strcpy(app,buf);
				dim-=strlen(buf);
				free((void*)buf);
				buf=app;
				i++;
				continue;	
			}
		}
		p=p->next;

	}
	printf("invio nella who di: %s\n a",buf );
	dim=strlen(buf)+1;
	inviaInt(i,COD_WHO);
	inviaByte(i,dim,buf);
}
void rimuoviDaLista(int i,struct listaClient ** testa){
	struct listaClient * p=*testa;
	struct listaClient * prev=NULL;
	while(p!=NULL){
		if(p->porta!=0){
			if(i==p->socket){
				printf("rimuovo l'utente username %s\n",p->username );
				if (prev==0)
					*testa=p->next;
				else{
					prev->next=p->next;
				}
				free(p);
			}
		}
		prev=p;
		p=p->next;
	}
}
void quit(int i,struct listaClient ** testa,fd_set*master){
	n_client--;
	rimuoviDaLista(i,testa);
	close(i);
	FD_CLR(i,master);
}
void connectUser(int i,struct listaClient*p){
	int dim=quantiByte(i);
	char user[dim];
	riceviByte(i,user,dim);
	struct listaClient *target=NULL;
	struct listaClient *sender=NULL;
	while(p!=NULL){
		if(p->socket==i)
			sender=p;
		if(strcmp(p->username,user)==0)
			target=p;
		p=p->next;
	}
	printf("il client %s si vuole connettere a %s\n",sender->username,user);
	if(target==NULL) {
		inviaInt(i,COD_CON_REF);
		return;
	}
	if(	target->stato==OCCUPATO){
		inviaInt(i,COD_CON_REF_OCC);
		return;
	}	
	target->stato=OCCUPATO;
	sender->stato=OCCUPATO;
	strcpy(sender->rival,user);
	strcpy(target->rival,sender->username);
	//invio la codifica di richiesta connect ,username e porta udp di ascolto;
	inviaInt(target->socket,COD_CON_REQ);   //cod conn
	dim=strlen(sender->username)+1;
	inviaByte(target->socket,dim,sender->username); //username
	inviaInt(target->socket,target->porta); //udp
	dim=strlen(sender->ip)+1;
	inviaByte(target->socket,dim,sender->ip); //ip
}
void gameAccepted(int i,struct listaClient * testa){
	struct listaClient * p=testa;
	while(testa!=NULL){
		if(testa->socket==i)
			break;
		testa=testa->next;
	}
	while(p!=NULL){                                 ///posso evitare i due while in sequenza aggiungendo una variabile alla struct
		if(strcmp(p->username,testa->rival)==0)
			break;
		p=p->next;
	}
	printf("il client %s ha accettato la partita con %s\n",testa->username,testa->rival);
	inviaInt(p->socket,COD_CON_ACC);
	inviaInt(p->socket,testa->porta);
	int dim=strlen(testa->ip)+1;
	printf("invio a %s l'ip :%s\n",p->username,p->ip);
	inviaByte(p->socket,dim,testa->ip);
}
void gameRefused(int i,struct listaClient * testa){
	struct listaClient * p=testa;
	while(testa!=NULL){
		if(testa->socket==i)
			break;
		testa=testa->next;
	}
	while(p!=NULL){                                 ///posso evitare i due while in sequenza aggiungendo una variabile alla struct
		if(strcmp(p->username,testa->rival)==0)
			break;
		p=p->next;
	}
	testa->stato=LIBERO;
	p->stato=LIBERO;
	printf("il client %s ha rifiutato la partita con %s\n",testa->username,testa->rival);
	strcpy(p->rival,"");
	strcpy(testa->rival,"");
	inviaInt(p->socket,COD_CON_REFUSED);
}
void disconnect(int i,struct listaClient * testa){
	struct listaClient *p=testa;
	while(p){
		if(p->socket==i)
			break;
		p=p->next;
	}
	while(testa!=NULL){                                 ///posso evitare i due while in sequenza aggiungendo una variabile alla struct
		if(strcmp(testa->username,p->rival)==0)
			break;
		testa=testa->next;
	}
	testa->stato=LIBERO;
	p->stato=LIBERO;
	printf("il client %s si è arreso nella partita con %s\n",p->username,testa->username);
	strcpy(p->rival,"");
	strcpy(testa->rival,"");
	inviaInt(testa->socket,DISCONNECT);
}

void decripta(int cod, int i,struct listaClient ** testa,fd_set*master){
	printf("sto decriptando\n");
	switch(cod){
		case COD_WHO:
			printf("ricevuta una who\n");
			who(i,*testa);
			break;
		case COD_QUIT:
			printf("ricevuta una quit\n");
			quit(i,testa,master);
			break;
		case COD_FAST_QUIT:
			printf("ricevuta una fast_quit\n");
			close(i);
			FD_CLR(i,master);
			break;
		case COD_CON_REQ:
			printf("ricevuta una richiesta di connect\n");
			connectUser(i,*testa);
			break;
		case COD_CON_ACC:
			gameAccepted(i,*testa);
			break;
		case COD_CON_REF:
			gameRefused(i,*testa);
			break;
		case DISCONNECT:
			disconnect(i,*testa);
			break;
		printf("codifica non riconosciuta\n");
		break;
	}
}
int usernamePresente(char*username,struct listaClient*p){
	while(p!=NULL){
		if(p->porta!=0 && strcmp(p->username,username)==0)
			return true;
		p=p->next;
	}
	return false;
}
int main(int argc, char*argv[]){
	if(argc<1){
		printf("Errore passaggio argomenti al server\n");
		exit(1);
	}
	int udp;
	struct listaClient * testa=NULL;
	struct listaClient * nuovoClient=NULL;
	int porta=atoi((const char*)argv[1]);
	if(porta<1025||porta>0xFFFF)
		inserisciPorta(&porta);
	printf(" porta scelta: %d \n",porta);
	int sd=socket(AF_INET,SOCK_STREAM,0);
	if(sd==-1){
		perror("Errore nel creare il socket ");
		exit(1);
	}
	//1 parte multiplexing,sd è il listener
	fd_set master;
	fd_set read;
	int fdmax;
	FD_ZERO(&master);
	FD_ZERO(&read);
////////////////////////////////////////////////

	printf("nuovo socket creato %d\n",sd);
	struct sockaddr_in server,client;
	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(porta);
	server.sin_addr.s_addr=INADDR_ANY;
	int ret,new_sd;

//assegno l'indirizzo al socket sd;
	ret=bind(sd,(struct sockaddr*)&server,sizeof(server));
	if(ret==-1){
		perror("Errore nella bind ");
		exit(1);
	}
	FD_SET(sd,&master);
	fdmax=sd;
	
	ret=listen(sd,QUEUE_LEN);
	if(ret==-1){
		perror("Errore nella listen ");
		exit(1);
	}

	int i;
	int dimMsg;
	socklen_t len=sizeof(client);
	for(;;){
		read=master;
		select(fdmax+1,&read,NULL,NULL,NULL);
		for(i=0;i<=fdmax;i++){
			if(FD_ISSET(i,&read)){
				if(i==sd){
					nuovoClient=(struct listaClient*)malloc(sizeof(struct listaClient));// posso evitare queste 4 righe se dall id del socket riesco a ottenerne l'IP e fare tutto fuori dal listener
					nuovoClient->next=NULL;
					nuovoClient->porta=0;
					len=sizeof(client);
					new_sd=accept(sd,(struct sockaddr*)&client,&len);
					strcpy(nuovoClient->ip,inet_ntoa(client.sin_addr));
					nuovoClient->socket=new_sd;
					strcpy(nuovoClient->rival," ");
					inserimentoLista(&testa,nuovoClient);
					if(new_sd==-1){
						perror("Errore nella accept");
						exit(1);
					}
					printf("Connessione stabilita con il client\n");
					FD_SET(new_sd,&master);
					if(new_sd>fdmax)
						fdmax=new_sd;
				}else{
					printf("numero di client %d\n",n_client);
					int cod=quantiByte(i);
					printf("codifica: %d\n",cod);
					if(cod==COD_INFO){
							n_client=n_client+1;
							udp=quantiByte(i);
							dimMsg=quantiByte(i);
							char c[dimMsg];
							riceviByte(i,c,dimMsg);
							if(usernamePresente(c,testa)==false){
								printf("username non presente\n");
								inviaInt(i,OK);
								inserimentoDatiLista(i,c,udp,testa);
								stampaLista(testa);
								break;
							}
							inviaInt(i,ERRORE);            
					}else{
						decripta(cod,i,&testa,&master);
					}
				}
			}

	
		}
	}
	return 0;
}


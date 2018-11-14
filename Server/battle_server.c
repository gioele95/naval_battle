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

#define SERVER
#include "../costants.h"


//DISCONNETTITI CON IL I


int n_client=0;// variabile usata per stimare la dimensione del buffer per lawho
fd_set master;
enum clientStatus{FREE,OCCUPIED};


struct clientList{
	int socket;
	struct clientList * next; 
	int port; 
	char username[USERNAME_LEN];
	char ip[IP_LEN];
	enum clientStatus status;
	char rival[USERNAME_LEN];
};
struct clientList * testa;
void sendInt(int sd, int msgl){
	int ret;
	int msg=htonl(msgl);
	printf("integer sent %d\n",ntohl(msg ));
	ret=send(sd,(void*)&msg,sizeof(int),0);
	printf("%d\n",ret );
	if(ret==-1){
		perror("Error in sending the integer to the client");
		//exit(1);
	}
}
int removeFromList(int i){
	int app;
	char *c=NULL;
	char user[USERNAME_LEN];
	struct clientList * p=testa;
	struct clientList * prev=NULL;
	while(p!=NULL){
		if(i==p->socket){
			app=p->port;
			if(app!=0){
				printf("removing the user: %s\n",p->username );
				if(p->status==OCCUPIED){
					strcpy(user,p->rival);
					c=p->rival;
				}
			}
			if (prev==0)
				testa=p->next;
			else{
				prev->next=p->next;
			}
			free(p);
		}
		prev=p;
		p=p->next;
	}
	if(c!=NULL){
		p=testa;
		while(p!=NULL){
			if(p->port!=0 && strcmp(user,p->username)==0){
				p->status=FREE;
				break;
			}
			p=p->next;
		}
	}
	return app;
}
void quit(int i){
	int a=removeFromList(i);
	if(a!=0)
		n_client--;
	close(i);
	FD_CLR(i,&master);
}
void checkReceive(int ret,int i){
	if(ret==-1){
		perror("receive Error");
		//	exit(1);
	}
	if(ret==0){
		printf("Client disconnected \n");
		quit(i);
		//		exit(0);
	}	
}

void printStatus(enum clientStatus s){
	if(s==FREE)
		printf("free)\n");
	else
		printf("occupied)\n");
}
int howManyBytes(int i){
	int ret;
	uint32_t dimMsg;
	ret=recv(i,(void*)&dimMsg,sizeof(int),0);
	checkReceive(ret,i);
	int dimMsg2=(int)ntohl(dimMsg);
	//printf("byte ricevuti: %d \n",ret);
	//printf("il client vuole mandare %d byte\n",dimMsg2 );
	return dimMsg2;
}
void receiveBytes(int i, void*buf,int dimMsg){
	int ret;
	ret=recv(i,(void*)buf,dimMsg,0);
	checkReceive(ret,i);
	printf("bytes received: %d \n",ret);
}
void listInsertion(struct clientList *p){
	p->next=testa;
	testa=p;
}
void insertDataInList(int i,char*buf,int udp, struct clientList *p){
	//effettua un controllo fai inserimentoLista in coda;
	while(1 ){
		if(p->socket==i)
			break;
		p=p->next;
	}
	p->status=FREE;
	strcpy(p->username,buf);
	p->port=udp;
}
void printList(){
	struct clientList * p=testa;
	while(p!=NULL){
		if(p->port!=0){
			printf("%s(",p->username );
			printStatus(p->status);
			printf("port: %d\n",p->port );
			printf("ip: %s\n\n",p->ip );
		}	
		p=p->next;
	}
}
void insertPort(int * portAscolto){
	printf("Listening port passed to the main invalid ");
	while(1){
		printf("Inserist the UDP listening port: ");
		scanf("%d",portAscolto);
		if(*portAscolto<1025||*portAscolto>0xFFFF){
			printf("Invalid port, RETRY\n");
			continue;
		}
		break;
	}
}
int statusLength(enum clientStatus s){
	if(s==FREE)
		return 6;
	return 8;
}
void sendByte(int sd, int dim, void * msg){
	int ret;
	printf("want to send to the client:%d bytes\n",sd );
	sendInt(sd,dim);
	printf("sending to the client %s\n",(char*)msg);
	ret=send(sd,msg,dim,0);
	if(ret==-1){
		perror("Error sending info to the client");
		exit(1);
	}
}
void who(int i){//faccio una stima di 20 char per client
	struct clientList * p=testa;
	int dim=n_client*20;
	char*app;
	char *buf=(char*)malloc(dim*sizeof(char));
	int d=0,ii=1;
	while(p!=NULL){
		if(p->port!=0){
			d=strlen(p->username)+2+statusLength(p->status);
			dim-=d;
			if(dim>0){
				strcat(buf,p->username);
				if(p->status==FREE)
					strcat(buf,"(free)");
				else
					strcat(buf,"(occupied)");
				strcat(buf,"\n");
			}else{
				dim=n_client*(20+5*ii);
				app=(char*)malloc(dim*sizeof(char));
				strcpy(app,buf);
				dim-=strlen(buf);
				free((void*)buf);
				buf=app;
				ii++;
				continue;	
			}
		}
		p=p->next;

	}
	printf("sending the who of: %s\n to",buf );
	dim=strlen(buf)+1;
	printf("sending to the client COD_WHO: %d\n",COD_WHO);
	sendInt(i,COD_WHO);
	sendByte(i,dim,buf);
	free(buf);
}
void connectUser(int i){
	struct clientList*p=testa;
	int dim=howManyBytes(i);
	char user[dim];
	receiveBytes(i,user,dim);
	struct clientList *target=NULL;
	struct clientList *sender=NULL;
	while(p!=NULL){
		if(p->socket==i)
			sender=p;
		if(strcmp(p->username,user)==0)
			target=p;
		p=p->next;
	}
	printf("the client %s wants to connect with %s\n",sender->username,user);
	if(target==NULL) {
		sendInt(i,COD_CON_REF);
		return;
	}
	if(	target->status==OCCUPIED){
		sendInt(i,COD_CON_REF_OCC);
		return;
	}	
	target->status=OCCUPIED;
	sender->status=OCCUPIED;
	strcpy(sender->rival,user);
	strcpy(target->rival,sender->username);
	//invio la codifica di richiesta connect ,username e port udp di ascolto;
	sendInt(target->socket,COD_CON_REQ);   //cod conn
	dim=strlen(sender->username)+1;
	sendByte(target->socket,dim,sender->username); //username
}
void gameAccepted(int i){
	struct clientList * head=testa;
	struct clientList * p=testa;
	while(head!=NULL){
		if(head->socket==i)
			break;
		head=head->next;
	}
	while(p!=NULL){                                 ///posso evitare i due while in sequenza aggiungendo una variabile alla struct
		if(strcmp(p->username,head->rival)==0)
			break;
		p=p->next;
	}
	sendInt(i,CONN_INFO);
	sendInt(i,p->port); //udp
	int dim=strlen(p->ip)+1;
	sendByte(i,dim,p->ip); //ip
	printf("client %s  accepted the game with %s\n",head->username,head->rival);
	sendInt(p->socket,COD_CON_ACC);
	sendInt(p->socket,head->port);
	dim=strlen(head->ip)+1;
	printf("sending to %s  the ip :%s\n",p->username,p->ip);
	sendByte(p->socket,dim,head->ip);
}
void gameRefused(int i){
	struct clientList * head=testa;
	struct clientList * p=testa;
	while(head!=NULL){
		if(head->socket==i)
			break;
		head=head->next;
	}
	while(p!=NULL){                                 ///posso evitare i due while in sequenza aggiungendo una variabile alla struct
		if(strcmp(p->username,head->rival)==0)
			break;
		p=p->next;
	}
	head->status=FREE;
	p->status=FREE;
	printf("client %s refused the game with %s\n",head->username,head->rival);
	strcpy(p->rival,"");
	strcpy(head->rival,"");
	sendInt(p->socket,COD_CON_REFUSED);
}
void disconnect(int i){
	struct clientList *p=testa;
	struct clientList * head=p;
	while(p!=NULL){
		if(p->socket==i)
			break;
		p=p->next;
	}
	if(p->status==FREE)
		return;
	while(head!=NULL){                                 ///posso evitare i due while in sequenza aggiungendo una variabile alla struct
		if(strcmp(head->username,p->rival)==0)
			break;
		head=head->next;
	}
	
	head->status=FREE;
	p->status=FREE;
	printf("client %s gave up in the game with %s\n",p->username,head->username);
	strcpy(p->rival,"");
	strcpy(head->rival,"");
	printf("sending to %s the disconnect\n",head->username );
	sendInt(head->socket,DISCONNECT);
}

void decrypt(int cod, int i,struct clientList ** testa,fd_set*master){
	printf("decrypting\n");
	switch(cod){
		case COD_WHO:
			printf("received a who\n");
			who(i);
			break;
		case COD_QUIT:
			printf("received a quit\n");
			quit(i);
			break;
		case COD_CON_REQ:
			printf("received a conection request\n");
			connectUser(i);
			break;
		case COD_CON_ACC:
			gameAccepted(i);
			break;
		case COD_CON_REF:
			gameRefused(i);
			break;
		case DISCONNECT:
			disconnect(i);
			break;
		printf("coding not recognised\n");
		break;
	}
}
int usernamePresent(char*username,struct clientList*p){
	while(p!=NULL){
		if(p->port!=0 && strcmp(p->username,username)==0)
			return true;
		p=p->next;
	}
	return false;
}
int main(int argc, char*argv[]){
	if(argc!=2){
		printf("Error in passing the arguments to the server\n");
		exit(1);
	}
	int udp;
	testa=NULL;
	struct clientList * nuovoClient=NULL;
	int port=atoi((const char*)argv[1]);
	if(port<1025||port>0xFFFF)
		insertPort(&port);
	printf("chosen port: %d \n",port);
	int sd=socket(AF_INET,SOCK_STREAM,0);
	if(sd==-1){
		perror("Errore in creating the socket");
		exit(1);
	}
	//1 multiplexing part,sd is the listener
	fd_set read;
	int fdmax;
	FD_ZERO(&master);
	FD_ZERO(&read);
	////////////////////////////////////////////////

	printf("new socket created %d\n",sd);
	struct sockaddr_in server,client;
	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(port);
	server.sin_addr.s_addr=INADDR_ANY;
	int ret,new_sd;

	//assigning the address to socket sd
	ret=bind(sd,(struct sockaddr*)&server,sizeof(server));
	if(ret==-1){
		perror("bind error ");
		exit(1);
	}
	FD_SET(sd,&master);
	fdmax=sd;
	
	ret=listen(sd,QUEUE_LEN);
	if(ret==-1){
		perror("Error in the listen ");
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
					nuovoClient=(struct clientList*)malloc(sizeof(struct clientList));// posso evitare queste 4 righe se dall id del socket riesco a ottenerne l'IP e fare tutto fuori dal listener
					nuovoClient->next=NULL;
					nuovoClient->port=0;
					len=sizeof(client);
					new_sd=accept(sd,(struct sockaddr*)&client,&len);
					strcpy(nuovoClient->ip,inet_ntoa(client.sin_addr));
					nuovoClient->socket=new_sd;
					strcpy(nuovoClient->rival," ");
					listInsertion(nuovoClient);
					if(new_sd==-1){
						perror("Errore in the accept");
						exit(1);
					}
					printf("Connection established with client\n");
					FD_SET(new_sd,&master);
					if(new_sd>fdmax)
						fdmax=new_sd;
				}else{
					printf("number of clients %d\n",n_client);
					int cod=howManyBytes(i);
					printf("coding: %d\n",cod);
					if(cod==COD_INFO){
							n_client=n_client+1;
							udp=howManyBytes(i);
							dimMsg=howManyBytes(i);
							char c[dimMsg];
							receiveBytes(i,c,dimMsg);
							if(usernamePresent(c,testa)==false){
								printf("username not present\n");
								sendInt(i,OK);
								insertDataInList(i,c,udp,testa);
								printList();
								break;
							}
							sendInt(i,ERRORE);            
					}else{
						decrypt(cod,i,&testa,&master);
					}
				}
			}

	
		}
	}
	return 0;
}


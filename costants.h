#ifdef SERVER
	//server constants
	#define INT_DIM 2
	#define ERRORE 6
	#define QUEUE_LEN 10
#else
	//client constants
	#define SHOT_DIM 3
	#define YOU_WON 14
	#define COD_SHOT 15
	#define COD_HIT 16
	#define N_SHIPS 2
	#define SQUARE_DIM 3
	#define TIMEOUT 20
	#define TIMEOUT_SHIPS 30
	#define TABLE_SIZE 36
#endif
//common constants


#define COD_WHO 1
#define COD_INFO 2
#define OK 3
#define COD_QUIT 5
#define COD_FAST_QUIT 7
#define COD_CON_REQ 8
#define COD_CON_REF 9
#define COD_CON_ACC 10
#define COD_CON_REF_OCC 11
#define COD_CON_REFUSED 12
#define DISCONNECT 13
#define true 1
#define false 0
#define USERNAME_LEN 30
#define CONN_INFO 17	
#define IP_LEN 16
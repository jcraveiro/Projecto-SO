/* 
 * -- simplehttpd.c --
 * A (very) simple HTTP server
 *
 * Sistemas Operativos 2014/2015
 */
#ifndef SIMPLEHTPD_HEADER
#define SIMPLEHTPD_HEADER

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

// Produce debug information
#define DEBUG	  	1	
//------------

// Header of HTTP reply to client 
#define	SERVER_STRING 	"Server: simpleserver/0.1.0\r\n"
#define HEADER_1	"HTTP/1.0 200 OK\r\n"
#define HEADER_2	"Content-Type: text/html\r\n\r\n"

#define GET_EXPR	"GET /"
#define CGI_EXPR	"cgi-bin/"
#define SIZE_BUF	1024
//------------

#define LINE_MAX 1024

typedef struct lnode* config_struct_ptr;
typedef struct lnode{
    int port;
    int n_threads;
    int scheduling;
    int n_scripts;
    char **authorized_scripts;
} config_struct;

typedef struct onode* request_ptr;
typedef struct onode {
    int socket;
    char* request_string;
} request;

typedef struct inode* selected_request_ptr;
typedef struct inode {
	request_ptr request;
	int type;
} selected_request;

typedef struct node* stats_struct_ptr;
typedef struct node{
    int type;
    char* request;
    int thread_number;
    char time[2][30];
    int denied_requests;
}stats_struct;

//simplehttpd.c
void start();
void initVariables();
void terminateVariables();
void create_request_buffer();
void create_threads();
void* thread_worker_behaviour(void* arg);
void* thread_scheduler_behaviour(void* arg);
void terminate_threads();
void start_server();
//TODO stop server

//stats.c
void stats_main(char* init_server_time);
void write_to_file();

//config.c
void readConfig (char* file );
void initConfig ();
int strToInt (char what[]);
void addStrToConfig(char what[]);
char** reallocShm(int* old_shmid, size_t new_size);
void config_main();

//socket_handling.c
int  fireup(int port);
void identify(int socket);
int  read_line(int socket, int n);

//request_handling.c
void get_request(int socket);
void send_header(int socket);
void send_page(int socket);
void execute_script(int socket);

//signal_handling.c
void catch_ctrlc(int);

//requestBuffer_handling.c
void addRequestToBuffer(int socket, char* request);
request_ptr getRequestFromBuffer(int index);
void removeRequestFromBuffer(int index);

//error_handling.c
void not_found(int socket);
void cannot_execute(int socket);
void cannot_process(int socket);

//utils.c
char* get_system_date();

//global variables
int port,socket_conn,new_conn;
char buf[SIZE_BUF];
char req_buf[SIZE_BUF];
char buf_tmp[SIZE_BUF];
int denied_requests = 0;

//----Configs----
sem_t *mutex_config;		//mutex para aceder as configs
int shm_config_id;			//id da memoria partilhada para armazenar as configs
config_struct_ptr shm_config;	//memoria partilhada para armazenar as configs

//----Estatisticas----
pthread_mutex_t mutex_estatisticas = PTHREAD_MUTEX_INITIALIZER; //mutex para as estatisticas
int fd_stats[2];

//----request buffer----
pthread_mutex_t mutex_request_buffer = PTHREAD_MUTEX_INITIALIZER; //mutex para o request buffer

int* mutex_thread_array;

pthread_t* thread_array;
pthread_t thread_scheduler;
struct {
    request_ptr requests;
    int free_pos;
} request_buffer;

selected_request_ptr temp_thread_buffer;

#endif

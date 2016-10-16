#include "simplehttpd.h"

int main(int argc, char* argv[]) {

    //port is setup, we can run things now
    start();

    return 0;
}

void start() {

    int i;

    initVariables();
    for (i = 0; i < 2; i++) {
        if (fork() == 0) {
            if (i == 0) {
                //run configs
            } else {
                //run statistics
            }
            exit(0);
        }
    }

    sem_wait(mutex_config);
    port = shm_config->port;
    create_request_buffer();
    create_threads();
    start_server();
}

void initVariables() {

    sem_unlink("MUTEX_CONFIG");
    mutex_config = sem_open("MUTEX_CONFIG",O_CREAT|O_EXCL,0700,0);

    shm_config_id = shmget(IPC_PRIVATE, sizeof(config_struct), IPC_CREAT|0700);
    shm_config = (config_struct_ptr)shmat(shm_config_id, NULL, 0);

    //init pipe
    pipe(fd_stats);
}

void terminateVariables() {

    //process mutexes
    sem_close(mutex_config);
    sem_unlink("MUTEX_CONFIG");

    //thread mutexes
    pthread_mutex_destroy(&mutex_estatisticas);
    pthread_mutex_destroy(&mutex_request_buffer);

    //shared memories
    shmctl(shm_config_id, IPC_RMID, NULL);

    //pipe
    close(fd_stats[0]);
    close(fd_stats[1]);
}

void create_request_buffer() {

    request_buffer.requests = (request_ptr)malloc((shm_config->n_threads)*2*sizeof(request));
    request_buffer.free_pos = shm_config->n_threads * 2;
    temp_thread_buffer = (selected_request_ptr)malloc((shm_config->n_threads)*sizeof(request));

}

//init thread array and all thread mutexes
void create_threads() {

    int i;

    sem_wait(mutex_config);
    mutex_thread_array = (int*)malloc((shm_config->n_threads)*sizeof(int));
    for(i = 0; i < shm_config->n_threads; i++) {
        pthread_create(&thread_array[i], NULL, thread_worker_behaviour, (void*)((long)i));
        mutex_thread_array[i] = 0;
    }
    pthread_create(&thread_scheduler, NULL, thread_scheduler_behaviour, NULL);
    sem_post(mutex_config);
}

void* thread_worker_behaviour(void* arg) {

    long my_index;
    request_ptr my_request;
    stats_struct_ptr stats_write;

    my_index = (long)arg;
    close(fd_stats[0]);
    stats_write = (stats_struct_ptr) malloc(sizeof(stats_struct));

    while(1) {
        if(mutex_thread_array[my_index] != 0) {
            my_request = temp_thread_buffer[my_index].request;

            stats_write->type = temp_thread_buffer[my_index].type;
            stats_write->request = my_request->request_string;
            stats_write->thread_number = my_index;
            strcpy(stats_write->time[0], get_system_date());
            stats_write->denied_requests = denied_requests;

            if (temp_thread_buffer[my_index].type == 0) {
                execute_script(my_request->socket);
            } else {
                send_page(my_request->socket);
            }
            strcpy(stats_write->time[1], get_system_date());
            close(my_request->socket);

            write(fd_stats[1], stats_write, sizeof(stats_struct));

            free(my_request->request_string);
            free(my_request);
            mutex_thread_array[my_index] = 0;
        }
    }

    free(stats_write);
    pthread_exit(NULL);
    return NULL;
}

void* thread_scheduler_behaviour(void* arg) {

    int selected_index = 0;  //TODO not this value refer to todo below
    int search_thread_index;
    int is_searching;
    request_ptr selected_request;

    while(1) {
        //TODO select request's index

        selected_request = getRequestFromBuffer(selected_index);
        removeRequestFromBuffer(selected_index);

        search_thread_index = 0;
        is_searching = 1;
        while(is_searching == 1) {
            if (mutex_thread_array[search_thread_index] == 0) {
                is_searching = 0;

                temp_thread_buffer[search_thread_index].request = selected_request;

                //checks type of request
                if (!strncmp(req_buf,CGI_EXPR,strlen(CGI_EXPR))) {
                    temp_thread_buffer[search_thread_index].type = 0;
                } else {
                    temp_thread_buffer[search_thread_index].type = 1;
                }

                mutex_thread_array[search_thread_index] = 1;
            }

            search_thread_index++;
            sem_wait(mutex_config);
            if (search_thread_index > shm_config->n_threads - 1) {
                search_thread_index = 0;
            }
            sem_post(mutex_config);
        }
    }

    
    pthread_exit(NULL);
    return NULL;
}

void terminate_threads() {

    int i;

    for (i = 0; i < shm_config->n_threads; i++) {
        pthread_join(thread_array[i], NULL);
    }
}

void start_server()
{
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    //signal(SIGINT,catch_ctrlc);

    printf("Listening for HTTP requests on port %d\n",port);

    // Configure listening port
    if ((socket_conn=fireup(port))==-1)
        exit(1);

    // Serve requests
    while (1)
    {
        // Accept connection on socket
        if ( (new_conn = accept(socket_conn,(struct sockaddr *)&client_name,&client_name_len)) == -1 ) {
            printf("Error accepting connection\n");
            exit(1);
        }

        // Identify new client
        identify(new_conn);

        // Process request
        get_request(new_conn);

        addRequestToBuffer(new_conn, req_buf);

    }

}

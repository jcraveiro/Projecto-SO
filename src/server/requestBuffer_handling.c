#include "simplehttpd.h"

void addRequestToBuffer(int socket, char* request) {

    if (request_buffer.free_pos > 0) {
        pthread_mutex_lock(&mutex_request_buffer);
        request_buffer.requests[2*shm_config->n_threads - request_buffer.free_pos].socket = socket;
        if (request_buffer.requests[2*shm_config->n_threads - request_buffer.free_pos].request_string != NULL) {
            free(request_buffer.requests[2*shm_config->n_threads - request_buffer.free_pos].request_string);
        }
        request_buffer.requests[2*shm_config->n_threads - request_buffer.free_pos].request_string = (char*)malloc((strlen(request)+1)*sizeof(char));
        strcpy(request_buffer.requests[2*shm_config->n_threads - request_buffer.free_pos].request_string, request);
        request_buffer.free_pos -= 1;
        pthread_mutex_unlock(&mutex_request_buffer);
    } else {
        denied_requests++;
        cannot_process(socket);
        close(socket);
    }
}

request_ptr getRequestFromBuffer(int index) {

    request_ptr choosen_request = (request_ptr)malloc(sizeof(request));

    choosen_request->socket = request_buffer.requests[index].socket;
    choosen_request->request_string = (char*)malloc((strlen(request_buffer.requests[index].request_string)+1)*sizeof(char));
    strcpy(choosen_request->request_string, request_buffer.requests[index].request_string);

    return choosen_request;
}

void removeRequestFromBuffer(int index) {

    int i, i_max;

    pthread_mutex_lock(&mutex_request_buffer);
    
    //adjust buffer to the right, from the index where the request was choosen
    i_max = 2*shm_config->n_threads-1;
    for (i = index; i < i_max; i++) {
        request_buffer.requests[i].socket = request_buffer.requests[i+1].socket;
        if (request_buffer.requests[i].request_string != NULL) {
            free(request_buffer.requests[i].request_string);
        }
        request_buffer.requests[i].request_string = request_buffer.requests[i+1].request_string; 
    }

    pthread_mutex_unlock(&mutex_request_buffer);
    request_buffer.free_pos += 1;
}

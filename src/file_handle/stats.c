#include "simplehttpd.h"

char* file = "log.txt";
stats_struct_ptr stats_read;

struct {
    char server_times [2][30];
    int n_static_requests;
    int n_dinamic_requests;
    int n_denied_requests;
} gather_stats;

void write_to_file() {

    FILE* log_fp;
    char separator = ',';
    char group_separator = '\n';
    close(fd_stats[1]);

    if( access( file, F_OK ) != -1 ) {
        // file exists
        if ((log_fp = fopen(file, "a")) == NULL) {
            printf("Ocorreu um erro na abertura do ficheiro de estatísticas\n");
            exit(-1);
        }

    } else {
        // file doesn't exist
        if ((log_fp = fopen(file, "w")) == NULL) {
            printf("Ocorreu um erro na abertura do ficheiro de estatísticas\n");
            exit(-1);
        }
        
    }
    
    fprintf(log_fp,"%d%c%s%c%d%c%s%c%s%c", stats_read->type,
            separator,
            stats_read->request,
            separator,
            stats_read->thread_number,
            separator,
            stats_read->time[0],
            separator,
            stats_read->time[1],
            group_separator);
}

void stats_main(char* init_server_time) {

    stats_read = (stats_struct_ptr) malloc(sizeof(stats_struct));

    gather_stats.n_static_requests = 0;
    gather_stats.n_dinamic_requests = 0;
    gather_stats.n_denied_requests = 0;
    strcpy(gather_stats.server_times[0], init_server_time);
    
    while(1) {
        read(fd_stats[0], stats_read, sizeof(stats_struct));

        gather_stats.n_dinamic_requests += 1-stats_read->type;
        gather_stats.n_static_requests += stats_read->type;
        if (stats_read->denied_requests > gather_stats.n_denied_requests) {
            gather_stats.n_denied_requests = stats_read->denied_requests;
        }
        
        write_to_file();
    }
}

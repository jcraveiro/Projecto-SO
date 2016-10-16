#include "simplehttpd.h"

char file[18] = "/../../config.txt";
int authorized_scripts_shmid;
int* individual_script_shmid;

void initConfig () {
    shm_config->port = 8080;
    shm_config->n_threads = 10;
    shm_config->scheduling = 0; //0 == FIFO 
    shm_config->n_scripts = 0;
    shm_config->authorized_scripts = NULL;
}

void readConfig (char* file ) {
    FILE* fp;
    char temp_array[LINE_MAX];
    int counter = 0;
    
    if ((fp = fopen(file, "r")) == NULL) {
        printf("Ocorreu um erro na abertura do ficheiro.\n");
        exit(0);
    }

    bzero(temp_array, LINE_MAX * sizeof(char));

    while (fgets(temp_array, LINE_MAX, fp) != NULL) {
        
        switch (counter) {
        case 0:
            if ((shm_config->port = strToInt(temp_array)) == -1) {
                return;
            }
            break;
        case 1:
            if ((shm_config->n_threads = strToInt(temp_array)) == -1) {
                return;
            }
            break;
        case 2:
            if ((shm_config->scheduling = strToInt(temp_array)) == -1) {
                return;
            }
            break;
        default:
            addStrToConfig(temp_array);
            break;   
            
        }
        counter++;
        bzero(temp_array, LINE_MAX * sizeof(char));
    }
}

int strToInt (char what[]) {
    int result = 0;
    int i;
    
    for (i = 0; what[i] != '\0'; i++) {
        if (what[i] < '0' || what[i] > '9') {
            return -1;
        }
        result = result * 10 + ( what[i] - '0' );
    }

    return result;
}

void addStrToConfig(char what[]) {
    int length = (int) strlen(what);

    if (length == LINE_MAX)
    {
        return ;
    }

    if (shm_config->n_scripts == 0) {
        authorized_scripts_shmid = shmget(IPC_PRIVATE, (shm_config->n_scripts+1)*sizeof(char*), IPC_CREAT|0700);
        shm_config->authorized_scripts = (char**)shmat(authorized_scripts_shmid, NULL, 0);
        
        individual_script_shmid =  (int*) malloc(sizeof(int));
        individual_script_shmid[0] = shmget(IPC_PRIVATE, (length+1)*sizeof(char), IPC_CREAT|0700);
        shm_config->authorized_scripts[0] = (char*)shmat(individual_script_shmid[0], NULL, 0);
        
        strcpy(shm_config->authorized_scripts[0], what);
    } else {

        
        shm_config->authorized_scripts = reallocShm(&authorized_scripts_shmid, (shm_config->n_scripts + 1) * sizeof(char*));
        
        individual_script_shmid = (int*) realloc(individual_script_shmid, (shm_config->n_scripts + 1) * sizeof(int));
        individual_script_shmid[shm_config->n_scripts] = shmget(IPC_PRIVATE, (length+1)*sizeof(char), IPC_CREAT|0700);
            
        shm_config->authorized_scripts[shm_config->n_scripts] = (char*)shmat(individual_script_shmid[shm_config->n_scripts], NULL, 0);
        strcpy(shm_config->authorized_scripts[shm_config->n_scripts], what);
    }
    shm_config->n_scripts += 1;
}

char** reallocShm(int* old_shmid, size_t new_size) {
    char** old_shm;
    char** new_shm;
    int new_shmid;
    
    old_shm = shmat(*old_shmid, NULL, 0);
    new_shmid = shmget(IPC_PRIVATE, new_size, IPC_CREAT|0700);
    new_shm = shmat(new_shmid, NULL, 0);

    memcpy(new_shm, old_shm, (new_size - sizeof(char*)));

    shmctl(*old_shmid, IPC_RMID, NULL);

    *old_shmid = new_shmid;

    return new_shm;
}

void config_main() {

    initConfig();
    readConfig(file);
    sem_post(mutex_config);
    
    while(1) {

        //TODO only runs when SIGHUP is receivedgit commit -am 
        sem_wait(mutex_config);
        readConfig(file);
        sem_post(mutex_config);
    }

}

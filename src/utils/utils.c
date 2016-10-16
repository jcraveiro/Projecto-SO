#include "simplehttpd.h"

char* get_system_date() {

    time_t rawtime;
    struct tm * timeinfo;
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    return asctime(timeinfo);
}

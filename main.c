#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>

#include "kbmk.h"



int main(int argc, char **argv){
    FILE *fp;
    if(argc <= 1){
        printf("%s", help_string);
    }
    else{
        printf("%s\n", argv[1]);
        fp = fopen(argv[1], "r");
    }
    parse_file(fp);
    kbmk_handle_kb_events();
    kbmk_keyboards_free();
    return 0;
}




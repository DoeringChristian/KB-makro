#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "kbmk.h"

int main(int argc, char **argv){
    if(argc < 2)
      return 0;
    
    printf("test\n");
    int fd = open(argv[1], O_RDONLY);

    struct kbmk_keyboard_list *list = kbmk_parse(fd);
    close(fd);


    kbmk_main(list);

    kbmk_keyboard_list_free(list);
    return 0;
}

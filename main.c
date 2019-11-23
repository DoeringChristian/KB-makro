#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define MODE_HELP 0x00
#define MODE_LIST 0x01
#define MODE_EVENT 0x02
#define MODE_SCRIPT 0x04

#define EVENT_DIRECTORY "/dev/input/"

#define ADDRESS_STRING_MAX 65536
#define COMMAND_STRING_MAX 65536

typedef struct command{
    uint8_t address;
    char to_execute[COMMAND_STRING_MAX];
} command_t;

uint8_t v_kb[65536];
uint8_t mode;
char *script_file;
char *event_name;
const char help_string[] = "Usage:\n  -f[script_file]\n   -e[event_name]\n    | -l\n    -e[event_name]\n";

int getLineNumber(char *file_name){
    FILE *fp = fopen(file_name, "r");
    if(fp == NULL){
        fclose(fp);
        return -1;
    }
    int i = 0;
    for(char c = getc(fp); c != EOF;c = getc(fp)){
        if(c == '\n')
            i++;
    }
    fclose(fp);
    return i;
}

int main(int argc, char *argv[]){
    mode = MODE_HELP;
    if(argc > 1){
        for(uint8_t i = 1;i < argc;i++){
            if(argv[i][0] == '-'){
                char *argument = &(argv[i][2]);
                switch(argv[i][1]){
                    case 'l':
                        mode |= MODE_LIST;
                        break;
                    case 'e':
                        event_name = argument;
                        mode |= MODE_EVENT;
                        break;
                    case 'f':
                        script_file = argument;
                        mode |= MODE_SCRIPT;
                        break;
                }
            }
        }
    }

    //printf("mode: %x\n", mode);

    if(mode == MODE_HELP){
        printf("%s", help_string);
    }
    else if(mode == (MODE_LIST | MODE_EVENT)){
        //list
        struct input_event event;
        int input_read;
        int flag, ret;
        //concat path
        char event_file[strlen(EVENT_DIRECTORY) + strlen(event_name)];
        strcpy(event_file, EVENT_DIRECTORY);
        strcat(event_file, event_name);
        printf("%s\n", event_file);

        int input_fd = open(event_file, O_RDONLY);

        if(input_fd == -1){
            printf("error device not found\n");
            return -1;
        }
        
        //ret = ioctl(input_fd, EVIOCGRAB, &flag);
        while(1){
            input_read = read(input_fd, &event, sizeof(event));
            if(input_read == sizeof(event) && event.type == EV_KEY){
                printf("    code: %x, type: %x, time: %x , value: %x\n", event.code, event.type, event.time.tv_usec, event.value);
                v_kb[event.code] = event.value;
            }
        }
    }
    else if(mode == (MODE_EVENT | MODE_SCRIPT)){
        printf("test\n");
        //run
        FILE *script_fp = fopen(script_file, "r");
        if(script_fp == NULL){
            printf("error opening script");
        }

        int commands_length = getLineNumber(script_file); 
        if(commands_length == -1){
            printf("error_script");
            return -1;
        }
        command_t commands[commands_length];
        char line[ADDRESS_STRING_MAX+ COMMAND_STRING_MAX];
        for(int i = 0;i < commands_length;i++){
            commands[i].address = 0;
            fgets(line, 8 + COMMAND_STRING_MAX, script_fp);
            printf("%i\n", commands_length);
            sscanf(line, "%4x", &(commands[i].address));

            strcpy(commands[i].to_execute, strchr(line, ' ')+1);
            printf("%s\n", commands[i].to_execute);
        }
        

        struct input_event event;
        if(strlen(argv[1]) > 50){
            printf("event file to long\n");
            return -1;
        }
        //concat paths
        char event_file[strlen(EVENT_DIRECTORY) + strlen(event_name)];
        strcpy(event_file, EVENT_DIRECTORY);
        strcat(event_file, event_name);
        //printf("%s\n", event_file);

        int input_fd = open(event_file, O_RDONLY);
        if(input_fd == -1){
            printf("error device not found\n");
            return -1;
        }

        int flag, ret;
        ret = ioctl(input_fd, EVIOCGRAB, &flag);
        if(ret == -1){
            printf("error grabing device\n");
            return -1;
        }
        int input_read;
        while(1){
            input_read = read(input_fd, &event, sizeof(event));
            if(input_read == sizeof(event) && event.type == EV_KEY){
                //printf("code: %x, type: %x, time: %x , value: %x\n", event.code, event.type, event.time.tv_usec, event.value);
                v_kb[event.code] = event.value;
                for(int i = 0;i < commands_length;i++){
                    if(v_kb[commands[i].address] == EV_KEY){
                        system(commands[i].to_execute);
                    }
                }
            }
        }
    }
    else{
        printf("%s", help_string);
    }


    
    return 0;
}
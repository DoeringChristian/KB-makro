#include "kbmk.h"

//const char help_string[] = "kb_makro [-l][-m file_path][event]\n";
const char help_string[] = "kb_makro [file_path]";
const char alph_digit[] = "0123456789";
const char file_devices[] = "/proc/bus/input/devices";
const char file_events[] = "/dev/input/";
uint keyboard_count = 0;



kbmk_error_t parse_file(FILE *fp){
    kbmk_error_t err;

    if(fp == NULL){
        err.file = 1;
        return err;
    }
    for(int i = 0;i < MAX_KEYBOARDS;i++){
        for(int j = 0;j < MAX_EVENTS;j++){
            kbmk_keyboards[i].command[j] = NULL;
        }
    }

    uint8_t state = PARSER_STATE_BEGINING;
    uint8_t prev_state = PARSER_STATE_BEGINING;
    int c = 0;
    char buffer[1024];
    uint16_t buffer_count = 0;
    do{
        c = fgetc(fp);
        switch(state){
            case PARSER_STATE_BEGINING:
                if(c == '-'){
                    state = PARSER_STATE_KBID;
                }
                else{
                    state = PARSER_STATE_ERROR;
                }
                break;
            case PARSER_STATE_NEWLINE:
                if(c == '-'){
                    state = PARSER_STATE_KBID;
                }
                else if(contains_const(c, alph_digit)){
                    buffer[buffer_count] = c;
                    buffer_count++;

                    state = PARSER_STATE_ADDRESS;
                }
                else{
                    state = PARSER_STATE_ERROR;
                }
                break;
            case PARSER_STATE_KBID:
                if(c != '\n'){
                    buffer[buffer_count] = c;
                    buffer_count++;
                    prev_state = PARSER_STATE_KBID;

                    state = PARSER_STATE_KBID;
                }
                else if(c == '\n'){
                    buffer[buffer_count] = 0;
                    buffer_count++;
                    if(keyboard_count < MAX_KEYBOARDS){
                        kbmk_keyboards[keyboard_count].event_path = malloc(strlen(file_events)+strlen(buffer)+1);
                        strcpy(kbmk_keyboards[keyboard_count].event_path, file_events);
                        strcat(kbmk_keyboards[keyboard_count].event_path, buffer);
                        keyboard_count++;
                    }
                    buffer_count = 0;

                    state = PARSER_STATE_NEWLINE;
                }
                else{
                    state = PARSER_STATE_ERROR;
                }
                break;
            case PARSER_STATE_ADDRESS:
                if(contains_const(c, alph_digit)){
                    buffer[buffer_count] = c;
                    buffer_count++;

                    state = PARSER_STATE_ADDRESS;
                }
                else if(c == ' '){
                    buffer[buffer_count] = c;
                    buffer_count++;

                    state = PARSER_STATE_COMMAND;
                }
                else{
                    state = PARSER_STATE_ERROR;
                }
                break;
            case PARSER_STATE_COMMAND:
                if(c != '\n'){
                    buffer[buffer_count] = c;
                    buffer_count++;

                    state = PARSER_STATE_COMMAND;
                }
                else if(c == '\n' || c == EOF){
                    buffer[buffer_count] = 0;
                    buffer_count++;
                    uint16_t keycode;
                    sscanf(buffer, "%i", &keycode);
                    kbmk_keyboards[keyboard_count-1].command[keycode] = malloc(strlen(&(buffer[5]))+1);
                    strcpy(kbmk_keyboards[keyboard_count-1].command[keycode], &(buffer[5]));

                    buffer_count = 0;

                    if(c == EOF)
                        return err;

                    state = PARSER_STATE_NEWLINE;
                }
                else{
                    state = PARSER_STATE_ERROR;
                }
                break;
            case PARSER_STATE_ERROR:
            default:
                err.parse_file = 1;
                return err;
                break;
        }
    } while(c != EOF);
    
}

void kbmk_keyboards_free(){
    for(int i = 0;i < MAX_KEYBOARDS;i++){
        free(kbmk_keyboards[i].event_path);
        for(int j = 0;j < MAX_EVENTS;j++){
            if(kbmk_keyboards[i].command[j] != NULL)
                free(kbmk_keyboards[i].command[j]);
        }
    }
}

kbmk_error_t kbmk_handle_kb_events(){
    kbmk_error_t err;
    kbmk_flags.halt = 0;
    int event_files[keyboard_count];
    for(int i = 0;i < keyboard_count;i++){
        event_files[i] = open(kbmk_keyboards[i].event_path, O_RDONLY);
        if(event_files[i] == -1){
            err.handle_kb_events_open = 1;
            continue;
        }
        int flag, ret;
        ret = ioctl(event_files[i], EVIOCGRAB, &flag);
        if(ret == -1){
            err.handle_kb_events_grab = 1;
            continue;
        }
        printf("%s\n", kbmk_keyboards[i].event_path);
    }
    while(kbmk_flags.halt == 0){
        for(int i = 0;i < keyboard_count;i++){
            int input_read;
            struct input_event event;
            input_read = read(event_files[i], &event, sizeof(event));
            if(input_read == sizeof(event) && event.type == EV_KEY && event.value == 1){
                printf("code: %x, type: %x, time: %x , value: %x\n", event.code, event.type, event.time.tv_usec, event.value);
                if(kbmk_keyboards[i].command[event.code] != NULL){
                    printf("command: %s\n", kbmk_keyboards[i].command[event.code]);
                    if(kbmk_keyboards[i].command[event.code][0] == ';'){
                        kbmk_flags.halt = 1;
                        printf("exit\n");
                    }
                    else{
                        system(kbmk_keyboards[i].command[event.code]);
                    }
                }
            }
        }
    }
    return err;
}

int contains_const(char a, const char *b){
    int match = 0;
    int i = 0;
    while(b[i] != 0){
        if(a == b[i])
            match = 1;
        i++;
    }
    return match;
}
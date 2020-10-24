#include "kbmk.h"

struct kbmk_keyboard *kbmk_keyboard_list_push_back(struct kbmk_keyboard_list *list, struct kbmk_keyboard *keyboard){
    if(list->head == NULL){
        list->head = keyboard;
        list->tail = list->head;
    }
    else{
        
        list->tail->next = keyboard;
        list->tail = list->tail->next;
    }
    return list->tail;
}
void kbmk_keyboard_list_free(struct kbmk_keyboard_list *list){
    if(list == NULL){

    }
    else if(list->head == NULL){
        free(list);
    }
    else{
        kbmk_keyboard_free(list->head);
        free(list);
    }
}
void kbmk_keyboard_free(struct kbmk_keyboard *keyboard){
    if(keyboard == NULL){

    }
    else{
        for(int i = 0;i < MAX_KEYS;i++){
            free(keyboard->keys[i].command);
        }
        kbmk_keyboard_free(keyboard->next);
        free(keyboard);
    }
}

struct kbmk_keyboard_list *kbmk_parse(int fd){

    
    struct kbmk_keyboard_list *list = malloc(sizeof(struct kbmk_keyboard_list));

    *list = kBMK_KEYBOARD_LIST_INIT();

    uint8_t state = PARSER_STATE_BEGINING;
    uint8_t prev_state = PARSER_STATE_BEGINING;
    char c = 0;
    char buffer[1024];
    ssize_t ret = 0;
    uint16_t buffer_count = 0;
    do{
        ret = read(fd, &c, sizeof(char));
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
                else if(strchr(DIGITS_10, c) != NULL){
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

                    struct kbmk_keyboard *keyboard = malloc(sizeof(struct kbmk_keyboard));
                    *keyboard = KBMK_KEYBOARD_INIT(malloc(strlen(buffer)+1));

                    *kbmk_keyboard_list_push_back(list, keyboard); 
                    strcpy(list->tail->path, buffer);
                    
                    buffer_count = 0;

                    state = PARSER_STATE_NEWLINE;
                }
                else{
                    state = PARSER_STATE_ERROR;
                }
                break;
            case PARSER_STATE_ADDRESS:
                if(strchr(DIGITS_10, c) != NULL){
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
                else if(c == '\n' || ret == 0){
                    buffer[buffer_count] = 0;
                    buffer_count++;
                    uint16_t keycode;
                    sscanf(buffer, "%4u", &keycode);
                    list->tail->keys[keycode] = KBMK_KEY_INIT(malloc(strlen(buffer)));
                    strcpy((list->tail->keys)[keycode].command, &(buffer[5]));

                    buffer_count = 0;

                    if(ret == 0)
                        return NULL;

                    state = PARSER_STATE_NEWLINE;
                }
                else{
                    state = PARSER_STATE_ERROR;
                }
                break;
            case PARSER_STATE_ERROR:
            default:
                
                return NULL;
                break;
        }
    } while(ret != 0);
    return list;
}

void kbmk_main(struct kbmk_keyboard_list *list){

    if(list == NULL){
        return ;
    }
    else if(list->head == NULL){
        return ;
    }

    //int fanotfd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT | FAN_NONBLOCK, O_RDONLY);
    int list_size = 0;
    for(struct kbmk_keyboard *tmp = list->head;tmp != NULL; tmp = tmp->next){
        list_size++;
    }
    struct pollfd pfd[list_size];

    int ioctl_flag, ioctl_ret;
    {
        int i = 0;
        for(struct kbmk_keyboard *tmp = list->head;tmp != NULL; tmp = tmp->next, i++){
            tmp->fd = open(tmp->path, O_RDONLY);
            pfd[i].fd = tmp->fd;
            pfd[i].events = POLLIN;
            ioctl_ret = ioctl(tmp->fd, EVIOCGRAB, &ioctl_flag);
        }
    }

    
    /*int flag, ret;
    ret = ioctl(event_files[i], EVIOCGRAB, &flag);*/

    int ev_size = 0;
    struct input_event event;
    while(1){
        poll(pfd, list_size, -1);
        for(int i = 0;i < list_size;i++){
            if((pfd[i].revents & POLLIN) == POLLIN){
                ev_size = read(pfd[i].fd, &event, sizeof(struct input_event));
                if(ev_size == sizeof(event) && event.type == EV_KEY && event.value == 1){
                    printf("code: %x, type: %x, time: %x , value: %x\n", event.code, event.type, event.time.tv_usec, event.value);
                    struct kbmk_keyboard *tmp = list->head;
                    for(int j = 0;j < i;tmp = tmp->next, j++);
                    if(tmp->keys[event.code].command != NULL){
                        if(!strcmp(tmp->keys[event.code].command,";")){
                            return;
                        }
                        else{
                            popen(tmp->keys[event.code].command, "r");
                        }
                    }
                }
            }
        }
        
                
    }
    {
        int i = 0;
        for(struct kbmk_keyboard *tmp = list->head;tmp != NULL; tmp = tmp->next, i++){
            tmp->fd = close(tmp->fd);
        }
    }


}

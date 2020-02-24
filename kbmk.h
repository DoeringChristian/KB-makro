#ifndef KBMK_H
#define KBMK_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>

#define PARSER_STATE_BEGINING 1
#define PARSER_STATE_NEWLINE 2
#define PARSER_STATE_KBID 3
#define PARSER_STATE_ADDRESS 4
#define PARSER_STATE_COMMAND 5
#define PARSER_STATE_ERROR 0
#define MAX_KEYBOARDS 10
#define MAX_EVENTS 0x10000

typedef unsigned int uint;

//constants
extern const char help_string[];
extern const char alph_digit[];
extern const char file_devices[];
extern const char file_events[];

//structs
typedef struct kbmk_error{
    uint8_t parse_file : 1;
    uint8_t file : 1;
    uint8_t handle_kb_events_open : 1;
    uint8_t handle_kb_events_grab : 1;
} kbmk_error_t;

struct{
    uint8_t halt : 1;
} kbmk_flags;

typedef struct kbmk_keyboard{
    char *event_path;
    char *command[MAX_EVENTS];
} kbmk_keyboard_t;

kbmk_keyboard_t kbmk_keyboards[10];
extern uint keyboard_count;

//functions
kbmk_error_t parse_file(FILE *fp);
void kbmk_keyboards_free();
kbmk_error_t kbmk_handle_kb_events();
int contains_const(char a, const char *b);
kbmk_error_t kbmk_handle_internal_event(char *event);



#endif //KBMK_H

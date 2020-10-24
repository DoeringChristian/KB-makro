#ifndef KBMK_H
#define KBMK_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <linux/input.h>
#include <sys/fanotify.h>
#include <limits.h>
#include <poll.h>


#define PARSER_STATE_BEGINING 1
#define PARSER_STATE_NEWLINE 2
#define PARSER_STATE_KBID 3
#define PARSER_STATE_ADDRESS 4
#define PARSER_STATE_COMMAND 5
#define PARSER_STATE_ERROR 0
#define MAX_KEYS 0x10000

#define DIGITS_10 "0123456789"

#define KBMK_KEY_INIT(_command) (struct kbmk_key){.command = (_command)}
#define KBMK_KEYBOARD_INIT(_path) (struct kbmk_keyboard){.path = (_path), .next = NULL, .keys = {0}}
#define kBMK_KEYBOARD_LIST_INIT() (struct kbmk_keyboard_list){.head = NULL, .tail = NULL}

struct kbmk_key{
    char *command;
};

struct kbmk_keyboard{
    struct kbmk_key keys[MAX_KEYS];
    char *path;
    struct kbmk_keyboard *next;
    int fd;
};

struct kbmk_keyboard_list{
    struct kbmk_keyboard *head;
    struct kbmk_keyboard *tail;
};

struct kbmk_keyboard *kbmk_keyboard_list_push_back(struct kbmk_keyboard_list *list, struct kbmk_keyboard *keyboard);
void kbmk_keyboard_list_free(struct kbmk_keyboard_list *list);
void kbmk_keyboard_free(struct kbmk_keyboard *keyboard);

struct kbmk_keyboard_list *kbmk_parse(int fd);
void kbmk_main(struct kbmk_keyboard_list *list);


#endif //KBMK_H

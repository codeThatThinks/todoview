/**** TodoItem data structure and helper functions ****/

#ifndef TODOITEM_H
#define TODOITEM_H

typedef struct
{
    char *key;              // dynamic string
    char *value;            // dynamic string
} TodoItemKey;

typedef struct
{
    char *text;             // dynamic string
    int completed;          // 1: true, 0: false
    char priority;          // 0 if no priority
    char *creation_date;    // dynamic string; NULL if no date
    char *completion_date;  // dynamic string; NULL if no date
    char **lists;           // dynamic array of dynamic strings
    int num_lists;
    char **tags;            // dynamic string array
    int num_tags;
    TodoItemKey *keys;      // dynamic TodoItemKey array
    int num_keys;
} TodoItem;

extern void todoitem_init(TodoItem *item);
extern void todoitem_debug(TodoItem *item);
extern void todoitem_free(TodoItem *item);
extern int todoitem_has_list(TodoItem *item, char *list);
extern int todoitem_has_tag(TodoItem *item, char *tag);

#endif

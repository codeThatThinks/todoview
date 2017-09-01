/**** Parses todo.txt files ****/

#ifndef MODEL_H
#define MODEL_H

#include "todoitem.h"

typedef enum
{
	FILTER_TYPE_LIST,
	FILTER_TYPE_TAG
} FilterItemType;

typedef struct
{
	char *name;
	FilterItemType type;
} FilterItem;

extern int model_load_items(char *file_path);
extern int model_save_items(char *file_path);
extern int model_get_items(TodoItem ***refined_items, char **filters, int num_filters, char **sorts);
extern void model_free(void);

#endif

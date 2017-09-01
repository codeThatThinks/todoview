/**** TodoItem data structure and helper functions ****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "todoitem.h"

void todoitem_init(TodoItem *item);
void todoitem_debug(TodoItem *item);
void todoitem_free(TodoItem *item);
int todoitem_has_list(TodoItem *item, char *list);
int todoitem_has_tag(TodoItem *item, char *tag);

void todoitem_init(TodoItem *item)
{
	item->text = NULL;
	item->completed = 0;
	item->priority = 0;
	item->creation_date = NULL;
	item->completion_date = NULL;
	item->lists = NULL;
	item->num_lists = 0;
	item->tags = NULL;
	item->num_tags = 0;
	item->keys = NULL;
	item->num_keys = 0;
}

void todoitem_debug(TodoItem *item)
{
	printf("\nTodoItem\n");

	if(item->text)
		printf("item->text\t%s\n", item->text);
	else
		printf("item->text\tNULL\n");

	printf("item->completed\t%i\n", item->completed);
	printf("item->priority\t%c\n", item->priority);

	if(item->creation_date)
		printf("item->creation_date\t%s\n", item->creation_date);
	else
		printf("item->creation_date\tNULL\n");

	if(item->completion_date)
		printf("item->completion_date\t%s\n", item->completion_date);
	else
		printf("item->completion_date\tNULL\n");

	if(item->lists)
		for(int i = 0; i < item->num_lists; i++)
		{
			if(item->lists[i])
				printf("item->lists[%i]\t%s\n", i, item->lists[i]);
			else
				printf("item->lists[%i]\tNULL\n", i);
		}
	else
		printf("item->lists\tNULL\n");

	printf("item->num_lists\t%i\n", item->num_lists);

	if(item->tags)
		for(int i = 0; i < item->num_tags; i++)
		{
			if(item->tags[i])
				printf("item->tags[%i]\t%s\n", i, item->tags[i]);
			else
				printf("item->tags[%i]\tNULL\n", i);
		}
	else
		printf("item->tags\tNULL\n");

	printf("item->num_tags\t%i\n", item->num_tags);

	if(item->keys)
		for(int i = 0; i < item->num_keys; i++)
		{
			if(item->keys[i].key)
				printf("item->keys[%i].key\t%s\n", i, item->keys[i].key);
			else
				printf("item->keys[%i].key\tNULL\n", i);

			if(item->keys[i].value)
				printf("item->keys[%i].value\t%s\n", i, item->keys[i].value);
			else
				printf("item->keys[%i].value\tNULL\n", i);
		}
	else
		printf("item->keys\tNULL\n");

	printf("item->num_keys\t%i\n\n", item->num_keys);
}

void todoitem_free(TodoItem *item)
{
	free(item->text);
	free(item->creation_date);
	free(item->completion_date);

	for(int i = 0; i < item->num_lists; i++)
	{
		free(item->lists[i]);
	}
	free(item->lists);

	for(int i = 0; i < item->num_tags; i++)
	{
		free(item->tags[i]);
	}
	free(item->tags);

	for(int i = 0; i < item->num_keys; i++)
	{
		free(item->keys[i].key);
		free(item->keys[i].value);
	}
	free(item->keys);
}

int todoitem_has_list(TodoItem *item, char *list)
{
	for(int i = 0; i < item->num_lists; i++)
	{
		if(strcmp(item->lists[i], list) == 0) return 1;
	}

	return 0;
}

int todoitem_has_tag(TodoItem *item, char *tag)
{
	for(int i = 0; i < item->num_tags; i++)
	{
		if(strcmp(item->tags[i], tag) == 0) return 1;
	}

	return 0;
}

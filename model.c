/**** Parses todo.txt files ****/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "model.h"
#include "todoitem.h"

TodoItem *items = NULL;
int num_items = 0;

void str_to_item(char *str, TodoItem *item);
void item_to_str(TodoItem *item, char *str);

int model_load_items(char *file_path);															/** returns: number of items read **/
int model_save_items(char *file_path);															/** returns: number of items written **/
int model_get_items(TodoItem ***refined_items, char **filters, int num_filters, char **sorts);	/** returns: sorted TodoItems array **/
void model_free(void);

void str_to_item(char *str, TodoItem *item)
{
	todoitem_init(item);

	// trim whitespace and copy into struct
	int start = 0, len = strlen(str);
	while(isspace(str[start])) start++;
	while(isspace(str[len - 1])) len--;

	item->text = (char *)malloc(sizeof(char) * (len - start + 1));
	strncpy(item->text, &str[start], len);
	item->text[len - start] = '\0';
	len -= start;

	// check if completed
	if(item->text[0] == 'x'
		&& item->text[1] == ' ')
	{
		item->completed = 1;

		for(int j = 0; j <= len - 2; j++) item->text[j] = item->text[j + 2];
		len -= 2;
	}

	// check for priority
	if(item->text[0] == '('
		&& item->text[1] >= 65 && item->text[1] <= 90
		&& item->text[2] == ')'
		&& item->text[3] == ' ')
	{
		item->priority = item->text[1];

		for(int j = 0; j <= len - 4; j++) item->text[j] = item->text[j + 4];
		len -= 4;
	}

	// check for completion/creation date
	item->completion_date = NULL;
	item->creation_date = NULL;
	if(item->text[0] >= 48 && item->text[0] <= 57
		&& item->text[1] >= 48 && item->text[1] <= 57
		&& item->text[2] >= 48 && item->text[2] <= 57
		&& item->text[3] >= 48 && item->text[3] <= 57
		&& item->text[4] == '-'
		&& ((item->text[5] - 48) * 10 + item->text[6] - 48) >= 1
		&& ((item->text[5] - 48) * 10 + item->text[6] - 48) <= 12
		&& item->text[7] == '-'
		&& ((item->text[8] - 48) * 10 + item->text[9] - 48) >= 1
		&& ((item->text[8] - 48) * 10 + item->text[9] - 48) <= 31
		&& item->text[10] == ' ')
	{	
		if(item->completed)
		{
			// completion date, check that creation date also exists, otherwise ignore
			if(item->text[11] >= 48 && item->text[11] <= 57
				&& item->text[12] >= 48 && item->text[12] <= 57
				&& item->text[13] >= 48 && item->text[13] <= 57
				&& item->text[14] >= 48 && item->text[14] <= 57
				&& item->text[15] == '-'
				&& ((item->text[16] - 48) * 10 + item->text[17] - 48) >= 1
				&& ((item->text[16] - 48) * 10 + item->text[17] - 48) <= 12
				&& item->text[18] == '-'
				&& ((item->text[19] - 48) * 10 + item->text[20] - 48) >= 1
				&& ((item->text[19] - 48) * 10 + item->text[20] - 48) <= 31
				&& item->text[21] == ' ')
			{
				item->completion_date = (char *)malloc(sizeof(char) * 11);
				strncpy(item->completion_date, &(item->text[0]), 10);
				item->completion_date[10] = '\0';

				item->creation_date = (char *)malloc(sizeof(char) * 11);
				strncpy(item->creation_date, &(item->text[11]), 10);
				item->creation_date[10] = '\0';

				for(int j = 0; j <= len - 22; j++) item->text[j] = item->text[j + 22];
				len -= 22;
			}
		}
		else
		{
			// creation date
			item->creation_date = (char *)malloc(sizeof(char) * 11);
			strncpy(item->creation_date, &(item->text[0]), 10);
			item->creation_date[10] = '\0';

			for(int j = 0; j <= len - 11; j++) item->text[j] = item->text[j + 11];
			len -= 11;
		}
	}

	// parse string for lists, tags, and keys
	for(int i = 0; i < len; i++)
	{
		if(item->text[i] == '@'
			&& (i == 0 || item->text[i - 1] == ' ')
			&& i != len - 1
			&& item->text[i + 1] != ' ')
		{
			// list
			int j = i + 1;
			while(!isspace(item->text[j]) && j < len) j++;

			item->num_lists++;
			item->lists = (char **)realloc(item->lists, sizeof(char *) * item->num_lists);
			item->lists[item->num_lists - 1] = (char *)malloc(sizeof(char) * (j - i));
			strncpy(item->lists[item->num_lists - 1], &(item->text[i + 1]), j - i - 1);
			item->lists[item->num_lists - 1][j - i - 1] = '\0';

			if(CONFIG_HIDE_TAGS)
			{
				if(j == len)
				{
					item->text[i] = '\0';
					len -= j - i;
				}
				else
				{
					for(int k = i; k < len - (j - i) + 1; k++) item->text[k] = item->text[k + (j - i) + 1];
					len -= j - i + 1;
				}
				
				i--;
			}
		}
		else if(item->text[i] == '+'
			&& (i == 0 || item->text[i - 1] == ' ')
			&& i != len - 1
			&& item->text[i + 1] != ' ')
		{
			// tag
			int j = i + 1;
			while(!isspace(item->text[j]) && j < len) j++;

			item->num_tags++;
			item->tags = (char **)realloc(item->tags, sizeof(char *) * item->num_tags);
			item->tags[item->num_tags - 1] = (char *)malloc(sizeof(char) * (j - i));
			strncpy(item->tags[item->num_tags - 1], &(item->text[i + 1]), j - i - 1);
			item->tags[item->num_tags - 1][j - i - 1] = '\0';

			if(CONFIG_HIDE_TAGS)
			{
				if(j == len)
				{
					item->text[i] = '\0';
					len -= j - i;
				}
				else
				{
					for(int k = i; k < len - (j - i) + 1; k++) item->text[k] = item->text[k + (j - i) + 1];
					len -= j - i + 1;
				}

				i--;
			}
		}
		else if((i == 0 || item->text[i - 1] == ' ')
			&& i != len - 1
			&& item->text[i + 1] != ' '
			&& !isspace(item->text[i]))
		{
			// check for key
			int j = i + 1;
			while(!isspace(item->text[j]) && j < len)
			{
				if(item->text[j] == ':')
				{
					int k = j + 1;
					while(!isspace(item->text[k]) && k < len) k++;

					item->num_keys++;
					item->keys = (TodoItemKey *)realloc(item->keys, sizeof(TodoItemKey) * item->num_keys);

					item->keys[item->num_keys - 1].key = (char *)malloc(sizeof(char) * (j - i + 1));
					strncpy(item->keys[item->num_keys - 1].key, &(item->text[i]), j - i);
					item->keys[item->num_keys - 1].key[j - i] = '\0';

					item->keys[item->num_keys - 1].value = (char *)malloc(sizeof(char) * (k - (j + 1) + 1));
					strncpy(item->keys[item->num_keys - 1].value, &(item->text[j + 1]), k - (j + 1));
					item->keys[item->num_keys - 1].value[k - (j + 1)] = '\0';

					if(CONFIG_HIDE_TAGS)
					{
						if(k == len)
						{
							item->text[i] = '\0';
							len -= k - i;
						}
						else
						{
							for(int l = i; l < len - (k - i) + 1; l++) item->text[l] = item->text[l + (k - i) + 1];
							len -= k - i + 1;
						}

						i--;
					}
					
					break;
				}

				j++;
			}
		}
	}

	// trim whitespace
	start = 0;
	while(isspace(item->text[start])) start++;
	while(isspace(item->text[len - 1])) len--;
	for(int i = 0; i < len - start; i++) item->text[i] = item->text[i + start];
	item->text[len - start] = '\0';
	item->text = (char *)realloc(item->text, sizeof(char) * (len - start + 1));
}

void item_to_str(TodoItem *item, char *str)
{

}

int model_load_items(char *file_path)
{
	model_free();

	char buffer[256];
	FILE *file = fopen(file_path, "r");
	if(file == NULL)
	{
		return 0;
	}

	while(!feof(file))
	{
		fgets(buffer, 256, file);

		if(buffer[0] != '\n' && buffer[0] != '\r')
		{
			for(int i = 0; i < 256; i++) if(buffer[i] == '\n') buffer[i] = '\0';

			num_items++;
			items = (TodoItem *)realloc(items, sizeof(TodoItem) * num_items);
			str_to_item(buffer, &items[num_items - 1]);
		}
	}

	fclose(file);

	return num_items;
}

int model_save_items(char *file_path)
{
	return 0;
}

int model_get_items(TodoItem ***refined_items, char **filters, int num_filters, char **sorts)
{
	int num_refined_items = 0;

	for(int i = 0; i < num_items; i++)
	{
		if(num_filters == 1
			&& strlen(filters[0]) == 0)
		{
			num_refined_items++;
			*refined_items = (TodoItem **)realloc(*refined_items, sizeof(TodoItem *) * num_refined_items);
			(*refined_items)[num_refined_items - 1] = &items[i];
		}
		else
		{
			for(int j = 0; j < num_filters; j++)
			{
				if(strlen(filters[j]) > 0
					&& todoitem_has_list(&items[i], filters[j]))
				{
					num_refined_items++;
					*refined_items = (TodoItem **)realloc(*refined_items, sizeof(TodoItem *) * num_refined_items);
					(*refined_items)[num_refined_items - 1] = &items[i];
					break;
				}
			}
		}
	}

	return num_refined_items;
}

void model_free()
{
	for(int i = 0; i < num_items; i++)
	{
		todoitem_free(&items[i]);
	}
	free(items);
	items = NULL;
	num_items = 0;
}

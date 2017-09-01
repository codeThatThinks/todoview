/**** Todo.txt viewer ****/

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "model.h"

int state;				/* 	0 = normal view
							1 = filter window open */
time_t file_last_update;

int num_panes;
int selected_pane;
WINDOW **panes;
int *pane_num_filters;
char ***pane_filters;	/* array of array of strings */
char ***pane_sorts;		/* array of array of strings */
WINDOW *filter_window;
int filter_current_line;

void add_pane(void);
void remove_pane(void);
void update_panes(void);
int get_pane_width(int pane);
int get_pane_offset(int pane);
void update_filter_window(void);

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("Usage: todoview [todo.txt]\n\n");
		return 1;
	}

	model_load_items(argv[1]);
	time(&file_last_update);

	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	refresh();

	num_panes = 0;
	panes = NULL;
	pane_num_filters = NULL;
	pane_filters = NULL;
	pane_sorts = NULL;

	add_pane();
	update_panes();
	selected_pane = 0;
	state = 0;

	while(1)
	{
		if(state == 0)
		{
			int c = getch();

			if(c == 43)
			{
				// plus
				if((COLS / (num_panes + 1)) >= CONFIG_MIN_PANE_WIDTH)
				{
					add_pane();
				}
				update_panes();
			}
			else if(c == 45)
			{
				// minus
				if(num_panes > 1)
				{
					remove_pane();
				}
				update_panes();
			}
			else if(c == 260)
			{
				// left arrow
				if(selected_pane > 0) selected_pane--;
				update_panes();
			}
			else if(c == 261)
			{
				// right arrow
				if(selected_pane < num_panes - 1) selected_pane++;
				update_panes();
			}
			else if(c == 10)
			{
				// enter, open filter window for current pane
				state = 1;
				filter_current_line = 0;
				update_filter_window();
			}

			// update todo list every 30 seconds
			if(time(NULL) >= file_last_update + CONFIG_FILE_UPDATE_FREQ)
			{
				model_load_items(argv[1]);
				time(&file_last_update);
				update_panes();
			}
		}
		else if(state == 1)
		{
			int c = wgetch(filter_window);

			if(c == 10
				|| (c == 27 && (c = wgetch(filter_window)) == ERR))
			{
				// escape or enter
				state = 0;
				wborder(filter_window, ' ', ' ', ' ',' ',' ',' ',' ',' ');
				wrefresh(filter_window);
				delwin(filter_window);
				filter_window = NULL;

				update_panes();
			}
			else if(c == KEY_UP)
			{
				// up arrow
				if(filter_current_line > 0) filter_current_line--;
				update_filter_window();
			}
			else if(c == KEY_DOWN)
			{
				// down arrow
				if(filter_current_line < pane_num_filters[selected_pane] - 1) filter_current_line++;
				update_filter_window();
			}
			else if(c == KEY_BACKSPACE)
			{
				// backspace
				int x, y;
				(void)y;
				getyx(filter_window, y, x);

				if(strlen(pane_filters[selected_pane][filter_current_line]) > 0)
				{
					mvwprintw(filter_window, filter_current_line, x - 1, " ");
					wmove(filter_window, filter_current_line, x - 1);
					wrefresh(filter_window);
					int len = strlen(pane_filters[selected_pane][filter_current_line]);
					pane_filters[selected_pane][filter_current_line] = (char *)realloc(pane_filters[selected_pane][filter_current_line], sizeof(char) * len);
					pane_filters[selected_pane][filter_current_line][len - 1] = '\0';

					if(strlen(pane_filters[selected_pane][pane_num_filters[selected_pane] - 1]) == 0
						&& filter_current_line == (pane_num_filters[selected_pane] - 2)
						&& strlen(pane_filters[selected_pane][filter_current_line]) == 0)
					{
						free(pane_filters[selected_pane][pane_num_filters[selected_pane] - 1]);
						pane_num_filters[selected_pane]--;
						pane_filters[selected_pane] = (char **)realloc(pane_filters[selected_pane], sizeof(char *) * pane_num_filters[selected_pane]);

						update_filter_window();
					}
				}
			}
			else if(c != 127
				&& c >= 32
				&& c <= 254
				&& wgetch(filter_window) == ERR)
			{
				// printable character
				wprintw(filter_window, "%c", c);
				wrefresh(filter_window);

				int len = strlen(pane_filters[selected_pane][filter_current_line]);
				pane_filters[selected_pane][filter_current_line] = (char *)realloc(pane_filters[selected_pane][filter_current_line], sizeof(char) * (len + 1));
				pane_filters[selected_pane][filter_current_line][len] = c;
				pane_filters[selected_pane][filter_current_line][len + 1] = '\0';
				
				if(filter_current_line == (pane_num_filters[selected_pane] - 1)
					&& strlen(pane_filters[selected_pane][filter_current_line]) > 0)
				{
					pane_num_filters[selected_pane]++;
					pane_filters[selected_pane] = (char **)realloc(pane_filters[selected_pane], sizeof(char *) * pane_num_filters[selected_pane]);
					pane_filters[selected_pane][pane_num_filters[selected_pane] - 1] = (char *)malloc(sizeof(char));
					pane_filters[selected_pane][pane_num_filters[selected_pane] - 1][0] = '\0';

					update_filter_window();
				}
			}
		}
	}

	endwin();
	free(panes);
	model_free();

	return 0;
}

void add_pane()
{
	// resize all current panes
	for(int i = 0; i < num_panes; i++)
	{
		wborder(panes[i], ' ', ' ', ' ',' ',' ',' ',' ',' ');
		wrefresh(panes[i]);
		delwin(panes[i]);

		panes[i] = newwin(LINES, COLS / (num_panes + 1), 0, COLS / (num_panes + 1) * i);
		wrefresh(panes[i]);
	}

	// add a new pane
	num_panes++;
	panes = (WINDOW **)realloc(panes, sizeof(WINDOW *) * num_panes);
	panes[num_panes - 1] = newwin(LINES, COLS - COLS / num_panes * (num_panes - 1), 0, COLS / num_panes * (num_panes - 1));
	wrefresh(panes[num_panes - 1]);

	pane_num_filters = (int *)realloc(pane_num_filters, sizeof(int) * num_panes);
	pane_num_filters[num_panes - 1] = 1;
	pane_filters = (char ***)realloc(pane_filters, sizeof(char **) * num_panes);
	pane_filters[num_panes - 1] = (char **)malloc(sizeof(char *));
	pane_filters[num_panes - 1][0] = (char *)malloc(sizeof(char));
	pane_filters[num_panes - 1][0][0] = '\0';

	selected_pane = num_panes - 1;
}

void remove_pane()
{
	// delete all windows
	for(int i = 0; i < num_panes; i++)
	{
		wborder(panes[i], ' ', ' ', ' ',' ',' ',' ',' ',' ');
		wrefresh(panes[i]);
		delwin(panes[i]);
	}

	// remove the last pane
	num_panes--;
	panes = (WINDOW **)realloc(panes, sizeof(WINDOW *) * num_panes);
	for(int i = 0; i < pane_num_filters[num_panes]; i++) free(pane_filters[num_panes][i]);
	free(pane_filters[num_panes]);
	pane_filters = (char ***)realloc(pane_filters, sizeof(char **) * num_panes);
	pane_num_filters = (int *)realloc(pane_num_filters, sizeof(int) * num_panes);

	// resize all current panes
	for(int i = 0; i < num_panes; i++)
	{
		if(i == num_panes - 1)
		{
			panes[i] = newwin(LINES, COLS - COLS / num_panes * (num_panes - 1), 0, COLS / num_panes * (num_panes - 1));
		}
		else
		{
			panes[i] = newwin(LINES, COLS / num_panes, 0, COLS / num_panes * i);
		}
		
		wrefresh(panes[i]);
	}

	selected_pane = num_panes - 1;
}

int get_pane_width(int pane)
{
	if(pane == num_panes - 1)
	{
		return COLS - COLS / num_panes * (num_panes - 1);
	}
	else
	{
		return COLS / num_panes;
	}
}

int get_pane_offset(int pane)
{
	if(pane == num_panes - 1)
	{
		return COLS / num_panes * (num_panes - 1);
	}
	else
	{
		return COLS / num_panes * pane;
	}
	
}

void update_panes()
{
	for(int i = 0; i < num_panes; i++)
	{
		wclear(panes[i]);
		if(i < num_panes - 1)
		{
			wborder(panes[i], ' ', ACS_VLINE, ' ', ' ', ' ', ACS_VLINE, ' ', ACS_VLINE);
		}

		if(selected_pane == i)
		{
			wattron(panes[i], A_REVERSE);
		}
		else
		{
			wattron(panes[i], A_BOLD);
		}
		if(strlen(pane_filters[i][0]) != 0)
		{
			char *filter_str = (char *)malloc(sizeof(char) * (strlen(pane_filters[i][0]) + 1));
			strcpy(filter_str, pane_filters[i][0]);

			if(pane_num_filters[i] > 2
				|| (pane_num_filters[i] == 2 && strlen(pane_filters[i][1]) != 0))
			{
				filter_str = (char *)realloc(filter_str, sizeof(char) * (strlen(pane_filters[i][0]) + 5));
				filter_str[strlen(pane_filters[i][0])] = ' ';
				filter_str[strlen(pane_filters[i][0]) + 1] = '.';
				filter_str[strlen(pane_filters[i][0]) + 2] = '.';
				filter_str[strlen(pane_filters[i][0]) + 3] = '.';
				filter_str[strlen(pane_filters[i][0]) + 4] = '\0';
			}

			if(i == num_panes - 1)
			{
				mvwprintw(panes[i], 0, 1, "%s", filter_str);
				for(int j = 0; j < (get_pane_width(i) - 1) - strlen(filter_str); j++) wprintw(panes[i], " ");
			}
			else
			{
				mvwprintw(panes[i], 0, 1, "%s", filter_str);
				for(int j = 0; j < (get_pane_width(i) - 2) - strlen(filter_str); j++) wprintw(panes[i], " ");
			}

			free(filter_str);
		}
		else
		{
			if(pane_num_filters[i] > 2
				|| (pane_num_filters[i] == 2 && strlen(pane_filters[i][1]) != 0))
			{
				mvwprintw(panes[i], 0, 1, "-- ...");
				if(i == num_panes - 1)
				{
					for(int j = 6; j < get_pane_width(i) - 1; j++) wprintw(panes[i], " ");
				}
				else
				{
					for(int j = 6; j < get_pane_width(i) - 2; j++) wprintw(panes[i], " ");
				}
			}
			else
			{
				mvwprintw(panes[i], 0, 1, "--");
				if(i == num_panes - 1)
				{
					for(int j = 3; j < get_pane_width(i) - 1; j++) wprintw(panes[i], " ");
				}
				else
				{
					for(int j = 3; j < get_pane_width(i) - 2; j++) wprintw(panes[i], " ");
				}
			}
		}
		if(selected_pane == i)
		{
			wattroff(panes[i], A_REVERSE);
		}
		else
		{
			wattroff(panes[i], A_BOLD);
		}

		TodoItem **pane_items = NULL;
		int num_pane_items = model_get_items(&pane_items, pane_filters[i], pane_num_filters[i], NULL);
		int current_line = 1;
		int word_wrap = get_pane_width(i) - 3;
		if(i == num_panes - 1) word_wrap++;

		for(int j = 0; j < num_pane_items; j++)
		{
			if(current_line < COLS)
			{
				if(strlen(pane_items[j]->text) > word_wrap)
				{
					int chars_to_print = strlen(pane_items[j]->text);
					mvwprintw(panes[i], current_line, 1, "%.*s", word_wrap, pane_items[j]->text);
					current_line++;
					chars_to_print -= word_wrap;

					while(chars_to_print > 0)
					{
						mvwprintw(panes[i], current_line, 3, "%.*s", word_wrap - 2, &(pane_items[j]->text[strlen(pane_items[j]->text) - chars_to_print]));
						current_line++;
						chars_to_print -= word_wrap;
					}
				}
				else
				{
					mvwprintw(panes[i], current_line, 1, "%s", pane_items[j]->text);
					current_line++;
				}
			}
		}
		
		wrefresh(panes[i]);
	}
}

void update_filter_window()
{
	if(filter_window != NULL)
	{
		wborder(filter_window, ' ', ' ', ' ',' ',' ',' ',' ',' ');
		wrefresh(filter_window);
		delwin(filter_window);
	}

	if(selected_pane == num_panes - 1)
	{
		filter_window = newwin(pane_num_filters[selected_pane] + 1, get_pane_width(selected_pane), 0, get_pane_offset(selected_pane));
	}
	else
	{
		filter_window = newwin(pane_num_filters[selected_pane] + 1, get_pane_width(selected_pane) - 1, 0, get_pane_offset(selected_pane));
	}
	nodelay(filter_window, TRUE);
	keypad(filter_window, TRUE);
	wborder(filter_window, ' ', ' ', ' ',ACS_HLINE,' ',' ',' ',' ');

	for(int i = 0; i < pane_num_filters[selected_pane]; i++)
	{
		if(i == filter_current_line)
		{
			mvwprintw(filter_window, i, 0, "> %s", pane_filters[selected_pane][i]);
		}
		else
		{
			mvwprintw(filter_window, i, 2, "%s", pane_filters[selected_pane][i]);
		}

		wmove(filter_window, filter_current_line, strlen(pane_filters[selected_pane][filter_current_line]) + 2);
	}

	wrefresh(filter_window);
}

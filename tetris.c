#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define CPS CLOCKS_PER_SEC
// #define SHAPE_LENGTH 4
// #define MAX_NUM_SHAPE 7
#define WIDTH 10
#define HEIGH 20
#define SCORE_X_OFFSET 7
// #define PAUSE_Y 14
// #define PAUSE_X 52
// #define PAUSE_MSG_LEN 5
#define GAME_OVER_OFFSET 2
#define GAME_OVER_MSG_LEN 9
#define GAME_NAME_LEN 6

enum key_buttons { 
	key_exit1 = 113,	// <q>
	key_exit2 = 81,		// <Q>
	key_pause = 27,		// <Escape>
	key_new_game = 10, 	// <Enter>	
	key_rotate = 32, 	// <Space>
	key_save = 115
};

enum { pattern_length = 4, pattern_count = 7 };

// all possible variants of 7 patterns
enum { 
	pattern0 = 0,

	pattern1 = 10, 
	pattern2 = 11,
	   
	pattern3 = 20, 
	pattern4 = 21,
	   
	pattern5 = 30, 
	pattern6 = 31,
	   
	pattern7 = 40, 
	pattern8 = 41, 
	pattern9 = 42,
	pattern10 = 43,
	   
	pattern11 = 50, 
	pattern12 = 51, 
	pattern13 = 52,
	pattern14 = 53,
	   
 	pattern15 = 60, 
	pattern16 = 61, 
	pattern17 = 62,
	pattern18 = 63,
};

// start coordinates of new pattern
enum { 
	y_0 = 0, 
	y_1 = 1, 
	y_2 = 2, 
	y_3 = 3,
	   
	x_0 = 38, 
	x_1 = 39, 
	x_2 = 40, 
	x_3 = 41 
};

enum { 
	pause_y = 14,		// coordinate 'y' of pause message
	pause_x = 52,		// coordinate 'x' of pause message
	pause_msg_len = 5	// 
};


typedef struct tag_cell {
	int x, y;
} cell;

typedef struct tag_lines {
	cell data;
	struct tag_lines *next;	// , *prev;
} lines;

typedef struct tag_boundary {
	int min_x, max_x, min_y, max_y;
	int cur_pattern, next_pattern;
	int is_failed, is_paused;
	int score;
} boundary;

/*	нужна структура, которая будет хранить массив cell с координатами фигуры
	и длинну этого массива length
*/
/*
typedef struct tag_pattern {
	cell *brick;
	int length;
} pattern;
*/

void game_field(boundary *, int , int );
void game_info();
void create_pattern(cell *, boundary *);
int find_next_pattern();
void show_next_pattern(boundary *, int );
void check_boundary_rotate(cell *, boundary *);
int check_occupied_rotate(const cell *, lines *);
void show_hide_pattern(const cell *, boundary *, char );
int check_left_right_mv(cell *, lines *, boundary *, int , int );
int check_mv_down(cell *, lines *, boundary *, int , int );
int check_fail(lines *, boundary *);
void show_hide_occupied(lines *, char );
void add_to_occupied(cell *, lines **);	// , lines **
int check_line();
void destroy_line(lines **, int );
int increase_score(int );
void print_score(int , int );
// void print_occupied(lines *);
void fill(lines **, lines **, boundary *);

void init(cell *s, lines **first, boundary *b) {	// lines **last, 
	int row, col;
	getmaxyx(stdscr, row, col);
	
	game_field(b, row, col);
	game_info();
	b->is_paused = 0;
	b->is_failed = 0;
	b->cur_pattern = -1;
	b->score = 0;
	print_score(b->min_x, b->score);
	
	lines *tmp;
	while(*first) {
		tmp = *first;
		*first = (*first)->next;
		free(tmp);
	}
	
	create_pattern(s, b);
	
	// find_next_pattern(&(b->next_pattern));
	
	// fill(first, last, coord);
	// show_hide_occupied(*first, '*');
}
/*
void fill(lines **first, lines **last, boundary *b) {
	int i, j;
	*first = NULL;
	*last = NULL;
	
	for(i = 21; i < 23; i++) {
		for(j = b->min_x + 1; j < b->max_x - 2; j++) {
			lines *tmp = malloc(sizeof(lines));
			tmp->data.y = i;
			tmp->data.x = j;
			
			tmp->next = NULL;
			tmp->prev = *last;
			
			if(*last) {
				(*last)->next = tmp;
			} else {
				*first = tmp;
			}
			*last = tmp;
		}
	}
}
*/
//библиотека стандартных фигур
void pattern_lib(cell *s, int n) {
	switch(n) {
		case pattern0:
			s[0].x = x_1;	//	01
			s[0].y = y_2;	//	**
			s[1].x = x_2;	//	**
			s[1].y = y_2;
			s[2].x = x_1;
			s[2].y = y_3;
			s[3].x = x_2;
			s[3].y = y_3;
			break;
			
		case pattern1:
			s[0].x = x_1;	//	0*
			s[0].y = y_1;	//	1**
			s[1].x = x_1;	//	  *
			s[1].y = y_2;
			s[2].x = x_2;
			s[2].y = y_2;
			s[3].x = x_2;
			s[3].y = y_3;
			break;
		case pattern2:
			s[0].x = x_1;	//	 **
			s[0].y = y_3;	//	**
			s[1].x = x_2;	//	01
			s[1].y = y_3;
			s[2].x = x_2;
			s[2].y = y_2;
			s[3].x = x_3;
			s[3].y = y_2;
			break;	
			
		case pattern3:
			s[0].x = x_2;	//	  *0
			s[0].y = y_1;	//	 **1
			s[1].x = x_2;	//	 *
			s[1].y = y_2;
			s[2].x = x_1;
			s[2].y = y_2;
			s[3].x = x_1;
			s[3].y = y_3;
			break;
		case pattern4:
			s[0].x = x_1;	//	01
			s[0].y = y_2;	//	**
			s[1].x = x_2;	//	 **
			s[1].y = y_2;
			s[2].x = x_2;
			s[2].y = y_3;
			s[3].x = x_3;
			s[3].y = y_3;
			break;	
			
		case pattern5:
			s[0].x = x_2;	//	 *0
			s[0].y = y_0;	//	 *1
			s[1].x = x_2;	//	 *
			s[1].y = y_1;	//	 *
			s[2].x = x_2;
			s[2].y = y_2;
			s[3].x = x_2;
			s[3].y = y_3;
			break;
		case pattern6:
			s[0].x = x_3;	//    10
			s[0].y = y_3;	//	****
			s[1].x = x_2;	
			s[1].y = y_3;
			s[2].x = x_1;
			s[2].y = y_3;
			s[3].x = x_0;
			s[3].y = y_3;
			break;
	
		case pattern7:
			s[0].x = x_2;	//	 *0
			s[0].y = y_1;	//	 *1
			s[1].x = x_2;	//	**
			s[1].y = y_2;
			s[2].x = x_2;
			s[2].y = y_3;
			s[3].x = x_1;
			s[3].y = y_3;
			break;
		case pattern8:
			s[0].x = x_3;	//	*
			s[0].y = y_3;	//	***
			s[1].x = x_2;	//	 10
			s[1].y = y_3;
			s[2].x = x_1;
			s[2].y = y_3;
			s[3].x = x_1;
			s[3].y = y_2;
			break;
		case pattern9:
			s[0].x = x_1;	//	 **
			s[0].y = y_3;	//	1*
			s[1].x = x_1;	//	0*
			s[1].y = y_2;
			s[2].x = x_1;
			s[2].y = y_1;
			s[3].x = x_2;
			s[3].y = y_1;
			break;
		case pattern10:
			s[0].x = x_1;	//	01
			s[0].y = y_2;	//	***
			s[1].x = x_2;	//	  *
			s[1].y = y_2;
			s[2].x = x_3;
			s[2].y = y_2;
			s[3].x = x_3;
			s[3].y = y_3;
			break;
		
		case pattern11:
			s[0].x = x_1;	//	0*
			s[0].y = y_1;	//	1*
			s[1].x = x_1;	//	 **
			s[1].y = y_2;
			s[2].x = x_1;
			s[2].y = y_3;
			s[3].x = x_2;
			s[3].y = y_3;
			break;
		case pattern12:
			s[0].x = x_3;	//	 10
			s[0].y = y_2;	//	***
			s[1].x = x_2;	//	*
			s[1].y = y_2;
			s[2].x = x_1;
			s[2].y = y_2;
			s[3].x = x_1;
			s[3].y = y_3;
			break;
		case pattern13:
			s[0].x = x_2;	//	**
			s[0].y = y_3;	// 	 *1
			s[1].x = x_2;	// 	 *0
			s[1].y = y_2;
			s[2].x = x_2;
			s[2].y = y_1;
			s[3].x = x_1;
			s[3].y = y_1;
			break;
		case pattern14:
			s[0].x = x_1;	//	  *
			s[0].y = y_3;	//	***
			s[1].x = x_2;	//	01
			s[1].y = y_3;
			s[2].x = x_3;
			s[2].y = y_3;
			s[3].x = x_3;
			s[3].y = y_2;
			break;
			
		case pattern15:
			s[0].x = x_1;	//	0*
			s[0].y = y_1;	//	1**2
			s[1].x = x_1;	//	3*
			s[1].y = y_2;
			s[2].x = x_2;
			s[2].y = y_2;
			s[3].x = x_1;
			s[3].y = y_3;
			break;
		case pattern16:
			s[0].x = x_3;	//	310
			s[0].y = y_2;	//	***
			s[1].x = x_2;	//	 *
			s[1].y = y_2;	//	 2
			s[2].x = x_2;
			s[2].y = y_3;
			s[3].x = x_1;
			s[3].y = y_2;
			break;
		case pattern17:
			s[0].x = x_2;	//	 *3
			s[0].y = y_3;	//     2**1
			s[1].x = x_2;	//	 *0
			s[1].y = y_2;
			s[2].x = x_1;
			s[2].y = y_2;
			s[3].x = x_2;
			s[3].y = y_1;
			break;
		case pattern18:
			s[0].x = x_1;	//	 2
			s[0].y = y_3;	//	 *
			s[1].x = x_2;	//	***
			s[1].y = y_3;	//	013
			s[2].x = x_2;
			s[2].y = y_2;
			s[3].x = x_3;
			s[3].y = y_3;
			break;
	}
}
/*
int find_length(int n) {
	int length = -1;
	if(n == pattern7 || n == pattern8) length = 5;
	else length = 4;
	return length;
}
*/
//создание новой фигуры
void create_pattern(cell *s, boundary *b) {
	if(b->cur_pattern != -1) {
		b->cur_pattern = b->next_pattern;
	} else {
		b->cur_pattern = find_next_pattern();
	}
	// *n = 4;
	// int length;
	b->next_pattern = find_next_pattern();
	show_next_pattern(b, b->next_pattern);
	// length = find_length(n);
	
	pattern_lib(s, b->cur_pattern);
	show_hide_pattern(s, b, '*');	// show
	// return len;
}

//генератор фигур
int find_next_pattern() {
	int n = random() % pattern_count;
	
	if(n == 0) return n;
	
	n *= 10;
	if(n < pattern7) 
		n += random() % 2;
	else 
		n += random() % 4;

	return n;
}

void show_next_pattern(boundary *b, int n) {
	int i, j;
	cell arr[pattern_length];
	
	for(i = 7; i < 11; i++) {
		for(j = 52 ; j < 56; j++) {
			move(i, j);
			addch(' ');
		}
	}
	
	pattern_lib(arr, n);
	for(i = 0; i < pattern_length; i++) {
		arr[i].x += 14;
		arr[i].y += 7;
	}
	show_hide_pattern(arr, b, '*');
	refresh();
}

int next_rotate_pos(int current) {
	int next_rotate;
	
	switch(current) {
		case pattern0:	// square
			next_rotate = pattern0;
			break;
			
		case pattern1:	// s
			next_rotate = pattern2;
			break;
		case pattern2:
			next_rotate = pattern1;
			break;
			
		case pattern3:	// z
			next_rotate = pattern4;
			break;
		case pattern4:
			next_rotate = pattern3;
			break;
			
		case pattern5:	// |
			next_rotate = pattern6;
			break;
		case pattern6:
			next_rotate = pattern5;
			break;
				
		case pattern7:	// r
			next_rotate = pattern8;
			break;
		case pattern8:
			next_rotate = pattern9;
			break;
		case pattern9:
			next_rotate = pattern10;
			break;
		case pattern10:
			next_rotate = pattern7;
			break;
			
		case pattern11:
			next_rotate = pattern12;
			break;
		case pattern12:
			next_rotate = pattern13;
			break;
		case pattern13:
			next_rotate = pattern14;
			break;
		case pattern14:
			next_rotate = pattern11;
			break;
			
		case pattern15:	//T
			next_rotate = pattern16;
			break;
		case pattern16:
			next_rotate = pattern17;
			break;
		case pattern17:
			next_rotate = pattern18;
			break;
		case pattern18:
			next_rotate = pattern15;
			break;
			
	}
	return next_rotate;
}

//поворот фигур
void rotate(cell *s_cur, lines *first, boundary *b) {
	int i;
	cell arr[pattern_length];
	cell s_start[pattern_length];
	
	int next_rotate = next_rotate_pos(b->cur_pattern);

	show_hide_pattern(s_cur, b, ' ');	// hide
	

	
	pattern_lib(s_start, next_rotate);
	// отображение повернутой фигуры на том же месте, где она находилась
	// до вызова функции поворота
	for(i = 0; i < pattern_length; i++) {
		arr[i].x = s_start[i].x - s_start[1].x + s_cur[1].x;
		arr[i].y = s_start[i].y - s_start[1].y + s_cur[1].y;
	}
	
	b->cur_pattern = next_rotate;
	/*
	for(i = 0; i < pattern_length; i++) {
		if(i == 1) continue;
		if(arr[i].x == arr[1].x && arr[i].y > arr[1].y) {
			arr[i].x -= 1;
			arr[i].y -= 1;
		} else if(arr[i].x == arr[1].x && arr[i].y < arr[1].y) {
			arr[i].x += 1;
			arr[i].y += 1;
		} else if(arr[i].x < arr[1].x && arr[i].y == arr[1].y) {
			arr[i].x += 1;
			arr[i].y -= 1;
		} else if(arr[i].x > arr[1].x && arr[i].y == arr[1].y) {
			arr[i].x -= 1;
			arr[i].y += 1;
		} else if(arr[i].x > arr[1].x && arr[i].y < arr[1].y) {
			arr[i].x += 0;
			arr[i].y += 2;
		} else if(arr[i].x > arr[1].x && arr[i].y > arr[1].y) {
			arr[i].x -= 2;
			arr[i].y += 0;
		} else if(arr[i].x < arr[1].x && arr[i].y > arr[1].y) {
			arr[i].x += 0;
			arr[i].y -= 2;
		} else if(arr[i].x < arr[1].x && arr[i].y < arr[1].y) {
			arr[i].x += 2;
			arr[i].y += 0;
		}
	}
	*/
	check_boundary_rotate(arr, b);
	int ok = check_occupied_rotate(arr, first);
	if(ok) {
		for(i = 0; i < pattern_length; i++) {
			s_cur[i].x = arr[i].x;
			s_cur[i].y = arr[i].y;
		}
	}
	show_hide_pattern(s_cur, b, '*');	// show
}

void check_boundary_rotate(cell *arr, boundary *b) {
	int i, j;
	
	for(i = pattern_length - 1; i >= 0 ; i--) {
		if(arr[i].x < b->min_x) {
			for(j = 0; j < pattern_length; j++) {
				arr[j].x += 2;
			}
			break;
		} else if(arr[i].x == b->max_x) {
			for(j = 0; j < pattern_length; j++) {
				arr[j].x -= 1;
			}
			break;
		} else if(arr[i].x == b->min_x) {
			for(j = 0; j < pattern_length; j++) {
				arr[j].x += 1;
			}
			break;
		}
		
		if(arr[i].y > b->max_y) {
			for(j = 0; j < pattern_length; j++) {
				arr[j].y -= 2;
			}
			break;
		} else if(arr[i].y == b->max_y) {
			for(j = 0; j < pattern_length; j++) {
				arr[j].y -= 1;
			}
			break;
		}	
	}
}

int check_occupied_rotate(const cell *arr, lines *first) {
	int i;
	lines* current;
	for(i = 0; i < pattern_length; i++) {
		current = first;
		while(current) {
			if(current->data.y == arr[i].y 
				&& current->data.x == arr[i].x) {
				return 0;
				break;
			}
			current = current->next;
		}
	}		
	return 1;
}

//показать фигуру
void show_hide_pattern(const cell *s, boundary *b, char ch) {
	int i;
	for(i = 0; i < pattern_length; i++) {
		// отображаются только те части фигуры, которые находятся в границах
		// игрового стакана
		if(s[i].y <= b->min_y) {
			continue;
		}
		move(s[i].y, s[i].x);
		addch(ch);
	}
	refresh();
}

// move
void move_pattern(cell *s, lines **first, boundary *b, int dx, int dy) {
	/* закончила ли движение предыдущая фигура */
		/* если да, то создаем новую фигуру */
	int i, mv_down, fail, full_line;
	dx = check_left_right_mv(s, *first, b, dx, dy);
	mv_down = check_mv_down(s, *first, b, dx, dy);
	
	if(mv_down) {
		show_hide_pattern(s, b, ' ');	// hide
		for(i = 0; i < pattern_length; i++) {
			s[i].x += dx;
			s[i].y += dy;
		}
		show_hide_pattern(s, b, '*');	// show
	} else {
		add_to_occupied(s, first);	// , last
		// print_occupied(*first);
		fail = check_fail(*first, b);
		if(fail) {
			b->is_failed = 1;
		} else {
			int num_lines = 0;
			for(i = b->min_y; i < b->max_y; i++) {
				full_line = check_line(*first, i);
				if(full_line) {
					show_hide_occupied(*first, ' ');	// hide
					destroy_line(first, i);		// , last
					num_lines++;
					show_hide_occupied(*first, '*');	// show
				} 
			}
			if(num_lines) {
				b->score += increase_score(num_lines);
				print_score(b->min_x, b->score);
			}
			create_pattern(s, b);
		}
	}
}

void show_hide_occupied(lines *first, char ch) {
	while(first) {
		move(first->data.y, first->data.x);
		addch(ch);

		first = first->next;
	}	
	refresh();
}

int check_left_right_mv(cell *s, lines *first, boundary *b, int dx, int dy) {
	int i;
	lines *current;

	for(i = 0; i < pattern_length; i++) {
		if(b->min_x == (s[i].x + dx) || 
				b->max_x == (s[i].x + dx)) 
			return 0;
		
		current = first;
		while(current) {
			if(current->data.y == (s[i].y + dy) &&
				current->data.x == (s[i].x + dx)) 
				return 0;
			current = current->next;
		}
	}
		
	return dx;
}
//проверка можно ли двигаться дальше вниз
	/*  if can`t, than add cell coord of elements to list, that contains
		occupied cells of game field
	*/
int check_mv_down(cell *s, lines *first, boundary *b, int dx, int dy) {
	int i;
	lines *current;
	for(i = 0; i < pattern_length; i++) {
		if(b->max_y == (s[i].y + dy)) return 0;
		
		current = first;
		while(current) {
			if(current->data.y == (s[i].y + dy) && 
				dx == 0 && current->data.x == (s[i].x + dx)) 
				return 0;
			current = current->next;
		}
	}	
	return 1;
}

int check_fail(lines *first, boundary *b) {
	while(first) {
		if(first->data.y == b->min_y)
			return 1;
		first = first->next;
	}
	return 0;
}

/*add cell coord of elements to list, that contains
		occupied cells of game field	
*/

void add_to_occupied(cell *s, lines **first) {	// , lines **last

	lines *tmp, *current;
	lines *prev;
	int i;
	for(i = 0; i < pattern_length; i++) {	
		/* вставка слева если нужно в конец	*/

		prev = NULL;
		current = *first;
	
		while(current && current->data.y < s[i].y) {
			prev = current;
			current = current->next;
		}
		
		while(current && current->data.y == s[i].y && 
			  current->data.x < s[i].x) {
			prev = current;
			current = current->next;
		}
		
		tmp = malloc(sizeof(*tmp));
		tmp->data.x = s[i].x;
		tmp->data.y = s[i].y;
		
		// tmp->prev = NULL;
		tmp->next = current;
		
		// single linked list
		if(prev) {
			prev->next = tmp;
		} else {
			*first = tmp;
		}
		
		/*
		//  double linked list
		if(current == NULL) {
			tmp->prev = *last;
		} else {
			tmp->prev = current->prev;
		}
		
		if(tmp->prev == NULL) {
			*first = tmp;			
		} else {
			tmp->prev->next = tmp;
		}
		
		if(tmp->next == NULL) {
			*last = tmp;
		} else {					
			tmp->next->prev = tmp;
		}
		*/
	}
}

// check to destroy line
	/*  cell levels[]
		int len
	for(i = 22, i > 0; i--) {
		int occupied = 0;
		for(j = 0; j < 18; j++) {
			for(k = 0; k < len; k++) {
				if(levels[k].y == i && levels[k].x == j) {
					occupied++;
				}
				
			}
		}
	}
	*/

// is line ready to be destroyed
int check_line(lines *first, int n) {
	//int i;
	int occur = 0;
	
	// lines *current = first;
	// for(i = b->min_y + 1; i < b->max_y - 1; i++) {
		while(first && first->data.y < n) {
			first = first->next;
		}
		while(first && first->data.y == n) {
			occur++;
			first = first->next;
		}
		// move(n - 15, 0);
		// printw("occur=%d", occur);
		
		if(occur == WIDTH)
			return 1;
			
	return 0;
}	

//сгорание уровней
void destroy_line(lines **first, int n) {	// , lines **last
	lines *prev = NULL;
	lines *current = *first;
	while(current->data.y < n) {
		prev = current;
		current = current->next;
	}
	lines *tmp;
	while(current && current->data.y == n) {
		tmp = current;
		// single linked list
		if(!prev) {
			current = current->next;
			*first = current;
		} else if(prev && current) {
			current = current->next;
			prev->next = current;
		} else {
			prev->next = NULL;
			// *last = prev;
		}
		/* double linked list
		// if first
		if(tmp->prev) {
			tmp->prev->next = tmp->next;
		} else {
			*first = tmp->next;
		}	
		// if last
		if(tmp->next) {
			tmp->next->prev = tmp->prev;
		} else {
			*last = tmp->prev;
		}
		current = current->next;
		*/
		free(tmp);
	}
	
	current = *first;
	while(current && current->data.y < n) {
		current->data.y++;
		current = current->next;
	}
	
}

int increase_score(int n) {
	
	if(n == 1)
		return 100;
	else if(n == 2)
		return 300;
	else if(n == 3)
		return 800;
	
	return 1500;
	/*
	switch(n) {
		case 1:
			return 100;
		case 2:
			return 300;
		case 3:
			return 800;
		case 4:
			return 1500;
	}
	*/
}

void print_score(int x, int score) {
	move(0, x + SCORE_X_OFFSET);
	printw("%d", score);
}

void game_field(boundary *coord, int row, int col) {

	coord->min_x = col / 2 - WIDTH / 2 - 1;
	coord->min_y = row - HEIGH - 1;
	coord->max_x = col / 2 + WIDTH / 2;
	coord->max_y = row - 1;
	
	int i, j;
	
	for(i = 0; i < row; i++) {
		for(j = coord->min_x; j < col; j++) {
			move(i, j);
			addch(' ');
		}
	}
	
	for(i = coord->min_y; i < row; i++) {
		if(i == coord->min_y || i == coord->max_y) {
			for(j = coord->min_x; j <= coord->max_x; j++) {
				move(i, j);
				printw("#");
			}
		} else {
			move(i, coord->min_x);
			printw("#");
			move(i, coord->max_x);
			printw("#");
		}
		
	}
}

void game_info(boundary *b) {
	// game name
	move(1, b->min_x / 2 - GAME_NAME_LEN / 2);
	printw("TETRIS");
	
	move(16, 0);
	printw("To rotate pattern press <Space>");
	
	move(18, 0);
	printw("To quit press <q>");
	
	move(20, 0);
	printw("To pause a game press <Escape>");
	
	
	move(22, 0);
	printw("To start a new game press <Enter>");
	
	// next pattern
	move(5, 52);
	printw("Next");
	
	// score
	move(0, b->min_x);
	printw("Score: ");
	
}

void pause_mode(boundary *b) {
	int i;
	if(!b->is_failed) {
		b->is_paused = (b->is_paused) ? 0 : 1;
		if(b->is_paused) {
			move(pause_y, pause_x);
			printw("Pause");
		} else {
			for(i = 0; i < pause_msg_len; i++) {
				move(pause_y, pause_x + i);	
				addch(' ');
			}
		}
	}
}

void game_over() {
	move(pause_y, pause_x - GAME_OVER_OFFSET);
	printw("Game over");
}

void save(const cell *s, const lines *first, const boundary *b) {
	/*
		Create save_file if need
	*/
	int i, n;
	FILE *f;
	char *path = "/home/talry/firstprog/PracticeC/tetris/save.txt";
	f = fopen(path, "w");
	if(!f) {
	}
	n = fprintf(f, "%d\n", b->cur_pattern);
	if(n < 0) {
	}
	// cur figure position
	for(i = 0; i < pattern_length; i++) {
		n = fprintf(f, "%d, %d\n", s[i].x, s[i].y);
	}
	n = fprintf(f, "%d\n", b->next_pattern);
	if(n < 0) {
	}
	// lines
	while(first) {
		n = fprintf(f, "%d, %d\n", first->data.x, first->data.y);
		first = first->next;
	}
	fclose(f);
	
}

void print_test(int n) {
	move(10, 3);
	printw("%d", n);
}

void test_save(cell *s, lines *first, boundary *b) {
	/*
		Create save_file if need
	*/
	int i, n;
	FILE *f;
	char *path = "/home/talry/firstprog/PracticeC/tetris/test_save.txt";
	f = fopen(path, "w");
	if(!f) {
	}
	n = fprintf(f, "%d\n", b->cur_pattern);
	if(n < 0) {
	}
	// cur figure position
	for(i = 0; i < pattern_length; i++) {
		n = fprintf(f, "%d, %d\n", s[i].x, s[i].y);
	}
	n = fprintf(f, "%d\n", b->next_pattern);
	if(n < 0) {
	}
	// lines
	while(first) {
		n = fprintf(f, "%d, %d\n", first->data.x, first->data.y);
		first = first->next;
	}
	fclose(f);
	
}
void load_game(cell *s, lines **first, boundary *b) {
	int i, n, x, y;
	FILE *f;
	char *path = "/home/talry/firstprog/PracticeC/tetris/save.txt";
	f = fopen(path, "r");
	n = fscanf(f, "%d", &b->cur_pattern);
	for(i = 0; i < pattern_length; i++) {
		n = fscanf(f, "%d, %d", &s[i].x, &s[i].y);
	}
	n = fscanf(f, "%d", &b->next_pattern);
	// lines - add to end
	lines *last = NULL;
	n = 2;
	while(1) {
		n = fscanf(f, "%d, %d", &x, &y);
		if(n == 2) {
			lines *tmp = malloc(sizeof(*tmp));
			tmp->data.x = x;
			tmp->data.y = y;
			tmp->next = NULL;
			if(last) {
				last->next = tmp;
				last = last->next;
			} else {
				*first = last = tmp;
			}
		} else {
			break;
		}
	}
	test_save(s, *first, b);
	fclose(f);
}

int main() {
	// int dx = 0, dy = 1;
	int key;
	int run = 1;
	double speed = 0.5;
	int level = 1;
	lines lns;
	cell s [pattern_length];
	
	boundary coord;
	lines *first = NULL;
	// lines *last = NULL;
	
	struct timespec sl;
	sl.tv_sec = 0;
	sl.tv_nsec = 100000000;
	clock_t movt;
	
	
	
	initscr();

	noecho();
	nodelay(stdscr, 1);
	keypad(stdscr, 1);
	curs_set(0);
	
	srandom(time(NULL));
	// getmaxyx(stdscr, row, col);
	
	init(s, &first, &coord);
	
	movt = clock();
	
	while(run) {
		key = getch();
		if(coord.score / (level * 2000) > 0) {	// 15 000 or 20 000
			level++;
			speed -= 0.02;
			move(10, 0);
			printw("level = %d, speed = %lf", level, speed);
		}
		if(coord.is_paused == 0) {
			switch(key) {
				case KEY_LEFT:
					// dx = -1;
					// dy = 0;
					move_pattern(s, &first, &coord, -1, 0);	//  &last, 
					break;
				case KEY_RIGHT:
					// dx = 1;
					// dy = 0;
					move_pattern(s, &first, &coord, 1, 0);	//  &last, 
					break;
				case KEY_DOWN:
					// dx = 0;
					// dy = 1;
					move_pattern(s, &first, &coord, 0, 1);	//  &last, 
					break;
				case key_rotate:
					rotate(s, first, &coord);
					break;
			}	
		}	
		switch(key) {	
			case key_exit1:
				run = 0;
				break;
			case key_exit2:
				run = 0;
				break;
				
			case key_pause:
				pause_mode(&coord);
				break;
			case key_save:
				save(s, first, &coord);
				break;
			case key_new_game:
				init(s, &first, &coord);
				break;
		}
		if(!coord.is_failed) {
			if((clock() - movt) >= CPS * speed) {
				if(!coord.is_paused) {
					move_pattern(s, &first, &coord, 0, 1);	//  &last, 
				} else {
					move_pattern(s, &first, &coord, 0, 0);	//  &last, 
					nanosleep(&sl, NULL);
				}
				movt += CPS * speed;
			}
		} else {
			game_over();
			nanosleep(&sl, NULL);
		}
		
	}
	
	endwin();
	
	return 0;
}

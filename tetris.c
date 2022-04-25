#include <curses.h>
#include <stdlib.h>
#include <time.h>

#define CPS CLOCKS_PER_SEC
#define MAX_FIGURE_LENGTH 5
#define WIDTH 18
enum { key_escape = 27 };
enum { fig0 = 0,
	   fig1 = 1, 
	   fig2 = 2, 
	   fig3 = 3, 
	   fig4 = 4, 
	   fig5 = 5,
	   fig6 = 6, 
	   fig7 = 7, 
	   fig8 = 8,  
};

typedef struct tag_cell {
	int x, y;
} cell;

typedef struct tag_lines {
	cell data;
	struct tag_lines *next, *prev;
} lines;


typedef struct tag_boundary {
	int min_x, max_x, min_y, max_y;
	bool is_failed;
} boundary;

void game_field(boundary *, int , int );
void create_figure(cell *);
void show_fig(cell *, int );
int check_left_right_mv(cell *, lines *, boundary *, int , int , int );
int check_mv_down(cell *, lines *, boundary *, int , int , int );
void stop_fig();
void add_to_occupied(cell *, lines **, lines **, int );
int check_line();
void destroy_line();
void print_occupied(lines *);

// static int z = 0;

void init(cell *figure, lines *first, lines *last, boundary *coord) {
	int row, col;
	getmaxyx(stdscr, row, col);
	
	game_field(coord, row, col);
	create_figure(figure);
	
	first = NULL;
	//last = NULL;
	// free lines
	
	
	
}

//библиотека стандартных фигур
void figure_lib(int n, cell *arr) {
	
	switch(n) {
		case fig0:
			// cell arr [2];
			arr[0].x = 39;	//	*
			arr[0].y = 1;	//	**
			arr[1].x = 39;	//	 *
			arr[1].y = 2;
			arr[2].x = 40;
			arr[2].y = 2;
			arr[3].x = 40;
			arr[3].y = 3;
			break;
		case fig1:
			arr[0].x = 39;	//	**
			arr[0].y = 1;	//	**
			arr[1].x = 40;	
			arr[1].y = 1;
			arr[2].x = 39;
			arr[2].y = 2;
			arr[3].x = 40;
			arr[3].y = 2;
	}
	// return arr;
}

int find_length(int n) {
	int length = -1;
	if(n == fig7 || n == fig8) length = 5;
	else length = 4;
	return length;
}
//создание новой фигуры
void create_figure(cell *fig) {
	int n = 1, length;
	
	// n = random_fig();
	length = find_length(n);
	
	// cell fig [len];
	figure_lib(n, fig);
	show_fig(fig, length);
	
	// return len;
}

//генератор фигур
int random_fig() {
	
	return -1;
}
//поворот фигур

//показать фигуру
void show_fig(cell *fig, int length) {
	int i;
	
	for(i = 0; i < length; i++) {
		move(fig[i].y, fig[i].x);
		printw("*");
	}
	refresh();
}

void hide_fig(cell *fig, int length) {
	int i;
	
	for(i = 0; i < length; i++) {
		move(fig[i].y, fig[i].x);
		printw(" ");
	}
	refresh();
}

// move
void move_figure(cell *figure, lines **first, lines **last, boundary *b, int length, int dx, int dy) {
	/* закончила ли движение предыдущая фигура */
		/* если да, то создаем новую фигуру */
	int i;
	dx = check_left_right_mv(figure, *first, b, length, dx, dy);

	if(check_mv_down(figure, *first, b, length, dx, dy)) {
		hide_fig(figure, length);
		for(i = 0; i < length; i++) {
			figure[i].x += dx;
			figure[i].y += dy;
		}
		show_fig(figure, length);
		//move(z, 0);
		//printw("tut %d", z);
		//z++;
	} else {
		// move(z, 0);
		// printw("zdes %d", z);
		// z++;
		//stop_fig();
		add_to_occupied(figure, first, last, length);
		// print_occupied(*first);
		if(check_line()) {
			destroy_line();
		} 
		create_figure(figure);
	}
	
	
	
}

void print_occupied(lines *first) {
	int i = 0;
	lines *cur = first;
	while(cur != NULL) {
		move(i, 0);
		printw("x=%d, y=%d",cur->data.x, cur->data.y);
		i++;
		cur = cur->next;
	}	
	refresh();
}


int check_left_right_mv(cell *figure, lines *first, boundary *b, int length, int dx, int dy) {
	int i;
	for(i = 0; i < length; i++) {
		if(b->min_x == (figure[i].x + dx) || 
				b->max_x == (figure[i].x + dx)) return 0;
		
		lines *current = first;
		while(current != NULL) {
			if(current->data.y == (figure[i].y + dy) &&
				current->data.x == (figure[i].x + dx)) return 0;
			current = current->next;
		}
	}
		
	return dx;
}
//проверка можно ли двигаться дальше вниз
	/*  if can`t, than add figure coord of elements to array, that contains
		occupied cells of game field
	*/
int check_mv_down(cell *figure, lines *first, boundary *b, int length, int dx, int dy) {
	int i;
	
	for(i = 0; i < length; i++) {
		if(b->max_y == (figure[i].y + dy)) return 0;
		
		lines *current = first;
		while(current != NULL) {
			if(current->data.y == (figure[i].y + dy) && 
				dx == 0 && current->data.x == (figure[i].x + dx)) return 0;
			current = current->next;
		}
	}	
	//move(z, 0);
	//printw("tuta1 %d", z);
	//z++;
	

	//move(z, 0);
	// printw("tut %d", z);
	//z++;
	
	return 1;
}
/*add figure coord of elements to list, that contains
		occupied cells of game field	
*/

void add_to_occupied(cell *figure, lines **first, lines **last, int length) {

	lines *tmp, *current;
	int i;
	for(i = 0; i < length; i++) {	
		/* вставка слева если нужно в конец
			y1=21	x1=39
			y2=21	x2=40
			y3=22	x3=39
			y4=22	x4=40
		*/

		current = *first;
	
		while(current != NULL && current->data.y < figure[i].y) {
			current = current->next;
		}
		
		while(current != NULL && current->data.x < figure[i].x) {
			current = current->next;
		}
		
		tmp = malloc(sizeof(*tmp));
		tmp->data.x = figure[i].x;
		tmp->data.y = figure[i].y;
		
		tmp->prev = NULL;
		tmp->next = current;
		
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
		
	}
}

void stop_fig() {
	
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
int check_line() {
	return -1;
}	

//сгорание уровней
void destroy_line() {

}

void game_field(boundary *coord, int row, int col) {

	coord->min_x = (col - 1) / 2 - WIDTH / 2;
	coord->min_y = 0;
	coord->max_x = col / 2 + WIDTH / 2;
	coord->max_y = row - 1;
	
	int i, j;
	
	for(i = 0; i < row; i++) {
		if(i == 0 || i == coord->max_y) {
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


int main() {
	
	int key;
	int run = 1;
	
	lines lns;
	cell figure [MAX_FIGURE_LENGTH];
	boundary coord;
	lines *first = NULL;
	lines *last = NULL;
	
	clock_t movt;
	
	
	
	initscr();

	noecho();
	nodelay(stdscr, 1);
	keypad(stdscr, 1);
	curs_set(0);
	
	
	// getmaxyx(stdscr, row, col);
	
	init(figure, first, last, &coord);
	
	movt = clock();
	
	while(run) {
		key = getch();
		switch(key) {
			case key_escape:
				run = 0;
				break;
			case KEY_LEFT:
				move_figure(figure, &first, &last, &coord, 4, -1, 0);
				break;
			case KEY_RIGHT:
				move_figure(figure, &first, &last, &coord, 4, 1, 0);
				break;
			case KEY_DOWN:
				move_figure(figure, &first, &last, &coord, 4, 0, 1);
				break;
		}
		
		if((clock() - movt) >= CPS * 0.5) {
			move_figure(figure, &first, &last, &coord, 4, 0, 1);
			movt += CPS * 0.5;
		}		
	}
	
	endwin();
	
	return 0;
}

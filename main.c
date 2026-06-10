/*
 * 2D ASCII Graphics Editor
 * Uses ncurses for TUI menus and rendering.
 * Canvas stored as a 2D char array.
 * Shapes: Circle, Rectangle, Line, Triangle
 * Operations: Add, Delete, Modify
 *
 * Compile: gcc graphics_editor.c -o graphics_editor -lncurses
 * Run    : ./graphics_editor
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ─── Canvas dimensions ─────────────────────────────────────────── */
#define ROWS        28
#define COLS        78
#define FILL_CH     '*'
#define BORDER_CH   '_'
#define EMPTY_CH    ' '

/* ─── Shape types ───────────────────────────────────────────────── */
#define SH_LINE     0
#define SH_RECT     1
#define SH_CIRCLE   2
#define SH_TRIANGLE 3
#define MAX_OBJECTS 64

/* ─── Color pairs ───────────────────────────────────────────────── */
#define CP_NORMAL   1
#define CP_FILL     2
#define CP_BORDER   3
#define CP_MENU     4
#define CP_TITLE    5
#define CP_SELECT   6
#define CP_STATUS   7

/* ─── Data structures ───────────────────────────────────────────── */
typedef struct {
    int type;
    int active;
    union {
        struct { int r1,c1,r2,c2; }         line;
        struct { int r1,c1,r2,c2; }         rect;
        struct { int cr,cc,radius; }         circle;
        struct { int r1,c1,r2,c2,r3,c3; }   triangle;
    };
} Shape;

static char canvas[ROWS][COLS+1];
static Shape objects[MAX_OBJECTS];
static int  obj_count = 0;

/* ─── Canvas utilities ──────────────────────────────────────────── */
void canvas_clear(void) {
    for (int r = 0; r < ROWS; r++) {
        memset(canvas[r], EMPTY_CH, COLS);
        canvas[r][COLS] = '\0';
    }
}

void canvas_set(int r, int c, char ch) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

/* ─── Drawing functions ─────────────────────────────────────────── */
void draw_line(int r1, int c1, int r2, int c2) {
    int dr = abs(r2-r1), dc = abs(c2-c1);
    int sr = (r1 < r2) ? 1 : -1;
    int sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;
    int r = r1, c = c1;
    for (;;) {
        canvas_set(r, c, FILL_CH);
        if (r == r2 && c == c2) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r += sr; }
        if (e2 <  dr) { err += dr; c += sc; }
    }
}

void draw_rect(int r1, int c1, int r2, int c2) {
    int rn = (r1<r2)?r1:r2, rx = (r1<r2)?r2:r1;
    int cn = (c1<c2)?c1:c2, cx = (c1<c2)?c2:c1;
    for (int c = cn; c <= cx; c++) {
        canvas_set(rn, c, BORDER_CH);
        canvas_set(rx, c, BORDER_CH);
    }
    for (int r = rn; r <= rx; r++) {
        canvas_set(r, cn, BORDER_CH);
        canvas_set(r, cx, BORDER_CH);
    }
}

void draw_circle(int cr, int cc, int radius) {
    int x = 0, y = radius, d = 3 - 2 * radius;
    while (y >= x) {
        canvas_set(cr+y, cc+x, FILL_CH); canvas_set(cr-y, cc+x, FILL_CH);
        canvas_set(cr+y, cc-x, FILL_CH); canvas_set(cr-y, cc-x, FILL_CH);
        canvas_set(cr+x, cc+y, FILL_CH); canvas_set(cr-x, cc+y, FILL_CH);
        canvas_set(cr+x, cc-y, FILL_CH); canvas_set(cr-x, cc-y, FILL_CH);
        x++;
        if (d > 0) { y--; d += 4*(x-y) + 10; }
        else         d += 4*x + 6;
    }
}

void draw_triangle(int r1,int c1,int r2,int c2,int r3,int c3) {
    draw_line(r1,c1,r2,c2);
    draw_line(r2,c2,r3,c3);
    draw_line(r3,c3,r1,c1);
}

/* ─── Render all objects → canvas ───────────────────────────────── */
void render_all(void) {
    canvas_clear();
    for (int i = 0; i < obj_count; i++) {
        if (!objects[i].active) continue;
        Shape *s = &objects[i];
        switch (s->type) {
            case SH_LINE:
                draw_line(s->line.r1,s->line.c1,s->line.r2,s->line.c2);
                break;
            case SH_RECT:
                draw_rect(s->rect.r1,s->rect.c1,s->rect.r2,s->rect.c2);
                break;
            case SH_CIRCLE:
                draw_circle(s->circle.cr,s->circle.cc,s->circle.radius);
                break;
            case SH_TRIANGLE:
                draw_triangle(s->triangle.r1,s->triangle.c1,
                              s->triangle.r2,s->triangle.c2,
                              s->triangle.r3,s->triangle.c3);
                break;
        }
    }
}

/* ─── Display canvas in a sub-window ───────────────────────────── */
void display_canvas(WINDOW *win, int start_y, int start_x) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            char ch = canvas[r][c];
            if (ch == FILL_CH) {
                wattron(win, COLOR_PAIR(CP_FILL) | A_BOLD);
                mvwaddch(win, start_y+r, start_x+c, ch);
                wattroff(win, COLOR_PAIR(CP_FILL) | A_BOLD);
            } else if (ch == BORDER_CH) {
                wattron(win, COLOR_PAIR(CP_BORDER) | A_BOLD);
                mvwaddch(win, start_y+r, start_x+c, ch);
                wattroff(win, COLOR_PAIR(CP_BORDER) | A_BOLD);
            } else {
                wattron(win, COLOR_PAIR(CP_NORMAL));
                mvwaddch(win, start_y+r, start_x+c, ch);
                wattroff(win, COLOR_PAIR(CP_NORMAL));
            }
        }
    }
}

/* ─── Helper: prompt for an integer in a popup ──────────────────── */
int ask_int(WINDOW *parent, const char *prompt, int lo, int hi, int def) {
    int pw = 44, ph = 5;
    int py = (LINES-ph)/2, px = (COLS+4-pw)/2;
    WINDOW *w = newwin(ph, pw, py, px);
    box(w, 0, 0);
    wattron(w, COLOR_PAIR(CP_TITLE));
    mvwprintw(w, 0, 2, " Input ");
    wattroff(w, COLOR_PAIR(CP_TITLE));
    mvwprintw(w, 1, 2, "%s [%d-%d] (def:%d):", prompt, lo, hi, def);
    mvwprintw(w, 2, 2, "> ");
    wrefresh(w);
    echo(); curs_set(1);
    char buf[16] = {0};
    mvwgetnstr(w, 2, 4, buf, 5);
    noecho(); curs_set(0);
    delwin(w);
    touchwin(parent); wrefresh(parent);
    int val = (buf[0] == '\0') ? def : atoi(buf);
    if (val < lo) val = lo;
    if (val > hi) val = hi;
    return val;
}

/* ─── Helper: message bar ───────────────────────────────────────── */
void status_msg(WINDOW *win, int row, const char *msg) {
    int w = getmaxx(win);
    wattron(win, COLOR_PAIR(CP_STATUS));
    mvwhline(win, row, 1, ' ', w-2);
    mvwprintw(win, row, 2, "%s", msg);
    wattroff(win, COLOR_PAIR(CP_STATUS));
    wrefresh(win);
}

/* ─── Add a new shape ───────────────────────────────────────────── */
void menu_add(WINDOW *win) {
    if (obj_count >= MAX_OBJECTS) {
        status_msg(win, LINES-2, "Error: max objects reached.");
        return;
    }
    /* choose shape type */
    int pw=30, ph=8, py=(LINES-ph)/2, px=(COLS+4-pw)/2;
    WINDOW *mw = newwin(ph, pw, py, px);
    box(mw,0,0);
    wattron(mw, COLOR_PAIR(CP_TITLE)|A_BOLD);
    mvwprintw(mw, 0, 2, " Select Shape ");
    wattroff(mw, COLOR_PAIR(CP_TITLE)|A_BOLD);
    const char *names[] = {"1. Line","2. Rectangle","3. Circle","4. Triangle"};
    for (int i=0;i<4;i++) mvwprintw(mw,2+i,4,"%s",names[i]);
    mvwprintw(mw,6,2,"Press 1-4 or q to cancel");
    wrefresh(mw);
    int ch = wgetch(mw);
    delwin(mw); touchwin(win); wrefresh(win);

    if (ch < '1' || ch > '4') return;
    int type = ch - '1';

    Shape s; memset(&s,0,sizeof(s));
    s.type = type; s.active = 1;

    switch (type) {
        case SH_LINE:
            s.line.r1 = ask_int(win,"R1 (start row)",    0, ROWS-1, 5);
            s.line.c1 = ask_int(win,"C1 (start col)",    0, COLS-1, 5);
            s.line.r2 = ask_int(win,"R2 (end row)",      0, ROWS-1,15);
            s.line.c2 = ask_int(win,"C2 (end col)",      0, COLS-1,40);
            break;
        case SH_RECT:
            s.rect.r1 = ask_int(win,"R1 (top row)",      0, ROWS-1, 3);
            s.rect.c1 = ask_int(win,"C1 (left col)",     0, COLS-1, 5);
            s.rect.r2 = ask_int(win,"R2 (bottom row)",   0, ROWS-1,14);
            s.rect.c2 = ask_int(win,"C2 (right col)",    0, COLS-1,30);
            break;
        case SH_CIRCLE:
            s.circle.cr     = ask_int(win,"Center row",  0, ROWS-1,13);
            s.circle.cc     = ask_int(win,"Center col",  0, COLS-1,25);
            s.circle.radius = ask_int(win,"Radius",      1, 12,     8);
            break;
        case SH_TRIANGLE:
            s.triangle.r1 = ask_int(win,"R1 (vertex 1 row)",0,ROWS-1, 2);
            s.triangle.c1 = ask_int(win,"C1 (vertex 1 col)",0,COLS-1,25);
            s.triangle.r2 = ask_int(win,"R2 (vertex 2 row)",0,ROWS-1,16);
            s.triangle.c2 = ask_int(win,"C2 (vertex 2 col)",0,COLS-1, 5);
            s.triangle.r3 = ask_int(win,"R3 (vertex 3 row)",0,ROWS-1,16);
            s.triangle.c3 = ask_int(win,"C3 (vertex 3 col)",0,COLS-1,45);
            break;
    }
    objects[obj_count++] = s;
    render_all();
    status_msg(win, LINES-2, "Shape added successfully.");
}

/* ─── List objects picker (returns index or -1) ─────────────────── */
int pick_object(WINDOW *parent, const char *title) {
    int active_count = 0;
    int idx[MAX_OBJECTS];
    for (int i=0;i<obj_count;i++) if(objects[i].active) idx[active_count++]=i;
    if (active_count == 0) {
        status_msg(parent, LINES-2, "No objects on canvas.");
        return -1;
    }
    int pw=54, ph=active_count+4, py=(LINES-ph)/2, px=(COLS+4-pw)/2;
    if (ph > LINES-2) ph = LINES-2;
    WINDOW *lw = newwin(ph, pw, py, px);
    box(lw,0,0);
    wattron(lw, COLOR_PAIR(CP_TITLE)|A_BOLD);
    mvwprintw(lw,0,2," %s ",title);
    wattroff(lw, COLOR_PAIR(CP_TITLE)|A_BOLD);

    static const char *tnames[] = {"LINE","RECT","CIRCLE","TRIANGLE"};
    for (int i=0;i<active_count;i++) {
        Shape *s = &objects[idx[i]];
        char desc[48];
        switch (s->type) {
            case SH_LINE:
                snprintf(desc,sizeof(desc),"(%d,%d)->(%d,%d)",
                    s->line.r1,s->line.c1,s->line.r2,s->line.c2); break;
            case SH_RECT:
                snprintf(desc,sizeof(desc),"(%d,%d)->(%d,%d)",
                    s->rect.r1,s->rect.c1,s->rect.r2,s->rect.c2); break;
            case SH_CIRCLE:
                snprintf(desc,sizeof(desc),"ctr(%d,%d) r=%d",
                    s->circle.cr,s->circle.cc,s->circle.radius); break;
            case SH_TRIANGLE:
                snprintf(desc,sizeof(desc),"(%d,%d)(%d,%d)(%d,%d)",
                    s->triangle.r1,s->triangle.c1,s->triangle.r2,
                    s->triangle.c2,s->triangle.r3,s->triangle.c3); break;
        }
        mvwprintw(lw,2+i,2,"[%d] %-8s %s", i+1, tnames[s->type], desc);
    }
    mvwprintw(lw,ph-2,2,"Enter number (1-%d) or 0 to cancel:", active_count);
    wrefresh(lw);

    echo(); curs_set(1);
    char buf[8]={0};
    mvwgetnstr(lw, ph-2, 38, buf, 3);
    noecho(); curs_set(0);
    delwin(lw); touchwin(parent); wrefresh(parent);

    int sel = atoi(buf);
    if (sel < 1 || sel > active_count) return -1;
    return idx[sel-1];
}

/* ─── Delete a shape ────────────────────────────────────────────── */
void menu_delete(WINDOW *win) {
    int i = pick_object(win, "Delete Shape");
    if (i < 0) return;
    objects[i].active = 0;
    render_all();
    status_msg(win, LINES-2, "Shape deleted.");
}

/* ─── Modify a shape ────────────────────────────────────────────── */
void menu_modify(WINDOW *win) {
    int i = pick_object(win, "Modify Shape");
    if (i < 0) return;
    Shape *s = &objects[i];
    switch (s->type) {
        case SH_LINE:
            s->line.r1 = ask_int(win,"R1", 0, ROWS-1, s->line.r1);
            s->line.c1 = ask_int(win,"C1", 0, COLS-1, s->line.c1);
            s->line.r2 = ask_int(win,"R2", 0, ROWS-1, s->line.r2);
            s->line.c2 = ask_int(win,"C2", 0, COLS-1, s->line.c2);
            break;
        case SH_RECT:
            s->rect.r1 = ask_int(win,"R1 (top)",    0, ROWS-1, s->rect.r1);
            s->rect.c1 = ask_int(win,"C1 (left)",   0, COLS-1, s->rect.c1);
            s->rect.r2 = ask_int(win,"R2 (bottom)", 0, ROWS-1, s->rect.r2);
            s->rect.c2 = ask_int(win,"C2 (right)",  0, COLS-1, s->rect.c2);
            break;
        case SH_CIRCLE:
            s->circle.cr     = ask_int(win,"Center row", 0, ROWS-1, s->circle.cr);
            s->circle.cc     = ask_int(win,"Center col", 0, COLS-1, s->circle.cc);
            s->circle.radius = ask_int(win,"Radius",     1, 12,      s->circle.radius);
            break;
        case SH_TRIANGLE:
            s->triangle.r1 = ask_int(win,"R1",0,ROWS-1,s->triangle.r1);
            s->triangle.c1 = ask_int(win,"C1",0,COLS-1,s->triangle.c1);
            s->triangle.r2 = ask_int(win,"R2",0,ROWS-1,s->triangle.r2);
            s->triangle.c2 = ask_int(win,"C2",0,COLS-1,s->triangle.c2);
            s->triangle.r3 = ask_int(win,"R3",0,ROWS-1,s->triangle.r3);
            s->triangle.c3 = ask_int(win,"C3",0,COLS-1,s->triangle.c3);
            break;
    }
    render_all();
    status_msg(win, LINES-2, "Shape modified.");
}

/* ─── Draw the main UI frame ─────────────────────────────────────── */
void draw_ui(WINDOW *win) {
    wclear(win);
    /* title bar */
    wattron(win, COLOR_PAIR(CP_TITLE)|A_BOLD);
    mvwhline(win, 0, 0, ' ', COLS+4);
    mvwprintw(win, 0, 2, "  ASCII 2D GRAPHICS EDITOR  "
                         "[ * = fill  _ = border ]");
    wattroff(win, COLOR_PAIR(CP_TITLE)|A_BOLD);

    /* canvas border */
    wattron(win, COLOR_PAIR(CP_NORMAL));
    mvwprintw(win, 1, 1, "+");
    mvwhline(win, 1, 2, '-', COLS);
    mvwprintw(win, 1, COLS+2, "+");
    mvwprintw(win, ROWS+2, 1, "+");
    mvwhline(win, ROWS+2, 2, '-', COLS);
    mvwprintw(win, ROWS+2, COLS+2, "+");
    for (int r=0; r<ROWS; r++) {
        mvwaddch(win, 2+r, 1, '|');
        mvwaddch(win, 2+r, COLS+2, '|');
    }
    wattroff(win, COLOR_PAIR(CP_NORMAL));

    /* menu bar */
    wattron(win, COLOR_PAIR(CP_MENU)|A_BOLD);
    mvwprintw(win, ROWS+3, 2,
        " [A]dd   [D]elete   [M]odify   [C]lear   [Q]uit ");
    wattroff(win, COLOR_PAIR(CP_MENU)|A_BOLD);

    /* status bar */
    wattron(win, COLOR_PAIR(CP_STATUS));
    mvwhline(win, LINES-2, 1, ' ', COLS+2);
    mvwprintw(win, LINES-2, 2, "Objects: %d/%d  |  Ready", obj_count, MAX_OBJECTS);
    wattroff(win, COLOR_PAIR(CP_STATUS));

    /* canvas content */
    display_canvas(win, 2, 2);
    wrefresh(win);
}

/* ─── Main ───────────────────────────────────────────────────────── */
int main(void) {
    initscr();
    start_color();
    use_default_colors();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    /* color pairs */
    init_pair(CP_NORMAL,  COLOR_WHITE,   COLOR_BLACK);
    init_pair(CP_FILL,    COLOR_MAGENTA, COLOR_BLACK);
    init_pair(CP_BORDER,  COLOR_CYAN,    COLOR_BLACK);
    init_pair(CP_MENU,    COLOR_BLACK,   COLOR_CYAN);
    init_pair(CP_TITLE,   COLOR_BLACK,   COLOR_MAGENTA);
    init_pair(CP_SELECT,  COLOR_BLACK,   COLOR_YELLOW);
    init_pair(CP_STATUS,  COLOR_WHITE,   236 > COLORS-1 ? COLOR_BLACK : 236);

    if (LINES < ROWS+6 || COLS < COLS+4) {
        endwin();
        fprintf(stderr,"Terminal too small. Need at least %dx%d.\n",COLS+4,ROWS+6);
        return 1;
    }

    WINDOW *win = newwin(LINES, COLS+4, 0, 0);
    canvas_clear();
    render_all();

    for (;;) {
        draw_ui(win);
        int ch = wgetch(win);
        switch (ch) {
            case 'a': case 'A': menu_add(win);    break;
            case 'd': case 'D': menu_delete(win); break;
            case 'm': case 'M': menu_modify(win); break;
            case 'c': case 'C':
                obj_count = 0;
                canvas_clear();
                render_all();
                status_msg(win, LINES-2, "Canvas cleared.");
                break;
            case 'q': case 'Q':
                delwin(win); endwin();
                return 0;
        }
    }
}

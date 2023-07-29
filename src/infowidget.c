#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <unistd.h>
#include <signal.h>

#include "drw.h"
#include "util.h"
#include "config.h"

#define LENGTH(X) (sizeof X / sizeof X[0])
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)))


static Display* dpy;
static int scr;
static Window root;
static Window win;
static XSetWindowAttributes xwa;

static Drw* drw;
static Clr* scheme[SchemeLast];


static int UPDATE_TIME = 2; /* in seconds */

static int x_pos;
static int y_pos;

static int WIDTH;
static int HEIGHT;

static char* command;

static int textsize;

static void redraw()
{
    /* clear screen of previous contents */
    drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

    /* put whatever you want to draw here */

    char text[32];
    sh(command, text, 32);

    int lpad = (WIDTH - TEXTW(text)) / 2;
    drw_paragraph(drw, lpad, (HEIGHT - textsize)/2, WIDTH, HEIGHT, text, textsize, textsize * 0.3);	

    /* you should only be changing this part and the WIDTH, HEIGHT and UPDATE_TIME */

    /* put the drawn things onto the window */
    drw_map(drw, win, 0, 0, WIDTH, HEIGHT);

}

/* updates the widget */
void sigalrm(int signum)
{
    XEvent exppp;

    memset(&exppp, 0, sizeof(exppp));
    exppp.type = Expose;
    exppp.xexpose.window = win;
    XSendEvent(dpy,win,False,ExposureMask,&exppp);
    XFlush(dpy);

    alarm(UPDATE_TIME);
}

/* visibility on */
void sigusr1(int signum)
{
    XMapWindow(dpy, win);
}

/* visibility off */
void sigusr2(int signum)
{
    XUnmapWindow(dpy, win);
}

int
main(int argc, char** argv)
{

    if(fork())
	return 0;
  
    /* if x and y pos are not provided, then throw error */
    if(argc != 10)
	die("usage: infowidget [xpos] [ypos] [width] [height] [override-redirect] [textsize] [color] [update-time] [command]");
	
    /* convert string arguments to integers, argv[0] is the command itself */
    x_pos = atoi(argv[1]);
    y_pos = atoi(argv[2]);
    WIDTH = atoi(argv[3]);
    HEIGHT = atoi(argv[4]);
    int override_redirect = atoi(argv[5]);
    textsize = atoi(argv[6]);
    int color = atoi(argv[7]);
    UPDATE_TIME = atoi(argv[8]);
    command = argv[9];

    /* set alarm signal to update the widget contents */
    signal(SIGALRM, sigalrm);
    signal(SIGUSR1, sigusr1);
    signal(SIGUSR2, sigusr2);

    /* obtain connection to X server */
    dpy = XOpenDisplay(NULL);
    if(dpy == NULL)
	die("Could not open display.");

    /* create window */
    scr = DefaultScreen(dpy);
    root = RootWindow(dpy, scr);

    /* set window attributes like override-redirect */
    xwa.background_pixel = BlackPixel(dpy, scr);
    xwa.override_redirect = override_redirect; /* This will tell the WM to ignore this window */
    xwa.backing_store = WhenMapped;

    win = XCreateWindow(dpy, root, x_pos, y_pos, WIDTH, HEIGHT, 0, DefaultDepth(dpy, scr), InputOutput, DefaultVisual(dpy, scr), CWBackPixel | CWOverrideRedirect, &xwa);


	
    /* tell X11 that we want to receive Expose events */
    XSelectInput(dpy, win, ExposureMask | ButtonPressMask);

    /* set class hint so dwm does not tile */
    XClassHint* class_hint = XAllocClassHint();
	
    /* make SURE to change the res_name to the name of your widget */
    class_hint->res_name = "infowidget";
    class_hint->res_class = "widget";

    XSetClassHint(dpy, win, class_hint);


    /* put window on the screen */
    XMapWindow(dpy, win);

    /* create drw object to draw stuff */
    drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

	
    char buf[64];
    snprintf(buf, 64, "iosevka:size=%d", textsize);

    const char* fonts[] = { buf };
    if(!drw_fontset_create(drw, fonts, LENGTH(fonts)))
	die("no fonts could be loaded.");

    /* initialize color schemes from config.h */
    for (int i=0;i<SchemeLast;i++)
	scheme[i] = drw_scm_create(drw, colors[i], 2);

    drw_setscheme(drw, scheme[color]);
	

    /* start updater thread */
    alarm(UPDATE_TIME);
    redraw();

    /* main loop */
    XEvent ev;
    while(XNextEvent(dpy, &ev) == 0){
	switch(ev.type){
	case Expose:
	    redraw();
	    break;
	}
    }

    /* close window and clean up */
    XUnmapWindow(dpy, win);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}

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


//#define WIDTH 250
//#define HEIGHT 160
#define UPDATE_TIME 3600 /* in seconds */

static int x_pos;
static int y_pos;

static int WIDTH;
static int HEIGHT;

static Fnt* huge;
static Fnt* big;
static Fnt* small;

static int textsize;

static char weather_condition[32];
static char temperature[8];
static char weather_symbol[8];

static int reload;

static void redraw()
{
    /* clear screen of previous contents */
    drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

    /* put whatever you want to draw here */

    if(reload){
		
	sh("curl 'wttr.in/Pune?format=\%C\\n'", weather_condition, 32);
	sh("curl 'wttr.in/Pune?format=%t\\n' | tr -d ' '", temperature, 8);
	sh("curl 'wttr.in/Pune?format=%c\\n' | tr -d ' '", weather_symbol, 8);
	reload = 0;
    }

    /* the correct font size must be set for each before getting width of text */
    drw_setfontset(drw, huge);
    int symbolw = TEXTW(weather_symbol);
    drw_setfontset(drw, big);
    int tempw = TEXTW(temperature);
    drw_setfontset(drw, small);
    int conditionw = TEXTW(weather_condition);

    int symbol_pad = (int)(textsize * 0.2857);
    int temp_pad = symbol_pad + symbolw + 10; 
    int condition_pad = (WIDTH - conditionw) / 2;

    int top_height = (int)(HEIGHT*0.625);

    drw_setfontset(drw, huge);
    drw_setscheme(drw, scheme[SchemeYellow]);
    drw_text(drw, symbol_pad, 0, WIDTH, top_height, 0, weather_symbol, 0);

    drw_setfontset(drw, big);
    drw_setscheme(drw, scheme[SchemePurple]);
    drw_text(drw, temp_pad, 0, WIDTH, top_height, 0, temperature, 0);

    drw_setfontset(drw, small);
    drw_setscheme(drw, scheme[SchemeAqua]);

    drw_text(drw, condition_pad, top_height, WIDTH, HEIGHT-top_height, 0, weather_condition, 0);

    /* you should only be changing this part and the WIDTH, HEIGHT and UPDATE_TIME */

    /* put the drawn things onto the window */
    drw_map(drw, win, 0, 0, WIDTH, HEIGHT);

}


void sigalrm(int signum)
{
    XEvent exppp;

    memset(&exppp, 0, sizeof(exppp));
    exppp.type = Expose;
    exppp.xexpose.window = win;
    XSendEvent(dpy,win,False,ExposureMask,&exppp);
    XFlush(dpy);

    alarm(UPDATE_TIME);
    reload = 1;
}

int
main(int argc, char** argv)
{

    /* if x and y pos are not provided, then throw error */
    if(argc != 7)
	die("usage: weather [xpos] [ypos] [width] [height] [override-redirect] [text-size]");
	
    /* convert string arguments to integers, argv[0] is the command itself */
    x_pos = atoi(argv[1]);
    y_pos = atoi(argv[2]);

    WIDTH = atoi(argv[3]);
    HEIGHT = atoi(argv[4]);

    int override_redirect = atoi(argv[5]);

    textsize = atoi(argv[6]);

    /* set alarm signal to update the widget contents */
    signal(SIGALRM, sigalrm);

    /* obtain connection to X server */
    dpy = XOpenDisplay(NULL);
    if(dpy == NULL)
	die("Could not open display.");

    /* create window */
    scr = DefaultScreen(dpy);
    root = RootWindow(dpy, scr);

    xwa.background_pixel = BlackPixel(dpy, scr);
    xwa.override_redirect = override_redirect; /* This will tell the WM to ignore this window */
    xwa.backing_store = WhenMapped;

    win = XCreateWindow(dpy, root, x_pos, y_pos, WIDTH, HEIGHT, 0, DefaultDepth(dpy, scr), InputOutput, DefaultVisual(dpy, scr), CWBackPixel | CWOverrideRedirect, &xwa);

    /* tell X11 that we want to receive Expose events */
    XSelectInput(dpy, win, ExposureMask);
	
    /* set class hint so dwm does not tile */
    XClassHint* class_hint = XAllocClassHint();
	
    class_hint->res_name = "weather";
    class_hint->res_class = "widget";

    XSetClassHint(dpy, win, class_hint);

    /* put window on the screen */
    XMapWindow(dpy, win);

    /* create drw object to draw stuff */
    drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

    char big_font_buf[64];
    snprintf(big_font_buf, 64, "iosevka:size=%d:style=Bold", textsize); //default is 35
    char small_font_buf[64];
    snprintf(small_font_buf, 64, "iosevka:size=%d", (int)(textsize * 0.4285));
    char huge_font_buf[64];
    snprintf(huge_font_buf, 64, "iosevka:size=%d:style=Bold", (int)(textsize * 1.8571));

    const char* big_font[] = { big_font_buf };
    const char* small_font[] = { small_font_buf };
    const char* huge_font[] = { huge_font_buf };
    
    big = drw_fontset_create(drw, big_font, 1);
    small = drw_fontset_create(drw, small_font, 1);
    huge = drw_fontset_create(drw, huge_font, 1);

    /* initialize color schemes from config.h */
    for (int i=0;i<SchemeLast;i++)
	scheme[i] = drw_scm_create(drw, colors[i], 2);


    /* fetch the weather and render it at startup */
    reload = 1;
    redraw(); 

    /* start updater thread */
    alarm(UPDATE_TIME);

    /* main loop */
    XEvent ev;
    while(XNextEvent(dpy, &ev) == 0)	{
	switch(ev.type)	{
	case Expose:
	    redraw();
	}
    }

    /* close window and clean up */
    XUnmapWindow(dpy, win);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}

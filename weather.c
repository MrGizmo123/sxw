#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <unistd.h>

#include "drw.h"
#include "util.h"
#include "config.h"

#define LENGTH(X) (sizeof X / sizeof X[0])
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)))


static Display* dpy;
static int scr;
static Window root;
static Window win;

static Drw* drw;
static Clr* scheme[SchemeLast];


#define WIDTH 250
#define HEIGHT 160
#define UPDATE_TIME 3600 /* in seconds */

static int x_pos;
static int y_pos;

static Fnt* huge;
static Fnt* big;
static Fnt* small;

static char weather_condition[32];
static char temperature[8];
static char weather_symbol[8];

static void redraw(int reload)
{
	/* clear screen of previous contents */
	drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

	/* put whatever you want to draw here */


	if(reload)
	{
		
		sh("curl 'wttr.in/Pune?format=\%C\\n'", weather_condition);
		sh("curl 'wttr.in/Pune?format=%t\\n'", temperature);
		sh("curl 'wttr.in/Pune?format=%c\\n'", weather_symbol);
		
	}

	/* the correct font size must be set for each before getting width of text */
	drw_setfontset(drw, huge);
	int symbolw = TEXTW(weather_symbol);
	drw_setfontset(drw, big);
	int tempw = TEXTW(temperature);
	drw_setfontset(drw, small);
	int conditionw = TEXTW(weather_condition);

	int symbol_pad = 10;
	int temp_pad = symbol_pad + symbolw + 10; 
	int condition_pad = (WIDTH - conditionw) / 2;

	drw_setfontset(drw, huge);
	drw_setscheme(drw, scheme[SchemeYellow]);
	drw_text(drw, symbol_pad, 0, WIDTH, 100, 0, weather_symbol, 0);

	drw_setfontset(drw, big);
	drw_setscheme(drw, scheme[SchemePurple]);
	drw_text(drw, temp_pad, 0, WIDTH, 100, 0, temperature, 0);

	drw_setfontset(drw, small);
	drw_setscheme(drw, scheme[SchemeAqua]);

	drw_text(drw, condition_pad, 100, WIDTH, 50, 0, weather_condition, 0);

	/* you should only be changing this part and the WIDTH, HEIGHT and UPDATE_TIME */

	/* put the drawn things onto the window */
	drw_map(drw, win, 0, 0, WIDTH, HEIGHT);

}

int
main(int argc, char** argv)
{

	/* if x and y pos are not provided, then throw error */
	if(argc < 3)
		die("not enough arguments, run with x_pos and y_pos as cmd line arguments");
	
	/* convert string arguments to integers, argv[0] is the command itself */
	x_pos = atoi(argv[1]);
	y_pos = atoi(argv[2]);

	/* obtain connection to X server */
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL)
		die("Could not open display.");

	/* create window */
	scr = DefaultScreen(dpy);
	root = RootWindow(dpy, scr);
	win = XCreateSimpleWindow(dpy, root, x_pos, y_pos, WIDTH, HEIGHT, 0, WhitePixel(dpy, scr), BlackPixel(dpy, scr));

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

	
	const char* big_font[] = {"iosevka:size=35:style=Bold"};
	const char* small_font[] = {"iosevka:size=25"};
	const char* huge_font[] = {"iosevka:size=65:style=Bold"};

	big = drw_fontset_create(drw, big_font, 1);
	small = drw_fontset_create(drw, small_font, 1);
	huge = drw_fontset_create(drw, huge_font, 1);

	/* initialize color schemes from config.h */
	for (int i=0;i<SchemeLast;i++)
		scheme[i] = drw_scm_create(drw, colors[i], 2);


	/* fetch the weather and render it at startup */
	redraw(1); 

	/* main loop */
	XEvent ev;
	unsigned long last_draw_time = (unsigned long)time(NULL);
	unsigned long current_time = last_draw_time;
	while(1)
	{
		/* if window is exposed then redraw */
		if(XCheckTypedEvent(dpy, Expose, &ev))
			redraw(0);

		/* redraw after 'UPDATE_TIME' seconds */
		current_time = (unsigned long)time(NULL);
		if((current_time - last_draw_time) > UPDATE_TIME)
		{
			redraw(1);
			last_draw_time = current_time;
		}

		/* sleep for 100000us i.e 100ms so that it does not consume all the cpu usage */
		usleep(100000);

	}


	/* close window and clean up */
	XUnmapWindow(dpy, win);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}

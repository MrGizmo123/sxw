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


#define UPDATE_TIME 2 /* in seconds */

static int x_pos;
static int y_pos;

static int WIDTH;
static int HEIGHT;

static char* command;
static char* text;

static void redraw()
{
  /* clear screen of previous contents */
  drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

  /* put whatever you want to draw here */

  int lpad = (WIDTH - TEXTW(text)) / 2;
  drw_text(drw, lpad, 0, WIDTH, HEIGHT, 0, text, 0);	

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
}

int
main(int argc, char** argv)
{

  if(fork())
    return 0;
  /* if x and y pos are not provided, then throw error */
  if(argc != 10)
    die("usage: iconbutton [xpos] [ypos] [width] [height] [override-redirect] [textsize] [text] [color] [command]");
	
  /* convert string arguments to integers, argv[0] is the command itself */
  x_pos = atoi(argv[1]);
  y_pos = atoi(argv[2]);
  WIDTH = atoi(argv[3]);
  HEIGHT = atoi(argv[4]);
  int override_redirect = atoi(argv[5]);
  int textsize = atoi(argv[6]);
  text = argv[7];
  int color = atoi(argv[8]);
  command = argv[9];

  /* set alarm signal to update the widget contents */
  signal(SIGALRM, sigalrm);

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
  class_hint->res_name = "icon_button";
  class_hint->res_class = "widget";

  XSetClassHint(dpy, win, class_hint);


  /* put window on the screen */
  XMapWindow(dpy, win);

  /* create drw object to draw stuff */
  drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

	
  char buf[64];
  snprintf(buf, 64, "Font Awesome 6:size=%d", textsize);

  const char* fonts[] = { buf };
  if(!drw_fontset_create(drw, fonts, LENGTH(fonts)))
    die("no fonts could be loaded.");

  /* initialize color schemes from config.h */
  for (int i=0;i<SchemeLast;i++)
    scheme[i] = drw_scm_create(drw, colors[i], 2);

  drw_setscheme(drw, scheme[color]);
	

  /* start updater thread */
  //alarm(UPDATE_TIME);
  //redraw();
  /* this does not require updater thread */

  /* main loop */
  XEvent ev;
  while(XNextEvent(dpy, &ev) == 0)
    {
      switch(ev.type)
	{
	case Expose:
	  redraw();
	  break;
	case ButtonPress:
	  if(ev.xbutton.button == Button1)
	    {
	      char buffer[16];
	      sh(command, buffer, 16);
	    }
	  break;
	}
    }

  /* close window and clean up */
  XUnmapWindow(dpy, win);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);

  return 0;
}

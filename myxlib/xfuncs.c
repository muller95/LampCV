#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

Display *theDisplay;
int theScreen;
int theDepth;
Visual *theVisual;
unsigned long theBlackPixel;
unsigned long theWhitePixel;
Colormap theColormap;

/*init and quit functions*/

void initX()
{
	if ((theDisplay = XOpenDisplay(NULL)) == NULL) {
		fprintf(stderr, "can't connect to xserver %s\n", XDisplayName(NULL));
		exit(1);
	}
	theScreen = DefaultScreen(theDisplay);
	theVisual = DefaultVisual(theDisplay, theScreen);
	theDepth = DefaultDepth(theDisplay, theScreen);
	theBlackPixel = BlackPixel(theDisplay, theScreen);
	theWhitePixel = WhitePixel(theDisplay, theScreen);
	theColormap = DefaultColormap(theDisplay, theScreen);

}

void getXinfo()
{
	printf("%s version %d of the X Window System, X%d R%d\n",
		ServerVendor(theDisplay),
		VendorRelease(theDisplay),
		ProtocolVersion(theDisplay),
		ProtocolRevision(theDisplay));
	printf("Color plane depth......................%d\n", theDepth);
	printf("Display width..........................%d\n", DisplayWidth(theDisplay, theScreen));
	printf("Display height.........................%d\n", DisplayHeight(theDisplay, theScreen));	
	printf("The display %s\n", XDisplayName((char *)theDisplay));
}

void quitX()
{
	XCloseDisplay(theDisplay);
}


/*window functions*/

int createGC(Window theNewWindow, GC *theNewGC)
{	
	*theNewGC = XCreateGC(theDisplay, theNewWindow, (unsigned long) 0, NULL);
	if (*theNewGC == 0) {
		return 0;
	} else {
		XSetForeground(theDisplay, *theNewGC, BlackPixel(theDisplay, theScreen));
		XSetBackground(theDisplay, *theNewGC, WhitePixel(theDisplay, theScreen));		
		return 1;	
	}
}

Window openWindow(int x, int y, int width, int height, int flag, GC *theNewGC)
{
	Window theNewWindow;
	XSizeHints sizehints;
	
	theNewWindow = XCreateSimpleWindow(theDisplay, XRootWindow(theDisplay, theScreen), x, y, width, height, 1, 
		BlackPixel(theDisplay, theScreen), WhitePixel(theDisplay, theScreen));

	if (createGC(theNewWindow, theNewGC) == 0) {
		XDestroyWindow(theDisplay, theNewWindow);
		fprintf(stderr, "Error creating graphics context\n");
		return (Window)0;
	}
	
	XSelectInput(theDisplay, theNewWindow, ExposureMask | ButtonPressMask);
	sizehints.x = x;
	sizehints.y = y;
	sizehints.width = width;
	sizehints.height = height;
	XSetWMNormalHints(theDisplay, theNewWindow, &sizehints);
	XMapWindow(theDisplay, theNewWindow);

	return theNewWindow;
}

void initX();
void getXinfo();
void quitX();
Window openWindow(int x, int y, int width, int height, int flag, GC* theNewGC);
/*void setColor(XImage *img, int x, int y, GC theGC, unsigned char r, unsigned char g, unsigned char b);*/
int createGC(Window theNewWindow, GC *theNewGC);

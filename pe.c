#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>



#define WIDTH 1000
#define HEIGHT WIDTH
#define NUMPOINTS 5000

typedef struct point {
	double x,y;
} point;

SDL_Window *initialise_window(int width, int height)
{
	SDL_Window *win;
	
	win = SDL_CreateWindow(
		"Polygon to Ellipse",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		0
		);

	if(win == NULL) {
		/* Window failed to be created; Print error and return NULL */
		printf("Failed to create window: %s\n", SDL_GetError());
		return NULL;
	}
	
	/* May have some other stuff here */
	
	return win;
}

/* Initialise points with random values between 0 and 1 */
void random_points(point *points, int npoints)
{
	int i;
	point *p;

	p=points;
	for(i=0;i<npoints;i++){
		p->x = ((double)rand())/((double)RAND_MAX);
		p->y = ((double)rand())/((double)RAND_MAX);
		p++;
	}
}

#define WEIGHT 0.5
double midpointiter(point *old, point *new, int npoints)
{
	point *p1, *p2, *np;
	int i;
	double l1;
	double l2;
	
	p2 = old + npoints - 1;
	p1 = old;
	np = new;
	l1 = l2 = 0;
	for(i=0;i<npoints;i++){
		point *onp;
		np->x = p1->x*WEIGHT + p2->x*(1-WEIGHT);
		np->y = p1->y*WEIGHT + p2->y*(1-WEIGHT);
		l1 += (p1->x - p2->x)*(p1->x - p2->x) + (p1->y - p2->y)*(p1->y - p2->y);
		p2 = p1++;
		onp = np++;
		if(i + 1 == npoints) np = new;
		l2 += (np->x - onp->x)*(np->x - onp->x) + (np->y - onp->y)*(np->y - onp->y);
	}
	return l2 / l1;
}

void scale(point *points, int npoints, double margin)
{
	int i;
	point *p;
	double minx, maxx, miny, maxy;

	p=points;
	minx = maxx = p->x;
	miny = maxy = p->y;
	p++;
	for(i=1; i<npoints; i++){
		if(p->x<minx) minx = p->x;
		else if(p->x>maxx) maxx = p->x;
		if(p->y<miny) miny = p->y;
		else if(p->y>maxy) maxy = p->y;
		p++;
	}
	p = points;
	for(i=0; i<npoints; i++){
	     p->x = ((p->x - minx) / (maxx - minx)) * (1 - 2*margin) + margin;
	     p->y = ((p->y - miny) / (maxy - miny)) * (1 - 2*margin) + margin;
	     p++;
	}
}

double iter(point **points, point **newpoints, int npoints)
{
	point *temp;
	double r;
	r = midpointiter(*points, *newpoints, npoints);
	scale(*newpoints, npoints, 0.1);
	temp = *points;
	*points = *newpoints;
	*newpoints = temp;
	return r;
}

void to_SDL_points(point *points, SDL_Point *spoints, int numpoints)
{
	int i;
	SDL_Point *sp;
	point *p;
	sp=spoints;
	p=points;
	for(i = 0; i < numpoints; i++){
	  sp->x = (int)(p->x*WIDTH);
	  sp->y = (int)(p->y*HEIGHT);
	  sp++;
	  p++;
	}
}

#define SPBUF 20
void render_poly(point *points, int numpoints, SDL_Renderer *ren)
{
	static int np = 0;
	static SDL_Point **spointsb = NULL;
	static int i = 0;
	SDL_Point *spoints;
	if(np != numpoints){
		np = numpoints;
		if(spointsb != NULL)
			for(i = 0; i < SPBUF; i++){
				free(spointsb[i]);
			}
		else
			spointsb = malloc(SPBUF*sizeof(*spointsb));
		for(i = 0; i < SPBUF; i++){
			spointsb[i] = malloc(np*sizeof(**spointsb));
		}
		i = 0;
	}
	spoints = spointsb[i];
	i = (i+1) % SPBUF;

	SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
	SDL_RenderClear(ren);
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

	to_SDL_points(points, spoints, numpoints);
	if(SDL_RenderDrawLines(ren, spoints, numpoints)<0)
		printf("draw failed %s\n", SDL_GetError());
	SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
	SDL_RenderDrawPoints(ren, spoints, numpoints);
	SDL_RenderPresent(ren);
}

int main()
{
	SDL_Window *window;
	SDL_Renderer *ren;
	SDL_Event event;

	point *points, *nextpoints;

	unsigned int ptime, ctime;

	int quit, event_ready;

	srand(time(NULL));

	/* Initialise SDL stuff */
	if(SDL_Init(SDL_INIT_EVERYTHING)){
		printf("SDL_Init Error: %s\n", SDL_GetError());
		goto exit;
	}
	window = initialise_window(WIDTH, HEIGHT);
	if(window == NULL){
		goto exit;
	}
	ren = SDL_CreateRenderer(window, -1, 0);
	if(ren == NULL){
		printf("SDL_CreateRenderer Error: %s", SDL_GetError());
		goto exit2;
	}

	/* Initialise data */
	points = malloc(NUMPOINTS*sizeof(*points));
	nextpoints = malloc(NUMPOINTS*sizeof(*points));
	random_points(points, NUMPOINTS);

	render_poly(points, NUMPOINTS, ren);

	ptime = SDL_GetTicks();
	quit = 0;
	while(!quit){
		event_ready = SDL_PollEvent(&event);
		if(!event_ready){
			iter(&points, &nextpoints, NUMPOINTS);
			iter(&points, &nextpoints, NUMPOINTS); /* iterate twice for smoothness */
			if((ctime = SDL_GetTicks()) - ptime > 16){
				render_poly(points, NUMPOINTS, ren);
				ptime = ctime;
			}
			continue;
		}
		switch(event.type){
		case SDL_QUIT:
			quit = 1;
			break;
		case SDL_KEYUP:
			random_points(points, NUMPOINTS);
			break;
		}
	}

exit3:
	SDL_DestroyRenderer(ren);
exit2:
	SDL_DestroyWindow(window);
exit:
	SDL_Quit();
}

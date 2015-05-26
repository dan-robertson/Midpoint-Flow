#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>



#define WIDTH 1000
#define HEIGHT WIDTH
#define NUMPOINTS 50

typedef struct point {
	double x,y;
} point;

typedef struct vec2 {
	double x,y;
} vec2;

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
#define RANDOM_SEGMENTS 5
void random_points(point *points, int npoints)
{
	int i,j;
	point *p, *p2, *q;

	p=points;
	for(i=0;i<npoints;i+=RANDOM_SEGMENTS){
		p->x = ((double)rand())/((double)RAND_MAX);
		p->y = ((double)rand())/((double)RAND_MAX);
		p+=RANDOM_SEGMENTS;
	}
	p=q=points;
	p2=points+RANDOM_SEGMENTS;
	for(i=0;i<npoints;i++){
	  if((j=i%RANDOM_SEGMENTS)==0){
		  p = p2;
		  if(p2 >= points+npoints)
		  	p2=points;
		  else
		  	p2+=RANDOM_SEGMENTS;
	  } else {
	  	double m;
		m=((double)j)/RANDOM_SEGMENTS;
		q->x = m*(p->x) + (1-m)*(p2->x);
		q->y = m*(p->y) + (1-m)*(p2->y);
	  }
	  q++;
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

vec2 diff(vec2 *l, vec2 *m, vec2 *r){
	vec2 ret;
	ret.x = 0.5*(l->x - r->x);
	ret.y = 0.5*(l->y - r->y);
	return ret;
}

vec2 curveature(point *p_2, point *p_1, point *p, point *p1, point *p2){
	vec2 u_1, u, u1, up;
	double um, uup;
	vec2 r;
	/*
       	u_1 = diff((vec2*)p_2, (vec2*)p_1, (vec2*)p);
	u   = diff((vec2*)p_1, (vec2*)p,   (vec2*)p1);
	u1  = diff((vec2*)p,   (vec2*)p1,  (vec2*)p2);
	
	up = diff(&u_1, &u, &u1);
	*/

	u.x = -0.3125*p_2->x - 0.85*p_1->x - 0.475*p->x +0.35*p1->x + 0.1875*p2->x;
	u.y = -0.3125*p_2->y - 0.85*p_1->y - 0.475*p->y +0.35*p1->y + 0.1875*p2->y;

	up.x = 2.5*p_2->x + 3*p_1->x - 4*p->x -3*p1->x +0.5*p2->x;
	up.y = 2.5*p_2->y + 3*p_1->y - 4*p->y -3*p1->y +0.5*p2->y;

	um = u.x*u.x + u.y*u.y;
	um = 1/um;
	uup = u.x*up.x + u.y*up.y;

	if(um<1000){
		r.x = um*(up.x - um*u.x*uup);
		r.y = um*(up.y - um*u.y*uup);
	} else {
	  r.x = r.y = 0;
	}
	return r;
}

#define CURVEWEIGHT 1e-6
void curvatureiter(point *old, point *new, int npoints){
	point *p_2, *p_1, *p, *p1, *p2;
	vec2 K;
	point *np;
	int i;
	
	p1 = old + npoints - 1;
	p = p1 - 1;
	p_1 = p - 1;
	p_2 = p_1 -1;
	p2 = old;
	np = new;

	for(i=0;i<npoints;i++){
		K = curveature(p_2,p_1,p,p1,p2);
		np->x = p->x + K.x*CURVEWEIGHT;
		np->y = p->y + K.y*CURVEWEIGHT;
		np++;
		p_2 = p_1;
		p_1 = p;
		p = p1;
		p1 = p2++;
	}
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
	/*r = midpointiter(*points, *newpoints, npoints);*/
	r=0;
	curvatureiter(*points, *newpoints, npoints);
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
	p=points;
	sp->x = (int)(p->x*WIDTH);
	sp->y = (int)(p->y*HEIGHT);
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
			spointsb[i] = malloc((np+1)*sizeof(**spointsb));
		}
		i = 0;
	}
	spoints = spointsb[i];
	i = (i+1) % SPBUF;

	SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
	SDL_RenderClear(ren);
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

	to_SDL_points(points, spoints, numpoints);
	if(SDL_RenderDrawLines(ren, spoints, numpoints+1)<0)
		printf("draw failed %s\n", SDL_GetError());
	SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
	SDL_RenderDrawPoints(ren, spoints, numpoints+1);
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
			/*iter(&points, &nextpoints, NUMPOINTS); /* iterate twice for smoothness */
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
		case SDL_KEYDOWN:
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

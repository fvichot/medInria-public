 /*
* This code is in part (the CONCAVE macro) a minimal variation 
* of the code from the Graphics Gems book series.
*
* For information on the Graphics Gems books and more of 
* the source code you can visit http://tog.acm.org/GraphicsGems/
*
* The following note on license is taken from the mentions website:
*
* EULA: The Graphics Gems code is copyright-protected. 
* In other words, you cannot claim the text of the code 
* as your own and resell it. Using the code is permitted 
* in any program, product, or library, non-commercial or 
* commercial. Giving credit is not required, though is 
* a nice gesture. The code comes as-is, and if there are 
* any flaws or problems with any Gems code, nobody involved 
* with Gems - authors, editors, publishers, or webmasters 
* - are to be held responsible. Basically, don't be a jerk, 
*   and remember that anything free comes with no guarantee.
*
*/

/*
* Concave Polygon Scan Conversion
* by Paul Heckbert
* from "Graphics Gems", Academic Press, 1990
*/

#include <stdio.h>
#include <math.h>
#include "GraphicsGems.h"

/*
* concave: scan convert nvert-sided concave non-simple polygon with vertices at
* (point[i].x, point[i].y) for i in [0..nvert-1] within the window win by
* calling spanproc for each visible span of pixels.
* Polygon can be clockwise or counterclockwise.
* Algorithm does uniform point sampling at pixel centers.
* Inside-outside test done by Jordan's rule: a point is considered inside if
* an emanating ray intersects the polygon an odd number of times.
* drawproc should fill in pixels from xl to xr inclusive on scanline y,
* e.g:
*	drawproc(y, xl, xr)
*	int y, xl, xr;
*	{
*	    int x;
*	    for (x=xl; x<=xr; x++)
*		pixel_write(x, y, pixelvalue);
*	}
*
*  Paul Heckbert	30 June 81, 18 Dec 89
*/

#define ALLOC(ptr, type, n)  ASSERT(ptr = (type *)malloc((n)*sizeof(type)))

typedef struct {		/* window: a discrete 2-D rectangle */
    int x0, y0;			/* xmin and ymin */
    int x1, y1;			/* xmax and ymax (inclusive) */
} Window;

typedef struct {		/* a polygon edge */
    double x;	/* x coordinate of edge's intersection with current scanline */
    double dx;	/* change in x with respect to y */
    int i;	/* edge number: edge i goes from pt[i] to pt[i+1] */
} Edge;

static int n;			/* number of vertices */
static Point2 *pt;		/* vertices */

static int nact;		/* number of active edges */
static Edge *active;		/* active edge list:edges crossing scanline y */

int compare_ind(), compare_active();

inline void drawproc(int y,int xl,int xr,Window * win,double * img,double value) // img same size as window
{
    int xlWin = xl-win->x0; // we put xr,xl and y in the coordinates of the window.
    int xrWin = xr-win->x0;
    int yWin = y-win->y0;
    int x;
    int gapY = win->y1 - win->y0; 
    for (x=xlWin; x <= xrWin ; x++)
        img[x*gapY+yWin]=value;
}

static void cdelete(int i)		/* remove edge i from active list */
{
    int j;

    for (j=0; j<nact && active[j].i!=i; j++);
    if (j>=nact) return;	/* edge not in active list; happens at win->y0*/
    nact--;
    //bcopy(&active[j+1], &active[j], (nact-j)*sizeof active[0]);
    memcpy(&active[j+1], &active[j], (nact-j)*sizeof active[0]);
}

static void cinsert(int i, int y)		/* append edge i to end of active list */
{
    int j;
    double dx;
    Point2 *p, *q;

    j = i<n-1 ? i+1 : 0;
    if (pt[i].y < pt[j].y) {p = &pt[i]; q = &pt[j];}
    else		   {p = &pt[j]; q = &pt[i];}
    /* initialize x position at intersection of edge with scanline y */
    active[nact].dx = dx = (q->x-p->x)/(q->y-p->y);
    active[nact].x = dx*(y+.5-p->y)+p->x;
    active[nact].i = i;
    nact++;
}

/* comparison routines for qsort */
int compare_ind(const void *arg1, const void * arg2)
{
    int* u = (int*)arg1;
    int* v = (int*)arg2;
    return pt[*u].y <= pt[*v].y ? -1 : 1;
}

int compare_active(const void *arg1,const void* arg2) 
{
    Edge* u = (Edge*)arg1;
    Edge* v = (Edge*)arg2;
    return u->x <= v->x ? -1 : 1;
}


//concave(nvert, point, win, spanproc)
//    int nvert;			/* number of vertices */
//Point2 *point;			/* vertices of polygon */
//Window *win;			/* screen clipping window */
static inline double * fillConcavePolygon(int nvert,Point2 *point,Window *win,double value)	
{
    int k, y0, y1, y, i, j, xl, xr;
    int *ind;		/* list of vertex indices, sorted by pt[ind[j]].y */
    const int nbPixels = (win->x1 - win->x0) * (win->y1 - win->y0) ;
    double * img = static_cast<double*>(calloc(nbPixels,sizeof(double)));
    n = nvert;
    pt = point;
    if (n<=0) return NULL;
    ALLOC(ind, int, n);
    ALLOC(active, Edge, n);

    /* create y-sorted array of indices ind[k] into vertex list */
    for (k=0; k<n; k++)
        ind[k] = k;
    qsort(ind, n, sizeof ind[0], compare_ind);	/* sort ind by pt[ind[k]].y */

    nact = 0;				/* start with empty active list */
    k = 0;				/* ind[k] is next vertex to process */
    y0 = MAX(win->y0, ceil(pt[ind[0]].y-.5));		/* ymin of polygon */
    y1 = MIN(win->y1, floor(pt[ind[n-1]].y-.5));	/* ymax of polygon */

    for (y=y0; y<=y1; y++) {		/* step through scanlines */
        /* scanline y is at y+.5 in continuous coordinates */

        /* check vertices between previous scanline and current one, if any */
        for (; k<n && pt[ind[k]].y<=y+.5; k++) {
            /* to simplify, if pt.y=y+.5, pretend it's above */
            /* invariant: y-.5 < pt[i].y <= y+.5 */
            i = ind[k];	
            /*
            * insert or delete edges before and after vertex i (i-1 to i,
            * and i to i+1) from active list if they cross scanline y
            */
            j = i>0 ? i-1 : n-1;	/* vertex previous to i */
            if (pt[j].y <= y-.5)	/* old edge, remove from active list */
                cdelete(j);
            else if (pt[j].y > y+.5)	/* new edge, add to active list */
                cinsert(j, y);
            j = i<n-1 ? i+1 : 0;	/* vertex next after i */
            if (pt[j].y <= y-.5)	/* old edge, remove from active list */
                cdelete(i);
            else if (pt[j].y > y+.5)	/* new edge, add to active list */
                cinsert(i, y);
        }

        /* sort active edge list by active[j].x */
        qsort(active, nact, sizeof active[0], compare_active);

        /* draw horizontal segments for scanline y */
        for (j=0; j<nact; j+=2) {	/* draw horizontal segments */
            /* span 'tween j & j+1 is inside, span tween j+1 & j+2 is outside */
            xl = ceil(active[j].x-.5);		/* left end of span */
            if (xl<win->x0) xl = win->x0;
            xr = floor(active[j+1].x-.5);	/* right end of span */
            if (xr>win->x1) xr = win->x1;
            if (xl<=xr)
                drawproc(y,xl,xr,win,img,value);		/* draw pixels in span */
            active[j].x += active[j].dx;	/* increment edge coords */
            active[j+1].x += active[j+1].dx;
        }
    }
    return img;
}

//#define MAXVERTICAL     100000
//struct Edge
//{
//    // osirix code
//    Edge *next;
//    long yTop, yBot;
//    long xNowWhole, xNowNum, xNowDen, xNowDir;
//    long xNowNumStep;
//};
//
//template <typename T> int sgn(T val) {
//    return (T(0) < val) - (val < T(0));
//}
//
//static inline void drawEdge( Point2 *p, int n,Edge *edgeTable[])
//{
//    memset( edgeTable, 0, sizeof(char*) * MAXVERTICAL);
//
//    for ( int i = 0; i < n; i++)
//        {
//        Point2 *p1, *p2, *p3;
//        Edge *e;
//        p1 = &p[ i];
//        p2 = &p[ (i + 1) % n];
//        if (p1->y == p2->y)
//            continue;   /* Skip horiz. edges */
//        /* Find next vertex not level with p2 */
//        for ( int j = (i + 2) % n; ; j = (j + 1) % n)
//                {
//            p3 = &p[ j];
//            if (p2->y != p3->y)
//                break;
//        }
//        e = static_cast<Edge*>(malloc( sizeof(Edge)));
//        e->xNowNumStep = abs(p1->x - p2->x);
//        if ( p2->y > p1->y)
//                {
//            e->yTop = p1->y;
//            e->yBot = p2->y;
//            e->xNowWhole = p1->x;
//            e->xNowDir = sgn(p2->x - p1->x);
//            e->xNowDen = e->yBot - e->yTop;
//            e->xNowNum = (e->xNowDen >> 1); //<=> e->xNowDen/2
//            if ( p3->y > p2->y)
//                e->yBot--;
//        }
//                else
//                {
//            e->yTop = p2->y;
//            e->yBot = p1->y;
//            e->xNowWhole = p2->x;
//            e->xNowDir = sgn((p1->x) - (p2->x));
//            e->xNowDen = e->yBot - e->yTop;
//            e->xNowNum = (e->xNowDen >> 1);
//            if ((p3->y) < (p2->y))
//                        {
//                e->yTop++;
//                e->xNowNum += e->xNowNumStep;
//                while (e->xNowNum >= e->xNowDen)
//                                {
//                    e->xNowWhole += e->xNowDir;
//                    e->xNowNum -= e->xNowDen;
//                }
//            }
//        }
//        e->next = edgeTable[ e->yTop];
//        edgeTable[ e->yTop] = e;
//    }
//}
//
///* DrawRuns first uses an insertion sort to order the X
// * coordinates of each active edge.  It updates the X coordinates
// * for each edge as it does this.
// * Then it draws a run between each pair of coordinates,
// * using the specified fill pattern.
// *
// * This routine is very slow and it would not be that
// * difficult to speed it way up.
// */
//static DCMPix **restoreImageCache = nil;
//
//static inline void DrawRuns( Edge *active,long curY,float *pix,long w,long h,float min,float max,BOOL outside,float newVal,BOOL addition,BOOL compute,float *imax,float *imin,long *count,
//    float *itotal,float *idev,float imean,long orientation,long stackNo,        // Only if X/Y orientation
//    BOOL restore)                
//{
//    long                        xCoords[ 4096];
//    float                        *curPix, val, temp;
//    long                        numCoords = 0;
//    long                        start, end, ims = w * h;
//
//    for ( Edge *e = active; e != NULL; e = e->next)
//    {
//        long i;
//        for ( i = numCoords; i > 0 &&
//            xCoords[i - 1] > e->xNowWhole; i--)
//            xCoords[i] = xCoords[i - 1];
//        xCoords[i] = e->xNowWhole;
//        numCoords++;
//        e->xNowNum += e->xNowNumStep;
//        while (e->xNowNum >= e->xNowDen)
//        {
//            e->xNowWhole += e->xNowDir;
//            e->xNowNum -= e->xNowDen;
//        }
//    }
//
//    if (numCoords % 2)  /* Protect from degenerate polygons */
//        xCoords[numCoords] = xCoords[numCoords - 1], numCoords++;
//
//    for ( long i = 0; i < numCoords; i += 2)
//    {
//        // ** COMPUTE
//        if( compute)
//        {
//            start = xCoords[i];                if( start < 0) start = 0;                if( start >= w) start = w;
//            end = xCoords[i + 1];        if( end < 0) end = 0;                        if( end >= w) end = w;
//
//            switch( orientation)
//            {
//            case 1:                curPix = &pix[ (curY * ims) + start + stackNo *w];                        break;
//            case 0:                curPix = &pix[ (curY * ims) + (start * w) + stackNo];                break;
//            case 2:                curPix = &pix[ (curY * w) + start];                                                        break;
//            }
//
//            long x = end - start;
//
//
//            while( x-- >= 0)
//            {
//                val = *curPix;
//
//                if( imax)
//                {
//                    if( val > *imax) *imax = val;
//                    if( val < *imin) *imin = val;
//
//                    *itotal += val;
//
//                    (*count)++;
//                }
//
//                if( idev)
//                {
//                    temp = imean - val;
//                    temp *= temp;
//                    *idev += temp;
//                }
//
//                if( orientation) curPix ++;
//                else curPix += w;
//            }
//
//        }
//
//        // ** DRAW
//        else
//        {
//            if( outside)        // OUTSIDE
//            {
//                if( i == 0)
//                {
//                    start = 0;                        if( start < 0) start = 0;                if( start >= w) start = w;
//                    end = xCoords[i];        if( end < 0) end = 0;                        if( end >= w) end = w;
//                    i--;
//                }
//                else
//                {
//                    start = xCoords[i]+1;                if( start < 0) start = 0;                if( start >= w) start = w;
//
//                    if( i == numCoords-1)
//                    {
//                        end = w;
//                    }
//                    else end = xCoords[i+1];
//
//                    if( end < 0) end = 0;                        if( end >= w) end = w;
//                }
//
//
//
//                switch( orientation)
//                {
//                case 1:                curPix = &pix[ (curY * ims) + start + stackNo *w];                break;
//                case 0:                curPix = &pix[ (curY * ims) + (start * w) + stackNo];                break;
//                case 2:                curPix = &pix[ (curY * w) + start];                                                        break;
//                }
//
//                long x = end - start;
//
//                if( addition)
//                {
//                    while( x-- > 0)
//                    {
//                        if( *curPix >= min && *curPix <= max) *curPix += newVal;
//
//                        if( orientation) curPix ++;
//                        else curPix += w;
//                    }
//                }
//                else
//                {
//                    while( x-- > 0)
//                    {
//                        if( *curPix >= min && *curPix <= max) *curPix = newVal;
//
//                        if( orientation) curPix ++;
//                        else curPix += w;
//                    }
//                }
//                long x = end - start;
//
//                while( x-- > 0)
//                {
//                    unsigned char*  rgbPtr = (unsigned char*) curPix;
//
//                    if( addition)
//                    {
//                        if( rgbPtr[ 1] >= min && rgbPtr[ 1] <= max) rgbPtr[ 1] += newVal;
//                        if( rgbPtr[ 2] >= min && rgbPtr[ 2] <= max) rgbPtr[ 2] += newVal;
//                        if( rgbPtr[ 3] >= min && rgbPtr[ 3] <= max) rgbPtr[ 3] += newVal;
//                    }
//                    else
//                    {
//                        if( rgbPtr[ 1] >= min && rgbPtr[ 1] <= max) rgbPtr[ 1] = newVal;
//                        if( rgbPtr[ 2] >= min && rgbPtr[ 2] <= max) rgbPtr[ 2] = newVal;
//                        if( rgbPtr[ 3] >= min && rgbPtr[ 3] <= max) rgbPtr[ 3] = newVal;
//                    }
//
//                    if( orientation) curPix ++;
//                    else curPix += w;
//                }
//            }
//        }
//    else                // INSIDE
//    {
//        float        *restorePtr = NULL;
//
//        start = xCoords[i];                if( start < 0) start = 0;                if( start >= w) start = w;
//        end = xCoords[i + 1];        if( end < 0) end = 0;                        if( end >= w) end = w;
//
//        switch( orientation)
//        {
//        case 0:                curPix = &pix[ (curY * ims) + (start * w) + stackNo];                if( restore && restoreImageCache) restorePtr = &[restoreImageCache[ curY] fImage][(start * w) + stackNo];                        break;
//        case 1:                curPix = &pix[ (curY * ims) + start + stackNo *w];                        if( restore && restoreImageCache) restorePtr = &[restoreImageCache[ curY] fImage][start + stackNo *w];                                break;
//        case 2:                curPix = &pix[ (curY * w) + start];                                                        if( restore && restoreImageCache) restorePtr = &[restoreImageCache[ stackNo] fImage][(curY * w) + start];                        break;
//        }
//
//        long x = end - start;
//
//        if( x >= 0)
//        {
//            if( restore && restoreImageCache)
//            {
//
//                if( orientation)
//                {
//                    while( x-- >= 0)
//                    {
//                        *curPix = *restorePtr;
//
//                        curPix ++;
//                        restorePtr ++;
//                    }
//                }
//                else
//                {
//                    while( x-- >= 0)
//                    {
//                        *curPix = *restorePtr;
//
//                        curPix += w;
//                        restorePtr += w;
//                    }
//                }
//
//            }
//            else
//            {
//                if( addition)
//                {
//                    while( x-- >= 0)
//                    {
//                        if( *curPix >= min && *curPix <= max) *curPix += newVal;
//
//                        if( orientation) curPix ++;
//                        else curPix += w;
//                    }
//                }
//                else
//                {
//                    while( x-- >= 0)
//                    {
//                        if( *curPix >= min && *curPix <= max) *curPix = newVal;
//
//                        if( orientation) curPix ++;
//                        else curPix += w;
//
//                    }
//                }
//            }
//        }
//        }
//    }
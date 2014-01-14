#ifndef __DATAVIZ_H
#define __DATAVIZ_H

#include "parser.h"

// constants

#define PI 3.14159265
#define FULL_ANGLE 6.28318531

// functions

int get_dimensions( struct tag *, int *, int *, int * );

int sector( int, char *, float, float, float, float, float, FILE * );
int arrow( float, float, float, float, FILE * );

int pie_chart( struct tag * );
int line_graph( struct tag * );
int bar_chart( struct tag * );

int read_file( const char * );

#endif

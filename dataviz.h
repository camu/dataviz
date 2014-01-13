#ifndef __DATAVIZ_H
#define __DATAVIZ_H

#include "parser.h"

// constants

#define PI 3.14159265
#define FULL_ANGLE 6.28318531

// functions

// svg functions
int sector( int, char *, float, float, float, float, float, FILE * );

// data visualisation functions
int pie_chart( struct tag *_taglist );

// dataviz functions
int read_file( const char * );

#endif

#ifndef __PARSER_H
#define __PARSER_H

#define PARSERBUFLEN 64

struct tag {
	int id;
        char *class;
        char *attr;
        char *val;
        struct tag *parent;
        struct tag *next;
};
int tag_struct_init( struct tag ** );
int tag_struct_set_class( const char *, int, struct tag ** );
int tag_struct_set_attr( const char *, int, struct tag ** );
int tag_struct_set_val( const char *, int, struct tag ** );
struct tag *tag_struct_push_back( struct tag ** );
int tag_struct_free( struct tag * );

char get_tag( FILE *, char ** );
char get_attr( FILE *, char ** );
int parse( FILE *, struct tag *, struct tag ** );

#endif

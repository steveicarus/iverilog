#ifndef VERSION
/*
 * Edit this definition in version_base.in to define the base version
 * number for the compiled result.
 */
# define VERSION_MAJOR1  0
# define VERSION_MAJOR2  9
# define VERSION_MINOR   6
# define VERSION_EXTRA   ""

/* This is a concatenation of MAJOR1.MAJOR2 that is used by
   vams_simparam.c to make a double value. */
# define VERSION_MAJOR   0.9

# define VERSION_STRINGIFY(x) #x
# define VERSION_STR(a,b,extra) VERSION_STRINGIFY(a.b) " " extra

#define VERSION VERSION_STR(VERSION_MAJOR,VERSION_MINOR,VERSION_EXTRA)
#endif

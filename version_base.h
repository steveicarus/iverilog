#ifndef VERSION
/*
 * Edit this definition in version_base.in to define the base version
 * number for the compiled result.
 */
# define VERSION_MAJOR  13
# define VERSION_MINOR   0

/*
 * This will be appended to the version. Use this to mark development
 * versions and the like.
 */
# define VERSION_EXTRA   " (devel)"

# define VERSION_STRINGIFY(x) #x
# define VERSION_STR(a,b,extra) VERSION_STRINGIFY(a.b) extra

#define VERSION VERSION_STR(VERSION_MAJOR,VERSION_MINOR,VERSION_EXTRA)
#endif

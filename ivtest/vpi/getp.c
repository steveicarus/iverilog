#include "veriuser.h"

static int
calltf(int ud, int reason)
{
    int i;
    PLI_BYTE8 *inst = tf_getinstance();

    (void)ud;  /* Parameter is not used. */
    (void)reason;  /* Parameter is not used. */

    for (i = 1; i < 5; i++) {
	io_printf("tf_getp(%d)\t\t-> %d\n", i, (int)tf_getp(i));
	io_printf("tf_igetp(%d,inst)\t-> %d\n", i, (int)tf_igetp(i,inst));
	io_printf("tf_getrealp(%d)\t\t-> %f\n", i, tf_getrealp(i));
	io_printf("tf_igetrealp(%d,inst)\t-> %f\n",
	    i, tf_igetrealp(i,inst));
    }

    return 0;
}

static int sizetf(int ud, int reason)
{
    (void)ud;  /* Parameter is not used. */
    (void)reason;  /* Parameter is not used. */
    return 32;
}

s_tfcell veriusertfs[2] = {
  {usertask, 0, 0, sizetf, calltf, 0, "$mytest", 1, 0, 0, {0} },
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0} }
};

static void veriusertfs_register(void)
{
      veriusertfs_register_table(veriusertfs);
}

void (*vlog_startup_routines[])(void) = { &veriusertfs_register, 0 };

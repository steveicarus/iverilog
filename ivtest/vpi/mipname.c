#include "veriuser.h"

static int
calltf(int ud, int reason)
{
    char *inst = tf_getinstance();

    (void)ud;  /* Parameter is not used. */
    (void)reason;  /* Parameter is not used. */

    io_printf("tf_mipname()\t\t-> %s\n", tf_mipname());
    io_printf("tf_imipname(inst)\t-> %s\n", tf_imipname(inst));

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

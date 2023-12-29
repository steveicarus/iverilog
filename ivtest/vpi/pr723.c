#include <assert.h>
#include "vpi_user.h"

static PLI_INT32 calltf(PLI_BYTE8 *data)
{
    int i;

    (void)data;  /* Parameter is not used. */

    for (i = 0; i < 31; i++) {
        if (vpi_mcd_name(1U<<i))
            vpi_printf("MCD %02d: %s\n", i+1, vpi_mcd_name(1U<<i));
    }
    for (i = 0; i < 33; i++) {
        if (vpi_mcd_name((1U<<31) | i))
            vpi_printf("FP  %02d: %s\n", i, vpi_mcd_name((1U<<31) | i));
    }

    return 0;
}

static void
VPIRegister(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type = vpiSysTask;
    tf_data.tfname = "$test";
    tf_data.calltf = calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf = 0;

    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = { VPIRegister, 0};

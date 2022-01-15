#include <stdio.h>
#include <string.h>
#include "vpi_user.h"

static void display_scope(vpiHandle scope)
{
    vpiHandle iter;
    vpiHandle item;

    iter = vpi_iterate(vpiReg, scope);
    if (iter) {
        while ( (item = vpi_scan(iter)) ) {
            vpi_printf("reg %s\n", vpi_get_str(vpiFullName, item));
        }
    }

    if (scope) {
        iter = vpi_iterate(vpiInternalScope, scope);
    } else {
        iter = vpi_iterate(vpiModule, NULL);
    }
    if (iter) {
        while ( (item = vpi_scan(iter)) ) {
            display_scope(item);
        }
    }
}

static PLI_INT32 list_regs_calltf(PLI_BYTE8*xx)
{
    (void)xx;  /* Parameter is not used. */

    display_scope(NULL);

    return 0;
}

static void list_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$list_regs";
    tf_data.calltf    = list_regs_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    tf_data.user_data = "$list_regs";
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
    list_register,
    0
};

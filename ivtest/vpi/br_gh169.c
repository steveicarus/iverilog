# include  <vpi_user.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

static void list_vars(vpiHandle scope)
{
    vpiHandle iter;
    vpiHandle item;

    iter = vpi_iterate(vpiNet, scope);
    if (iter) {
        while ( (item = vpi_scan(iter)) ) {
            vpi_printf("  wire %s\n", vpi_get_str(vpiFullName, item));
        }
    }

    iter = vpi_iterate(vpiReg, scope);
    if (iter) {
        while ( (item = vpi_scan(iter)) ) {
            vpi_printf("  reg  %s\n", vpi_get_str(vpiFullName, item));
        }
    }

    iter = vpi_iterate(vpiRealVar, scope);
    if (iter) {
        while ( (item = vpi_scan(iter)) ) {
            vpi_printf("  real  %s\n", vpi_get_str(vpiFullName, item));
        }
    }

    iter = vpi_iterate(vpiInternalScope, scope);
    if (iter) {
        while ( (item = vpi_scan(iter)) ) {
            vpi_printf("scope %s\n", vpi_get_str(vpiFullName, item));
            list_vars(item);
        }
    }
}

static PLI_INT32 list_vars_calltf(PLI_BYTE8*xx)
{
    vpiHandle iter = vpi_iterate(vpiModule, NULL);
    vpiHandle item;

    (void)xx;  /* Parameter is not used. */

    while ( (item = vpi_scan(iter)) ) {
        vpi_printf("scope %s\n", vpi_get_str(vpiName, item));
        list_vars(item);
    }
    return 0;
}

static void list_vars_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$list_vars";
    tf_data.calltf    = list_vars_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    tf_data.user_data = "$list_vars";
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
    list_vars_register,
    0
};

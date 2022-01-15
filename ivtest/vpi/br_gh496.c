# include  <sv_vpi_user.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

static PLI_INT32 list_packages_calltf(PLI_BYTE8*xx)
{
    vpiHandle iter = vpi_iterate(vpiPackage, NULL);
    vpiHandle item;

    (void)xx;  /* Parameter is not used. */

    while ( (item = vpi_scan(iter)) ) {
        vpi_printf("package %s\n", vpi_get_str(vpiName, item));
    }
    return 0;
}

static PLI_INT32 list_modules_calltf(PLI_BYTE8*xx)
{
    vpiHandle iter = vpi_iterate(vpiModule, NULL);
    vpiHandle item;

    (void)xx;  /* Parameter is not used. */

    while ( (item = vpi_scan(iter)) ) {
        vpi_printf("module %s\n", vpi_get_str(vpiName, item));
    }
    return 0;
}

static void list_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$list_packages";
    tf_data.calltf    = list_packages_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    tf_data.user_data = "$list_packages";
    vpi_register_systf(&tf_data);

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$list_modules";
    tf_data.calltf    = list_modules_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    tf_data.user_data = "$list_modules";
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
    list_register,
    0
};

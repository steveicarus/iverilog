#include "vpi_user.h"

static PLI_INT32 calltf(PLI_BYTE8 *data)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle mod = vpi_scan(argv);

    (void)data;  /* Parameter is not used. */

    vpi_free_object(argv);

    vpi_printf("Module instance %s is%s a cell.\n",
               vpi_get_str(vpiFullName, mod),
               vpi_get(vpiCellInstance, mod) ? "" : " not");
    return 0;
}

static void vpi_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type = vpiSysTask;
    tf_data.tfname = "$is_cell";
    tf_data.calltf = calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf = 0;

    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = {vpi_register, 0};

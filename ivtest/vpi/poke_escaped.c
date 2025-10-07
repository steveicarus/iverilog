#include "vpi_user.h"

static PLI_INT32 poke_escaped_calltf(PLI_BYTE8 *xx)
{
    vpiHandle scope;
    vpiHandle variable;
    s_vpi_value value;

    (void)xx;  /* Parameter is not used. */

    scope = vpi_handle(vpiScope, NULL);
    variable = vpi_handle_by_name("!\"escaped\"!\\", scope);
    if (variable == 0) {
        vpi_printf("FAILED: failed to find !\"escaped\"!\\ by name\n");
        vpi_sim_control(vpiFinish, 1);
        return 0;
    }

    value.format = vpiStringVal;
    value.value.str = "PASSED";

    vpi_put_value(variable, &value, 0, vpiNoDelay);

    return 0;
}

static void poke_escaped_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$poke_escaped";
    tf_data.calltf    = poke_escaped_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
    poke_escaped_register,
    0
};

#include <assert.h>
#include "vpi_user.h"

static PLI_INT32 check_val_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    (void) name; /* Not used */
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle item, index, value;
    s_vpi_value val;

    assert(argv);
    item = vpi_scan(argv);
    assert(item);
    index = vpi_scan(argv);
    assert(index);
    value = vpi_scan(argv);
    assert(value);
    vpi_free_object(argv);

    PLI_INT32 i_idx;
    val.format = vpiIntVal;
    vpi_get_value(index, &val);
    i_idx = val.value.integer;

    vpiHandle sel;
    sel = vpi_handle_by_index(item, i_idx);
    assert(sel);

    vpi_printf("The index is %d\n", vpi_get(vpiIndex, sel));
    vpi_printf("The type is %s\n", vpi_get_str(vpiType, sel));

    vpi_printf("%s ", vpi_get_str(vpiName, sel));

    val.format = vpiObjTypeVal;
    vpi_get_value(value, &val);

    if (val.format == vpiRealVal) {
	vpi_printf("=> %.2f == ", val.value.real);
	val.format = vpiRealVal;
	vpi_get_value(sel, &val);
	vpi_printf("%.2f\n", val.value.real);
    } else {
	val.format = vpiDecStrVal;
	vpi_get_value(value, &val);
	vpi_printf("=> %s == ", val.value.str);
	val.format = vpiDecStrVal;
	vpi_get_value(sel, &val);
	vpi_printf("%s\n", val.value.str);
    }

    return 0;
}
static PLI_INT32 put_val_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
    (void) name; /* Not used */
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    vpiHandle argv = vpi_iterate(vpiArgument, callh);
    vpiHandle item, index, value;
    s_vpi_value val;

    assert(argv);
    item = vpi_scan(argv);
    assert(item);
    index = vpi_scan(argv);
    assert(index);
    value = vpi_scan(argv);
    assert(value);
    vpi_free_object(argv);

    PLI_INT32 i_idx;
    val.format = vpiIntVal;
    vpi_get_value(index, &val);
    i_idx = val.value.integer;

    vpiHandle sel;
    sel = vpi_handle_by_index(item, i_idx);
    assert(sel);

    val.format = vpiObjTypeVal;
    vpi_get_value(value, &val);
    assert (val.format != vpiRealVal);
    val.format = vpiIntVal;
    vpi_get_value(value, &val);
    vpi_put_value(sel, &val, NULL, vpiNoDelay);

    return 0;
}

static void check_val_register(void)
{
    s_vpi_systf_data tf_data;

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$check_val";
    tf_data.calltf    = check_val_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    vpi_register_systf(&tf_data);

    tf_data.type      = vpiSysTask;
    tf_data.tfname    = "$put_val";
    tf_data.calltf    = put_val_calltf;
    tf_data.compiletf = 0;
    tf_data.sizetf    = 0;
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])(void) = {
    check_val_register,
    0
};

#include "vpi_user.h"

static void get_type(char *obj, vpiHandle scope)
{
    vpiHandle hand;

    vpi_printf("Looking for \"%s\": ",  obj);

    hand = vpi_handle_by_name(obj, scope);
    if (hand) vpi_printf("found \"%s\"\n", vpi_get_str(vpiName, hand));
    else vpi_printf("Not found\n");
}

static PLI_INT32 CompileTF(PLI_BYTE8 *x)
{
	(void)x;  /* Parameter is not used. */
        vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
        vpiHandle scope = vpi_handle(vpiScope, callh);

        get_type("\\esc.mod ", NULL);
        get_type("\\esc.port", scope);
        get_type("\\esc.port ", scope);
        get_type("\\esc.mod .\\esc.inm .\\esc.port", NULL);
        get_type("\\esc.mod .\\esc.inm .\\esc.port ", NULL);
        get_type("\\esc.val", scope);
        get_type("\\esc.val ", scope);
        get_type("\\esc.mod .\\esc.inm .\\esc.val", NULL);
        get_type("\\esc.mod .\\esc.inm .\\esc.val ", NULL);
        get_type("\\esc.mod .\\esc.inm .normal", NULL);
        get_type("\\esc.mod .inst.\\esc.id", NULL);
        get_type("\\esc.mod .inst.\\esc.id ", NULL);
        get_type("\\esc.mod .inst.normal", NULL);

	return 0;
}

static void my_Register(void)
{
	s_vpi_systf_data tf_data;

	tf_data.type = vpiSysTask;
	tf_data.tfname = "$print_if_found";
	tf_data.calltf = 0;
	tf_data.compiletf = CompileTF;
	tf_data.sizetf = 0;
	vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = {
my_Register, 0};

#include <stdio.h>
#include <string.h>
#include "vpi_user.h"

static void spaces(int num)
{
    while (num > 0) {
	vpi_printf(" ");
	num--;
    }
}

static void RecurseScope(vpiHandle handle, int depth)
{
    vpiHandle iter, hand;

    iter = !handle ? vpi_iterate(vpiModule, NULL) :
		     vpi_iterate(vpiInternalScope, handle);

    while (iter && (hand = vpi_scan(iter))) {
	spaces(depth);
	vpi_printf("%s is type ", vpi_get_str(vpiName, hand));
	switch (vpi_get(vpiType,hand)) {
	    case vpiModule: vpi_printf("vpiModule\n"); break;
	    case vpiTask: vpi_printf("vpiTask\n"); break;
	    case vpiFunction: vpi_printf("vpiFunction\n"); break;
	    case vpiNamedBegin: vpi_printf("vpiNamedBegin\n"); break;
	    case vpiNamedFork: vpi_printf("vpiNamedFork\n"); break;
	    default: vpi_printf("unknown (%d)\n", (int)vpi_get(vpiType,hand)); break;
	}
	RecurseScope(hand, depth + 2);
    }
}

static PLI_INT32 CompileTF(PLI_BYTE8 *x)
{
	(void)x;  /* Parameter is not used. */
	RecurseScope(NULL, 0);
	return 0;
}

static void my_Register(void)
{
	s_vpi_systf_data tf_data;

	tf_data.type = vpiSysTask;
	tf_data.tfname = "$test";
	tf_data.calltf = 0;
	tf_data.compiletf = CompileTF;
	tf_data.sizetf = 0;
	vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[]) (void) = {
my_Register, 0};

/* Read enum constants using explicit VPI value formats. */

#include <assert.h>
#include <string.h>
#include <sv_vpi_user.h>

static vpiHandle find_constant(vpiHandle scope, const char*name)
{
      vpiHandle types = vpi_iterate(vpiEnumTypespec, scope);
      vpiHandle type;

      while (types && (type = vpi_scan(types))) {
	    vpiHandle constants = vpi_iterate(vpiEnumConst, type);
	    vpiHandle item;
	    while (constants && (item = vpi_scan(constants))) {
		  if (strcmp(vpi_get_str(vpiName, item), name) == 0) {
			vpi_free_object(constants);
			vpi_free_object(types);
			return item;
		  }
	    }
      }
      return 0;
}

static PLI_INT32 read_constant(PLI_BYTE8*data)
{
      vpiHandle call = vpi_handle(vpiSysTfCall, 0);
      vpiHandle args = vpi_iterate(vpiArgument, call);
      s_vpi_value value;

      value.format = vpiStringVal;
      vpi_get_value(vpi_scan(args), &value);
      vpiHandle item = find_constant(vpi_handle(vpiScope, call), value.value.str);
      assert(item);
      vpi_free_object(args);

      value.format = *(PLI_INT32*)data;
      vpi_get_value(item, &value);
      if (value.format == vpiDecStrVal) value.format = vpiStringVal;
      vpi_put_value(call, &value, 0, vpiNoDelay);
      return 0;
}

static void register_functions(void)
{
      static struct {
	    const char*name;
	    PLI_INT32 format;
	    PLI_INT32 type;
      } functions[] = {
	    {"$enum_int", vpiIntVal, vpiIntFunc},
	    {"$enum_decimal", vpiDecStrVal, vpiStringFunc},
	    {"$enum_real", vpiRealVal, vpiRealFunc}
      };

      for (unsigned idx = 0 ; idx < sizeof(functions)/sizeof(functions[0]) ; idx += 1) {
	    s_vpi_systf_data tf = {0};
	    tf.type = vpiSysFunc;
	    tf.sysfunctype = functions[idx].type;
	    tf.tfname = functions[idx].name;
	    tf.calltf = read_constant;
	    tf.user_data = (PLI_BYTE8*)&functions[idx].format;
	    vpi_register_systf(&tf);
      }
}

void (*vlog_startup_routines[])(void) = {register_functions, 0};

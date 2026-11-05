/* Equal-width enums must retain their own literal mappings. */
#include <assert.h>
#include <string.h>
#include <sv_vpi_user.h>

static PLI_INT32 check_typespec(PLI_BYTE8*data)
{
      static const struct {
            const char*signal;
            const char*names[2];
            int values[2];
      } cases[] = {
            {"test.a", {"A", "B"}, {0, 5}},
            {"test.b", {"C", "D"}, {1, 4}},
            {"test.signed_logic", {"E", "F"}, {-2, 3}},
            {"test.signed_bit", {"G", "H"}, {-1, 2}},
            {"test.unsigned_bit", {"I", "J"}, {2, 6}},
            {"test.integer_enum", {"K", "L"}, {-3, 8}}
      };
      (void)data;
      for (unsigned idx = 0; idx < sizeof(cases)/sizeof(cases[0]); idx += 1) {
            vpiHandle signal = vpi_handle_by_name(cases[idx].signal, 0);
            assert(signal);
            vpiHandle spec = vpi_handle(vpiTypespec, signal);
            assert(spec && vpi_get(vpiType, spec) == vpiEnumTypespec);
            vpiHandle constants = vpi_iterate(vpiEnumConst, spec);
            assert(constants);
            for (unsigned n = 0; n < 2; n += 1) {
                  vpiHandle item = vpi_scan(constants);
                  assert(item);
                  assert(strcmp(vpi_get_str(vpiName, item), cases[idx].names[n]) == 0);
                  s_vpi_value value;
                  value.format = vpiIntVal;
                  vpi_get_value(item, &value);
                  assert(value.value.integer == cases[idx].values[n]);
            }
            assert(!vpi_scan(constants));
      }
      vpiHandle a = vpi_handle_by_name("test.a", 0);
      vpiHandle shared = vpi_handle_by_name("test.shared", 0);
      vpiHandle plain = vpi_handle_by_name("test.plain", 0);
      assert(shared && plain);
      assert(vpi_handle(vpiTypespec, a) == vpi_handle(vpiTypespec, shared));
      assert(!vpi_handle(vpiTypespec, plain));
      vpi_printf("PASSED\n");
      return 0;
}

static void register_test(void)
{
      s_vpi_systf_data tf = {0};
      tf.type = vpiSysTask;
      tf.tfname = "$check_enum_typespec";
      tf.calltf = check_typespec;
      vpi_register_systf(&tf);
}

void (*vlog_startup_routines[])(void) = {register_test, 0};

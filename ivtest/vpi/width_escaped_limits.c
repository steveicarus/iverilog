#include <vpi_user.h>
#include <string.h>
static void fail(const char *why)
{
    vpi_printf("FAIL WIDTH escaped audit: %s\n", why);
    vpi_control(vpiFinish, 1);
}
static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    (void)unused;
#ifdef vpiTchk
    static const char *owners[]={"tb.dotted","tb.slashed","tb.hierarchy"};
    static const char *refs[]={"CLK.dot","CLK\\tail","CLK"};
    static const unsigned limits[]={20,20,10};
    unsigned i;
    for (i=0;i<3;++i) {
        vpiHandle scope=vpi_handle_by_name((PLI_BYTE8 *)owners[i],NULL);
        vpiHandle iter,obj,term,expr;
        s_vpi_time val={0};
        s_vpi_delay delay={0};
        if (!scope) { fail("missing owner");return 0; }
        iter=vpi_iterate(vpiTchk,scope);
        if (!iter || !(obj=vpi_scan(iter))) { fail("missing object");return 0; }
        if (vpi_get(vpiTchkType,obj)!=vpiWidth || vpi_handle(vpiScope,obj)!=scope) {
            fail("wrong type/scope");return 0;
        }
        term=vpi_handle(vpiTchkRefTerm,obj);
        expr=term?vpi_handle(vpiExpr,term):NULL;
        if (!expr || strcmp(vpi_get_str(vpiName,expr),refs[i])) {
            fail("wrong decoded local name");return 0;
        }
        delay.da=&val;delay.no_of_delays=1;delay.time_type=vpiSimTime;
        vpi_get_delays(obj,&delay);
        if (val.high || val.low!=limits[i]) { fail("wrong annotation limit");return 0; }
        if (vpi_scan(iter)) { fail("extra object");return 0; }
    }
    vpi_printf("WIDTH_ESCAPED_LIMITS 20 20 10\n");
#else
    fail("typed WIDTH API unavailable");
#endif
    return 0;
}
static void register_all(void)
{
    s_vpi_systf_data tf={0};
    tf.type=vpiSysTask;tf.tfname="$width_escaped_limits";tf.calltf=audit;
    vpi_register_systf(&tf);
}
void pulse_history_register(void);
void (*vlog_startup_routines[])(void)={register_all,pulse_history_register,0};

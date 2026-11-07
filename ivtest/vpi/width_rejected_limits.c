/* Independent no-update and exact object-count check after rejected SDF. */
#include <vpi_user.h>
#include <string.h>
static void fail(const char *why)
{
    vpi_printf("FAIL WIDTH rejection audit: %s\n", why);
    vpi_control(vpiFinish, 1);
}
static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    (void)unused;
#ifdef vpiTchk
    static const char *names[] = {
        "tb.conditional", "tb.malformed", "tb.wrong_edge", "tb.overflow"
    };
    unsigned i;
    for (i=0; i<4; ++i) {
        vpiHandle scope=vpi_handle_by_name((PLI_BYTE8 *)names[i], NULL);
        vpiHandle iter, obj;
        s_vpi_time value={0};
        s_vpi_delay delay={0};
        if (!scope) { fail("missing owner"); return 0; }
        iter=vpi_iterate(vpiTchk, scope);
        if (!iter || !(obj=vpi_scan(iter))) {
            fail("missing WIDTH check"); return 0;
        }
        if (vpi_get(vpiTchkType,obj)!=vpiWidth ||
            vpi_handle(vpiScope,obj)!=scope) {
            fail("wrong object identity"); return 0;
        }
        delay.da=&value; delay.no_of_delays=1;
        delay.time_type=vpiSimTime;
        vpi_get_delays(obj,&delay);
        if (value.high || value.low!=10) {
            fail("rejected SDF altered WIDTH limit"); return 0;
        }
        if (vpi_scan(iter)) { fail("unexpected duplicate object"); return 0; }
    }
    vpi_printf("WIDTH_REJECTED_LIMITS 4 unchanged\n");
#else
    fail("typed WIDTH API unavailable");
#endif
    return 0;
}
static void register_all(void)
{
    s_vpi_systf_data tf={0};
    tf.type=vpiSysTask; tf.tfname="$width_rejected_limits"; tf.calltf=audit;
    vpi_register_systf(&tf);
}
void (*vlog_startup_routines[])(void)={register_all,0};

/* Independent object cardinality, binding, readback and timing callback audit. */
#include <vpi_user.h>
#include <stdio.h>
#include <string.h>

static void fail(const char *message)
{
    vpi_printf("FAIL WIDTH audit: %s\n", message);
    vpi_control(vpiFinish, 1);
}

#ifdef vpiTchk
static vpiHandle objects[4];
static unsigned seen[4];
static const unsigned counts[4] = {2, 3, 2, 1};
static const unsigned long times[4][3] = {
    {109,303,0}, {109,210,303}, {109,303,0}, {109,0,0}
};
static const double limits[4] = {10.0,20.0,10.0,10.0};

static unsigned long long now(void)
{
    s_vpi_time t = {0};
    t.type = vpiSimTime;
    vpi_get_time(NULL, &t);
    return ((unsigned long long)t.high << 32) | t.low;
}

static PLI_INT32 violation(p_cb_data cb)
{
    unsigned i;
    for (i=0; i<4; ++i) if (objects[i] == cb->obj) break;
    if (i == 4) { fail("callback object was not enumerated"); return 0; }
    if (seen[i] >= counts[i] || now() != times[i][seen[i]]) {
        fail("wrong/extra timing callback"); return 0;
    }
    vpi_printf("WIDTH_TCHK %u %llu\n", i, now());
    ++seen[i];
    return 0;
}

static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    static const char *names[3] = {"tb.same", "tb.omit", "tb.threshold"};
    static const unsigned expected[3] = {2,1,1};
    unsigned s;
    s_cb_data cb = {0};
    (void)unused;
    for (s=0; s<3; ++s) {
        vpiHandle scope = vpi_handle_by_name((PLI_BYTE8 *)names[s], NULL);
        vpiHandle iter, obj;
        unsigned count=0;
        if (!scope) { fail("missing module"); return 0; }
        iter=vpi_iterate(vpiTchk, scope);
        if (!iter) { fail("missing typed WIDTH objects"); return 0; }
        while ((obj=vpi_scan(iter))) {
            vpiHandle notifier, reference, expression;
            s_vpi_time value = {0};
            s_vpi_delay delay = {0};
            unsigned i;
            ++count;
            if (vpi_get(vpiType,obj) != vpiTchk ||
                vpi_get(vpiTchkType,obj) != vpiWidth ||
                vpi_handle(vpiScope,obj) != scope) {
                fail("type/scope binding"); return 0;
            }
            notifier=vpi_handle(vpiTchkNotifier,obj);
            if (s==0) {
                const char *name;
                if (!notifier) { fail("missing declared notifier"); return 0; }
                name=vpi_get_str(vpiName,notifier);
                if (!strcmp(name,"n10")) i=0;
                else if (!strcmp(name,"n20")) i=1;
                else { fail("wrong declared notifier"); return 0; }
            } else {
                if (notifier) { fail("fabricated optional notifier"); return 0; }
                i=s+1;
            }
            if (objects[i]) { fail("duplicate typed object"); return 0; }
            objects[i]=obj;
            reference=vpi_handle(vpiTchkRefTerm,obj);
            if (!reference || reference != vpi_handle(vpiTchkRefTerm,obj) ||
                vpi_get(vpiEdge,reference) != vpiPosedge) {
                fail("reference term cache/edge"); return 0;
            }
            expression=vpi_handle(vpiExpr,reference);
            if (!expression || strcmp(vpi_get_str(vpiName,expression),"CLK")) {
                fail("reference expression"); return 0;
            }
            delay.da=&value; delay.no_of_delays=1;
            delay.time_type=vpiScaledRealTime;
            vpi_get_delays(obj,&delay);
            if (value.real != limits[i]) { fail("scaled limit"); return 0; }
            memset(&value,0,sizeof(value));
            delay.time_type=vpiSimTime;
            vpi_get_delays(obj,&delay);
            if (value.high || value.low != (unsigned)limits[i]) {
                fail("integer limit"); return 0;
            }
        }
        if (count != expected[s]) { fail("object cardinality"); return 0; }
    }
    cb.reason=cbTchkViolation; cb.cb_rtn=violation;
    if (!vpi_register_cb(&cb)) fail("timing callback registration");
    vpi_printf("WIDTH_OBJECTS 4\n");
    return 0;
}

static PLI_INT32 finish(PLI_BYTE8 *unused)
{
    unsigned i;
    (void)unused;
    for (i=0; i<4; ++i) if (seen[i] != counts[i]) {
        fail("missing timing callbacks"); return 0;
    }
    vpi_printf("WIDTH_CALLBACKS 8\n");
    return 0;
}
#else
static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    (void)unused; fail("typed WIDTH API unavailable"); return 0;
}
static PLI_INT32 finish(PLI_BYTE8 *unused)
{
    (void)unused; fail("typed WIDTH API unavailable"); return 0;
}
#endif

static void register_all(void)
{
    s_vpi_systf_data tf = {0};
    tf.type=vpiSysTask; tf.tfname="$width_audit"; tf.calltf=audit;
    vpi_register_systf(&tf);
    tf.tfname="$width_audit_finish"; tf.calltf=finish;
    vpi_register_systf(&tf);
}
void (*vlog_startup_routines[])(void) = {register_all,0};

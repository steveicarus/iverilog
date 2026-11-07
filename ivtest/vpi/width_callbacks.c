/* Reuse the unchanged object audit; add persistent callback lifetime checks. */
/* Preserve the Windows export declaration before renaming the included entry. */
#include <vpi_user.h>

#define audit base_audit
#define finish base_finish
#define register_all base_register_all
#define vlog_startup_routines base_vlog_startup_routines
#include "width_objects.c"
#undef audit
#undef finish
#undef register_all
#undef vlog_startup_routines

#ifdef vpiTchk
static vpiHandle self_handle;
static unsigned filtered_seen, self_seen, removed_seen;

static PLI_INT32 filtered(p_cb_data cb)
{
    static const unsigned long expected[3] = {109,210,303};
    if (cb->obj != objects[1] || filtered_seen >= 3 ||
        now() != expected[filtered_seen]) {
        fail("object-filtered timing callback"); return 0;
    }
    ++filtered_seen;
    return 0;
}

static PLI_INT32 removed(p_cb_data unused)
{
    (void)unused;
    ++removed_seen;
    fail("removed timing callback fired");
    return 0;
}

static PLI_INT32 self_removing(p_cb_data cb)
{
    unsigned i;
    for (i=0; i<4; ++i) if (objects[i] == cb->obj) break;
    if (self_seen || i==4 || now()!=109) {
        fail("self-removing callback history"); return 0;
    }
    ++self_seen;
    if (!vpi_remove_cb(self_handle)) fail("self-removal failed");
    return 0;
}

static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    s_cb_data cb={0};
    vpiHandle handle;
    base_audit(unused);
    if (!objects[1]) { fail("supplemental binding unavailable"); return 0; }
    cb.reason=cbTchkViolation; cb.obj=objects[1]; cb.cb_rtn=filtered;
    if (!vpi_register_cb(&cb)) { fail("filter registration"); return 0; }
    cb.obj=NULL; cb.cb_rtn=removed;
    handle=vpi_register_cb(&cb);
    if (!handle || !vpi_remove_cb(handle)) { fail("pre-event removal"); return 0; }
    cb.cb_rtn=self_removing;
    self_handle=vpi_register_cb(&cb);
    if (!self_handle) fail("self-removal registration");
    return 0;
}

static PLI_INT32 finish(PLI_BYTE8 *unused)
{
    if (filtered_seen!=3 || self_seen!=1 || removed_seen!=0) {
        fail("supplemental callback totals"); return 0;
    }
    vpi_printf("WIDTH_CALLBACK_LIFETIME filter=3 self=1 removed=0\n");
    return base_finish(unused);
}
#else
static PLI_INT32 audit(PLI_BYTE8 *unused) { return base_audit(unused); }
static PLI_INT32 finish(PLI_BYTE8 *unused) { return base_finish(unused); }
#endif

static void register_all(void)
{
    s_vpi_systf_data tf={0};
    (void)base_register_all;
    tf.type=vpiSysTask; tf.tfname="$width_audit"; tf.calltf=audit;
    vpi_register_systf(&tf);
    tf.tfname="$width_audit_finish"; tf.calltf=finish;
    vpi_register_systf(&tf);
}
void (*vlog_startup_routines[])(void)={register_all,0};

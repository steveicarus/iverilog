/* Copyright (c) 2026 Jerry Zhong
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Observe the existing WIDTH/PERIOD history monitors from cbStartOfSimulation.
 * Expected values and times come from the monitor's literal parameters. */
#include <vpi_user.h>
#include <string.h>
#define MAX_MONITORS 64
#define MAX_CHANGES 16
struct monitor {
    vpiHandle scope, signal;
    PLI_INT32 before, initial, values[MAX_CHANGES];
    unsigned count, expected, index, completed;
    unsigned long long times[MAX_CHANGES], expected_times[2];
};
static struct monitor monitors[MAX_MONITORS];
static unsigned monitor_count;
static void fail(const char *why)
{
    vpi_printf("FAIL pulse startup history: %s\n",why);
    vpi_control(vpiFinish,1);
}
static PLI_INT32 scalar(vpiHandle h)
{
    s_vpi_value v={0};v.format=vpiScalarVal;vpi_get_value(h,&v);
    return v.value.scalar;
}
static unsigned long long now(void)
{
    s_vpi_time t={0};t.type=vpiSimTime;vpi_get_time(NULL,&t);
    return ((unsigned long long)t.high<<32)|t.low;
}
static PLI_INT32 changed(p_cb_data cb)
{
    s_vpi_time t={0};s_vpi_value v={0};
    PLI_INT32 reason=cb->reason;vpiHandle signal=cb->obj;
    unsigned index=*(unsigned *)cb->user_data;
    int has_time=cb->time!=NULL,has_value=cb->value!=NULL;
    struct monitor *m;
    if(has_time)t=*cb->time;
    if(has_value)v=*cb->value;
    if(reason!=cbValueChange||!has_time||t.type!=vpiSimTime||
       !has_value||v.format!=vpiScalarVal||index>=monitor_count) {
        fail("callback payload");return 0;
    }
    m=&monitors[index];
    if(signal!=m->signal||m->count==MAX_CHANGES||
       (((unsigned long long)t.high<<32)|t.low)!=now()||v.value.scalar!=scalar(signal)) {
        fail("callback identity/time/value");return 0;
    }
    m->values[m->count]=v.value.scalar;
    m->times[m->count++]=((unsigned long long)t.high<<32)|t.low;
    return 0;
}
static unsigned long long parameter(vpiHandle scope,const char *name)
{
    vpiHandle p=vpi_handle_by_name((PLI_BYTE8 *)name,scope);
    s_vpi_value v={0};v.format=vpiVectorVal;
    if(!p){fail("missing monitor parameter");return 0;}
    vpi_get_value(p,&v);
    if(v.value.vector[0].bval){fail("unknown monitor parameter");return 0;}
    return v.value.vector[0].aval;
}
static void scan_scope(vpiHandle scope)
{
    const char *type=vpi_get_str(vpiDefName,scope);
    vpiHandle iter,child;
    if(type && ((!strncmp(type,"width_history_check",19) &&
                 (type[19]==0||type[19]=='[')) ||
                (!strncmp(type,"period_history_check",20) &&
                 (type[20]==0||type[20]=='[')))) {
        struct monitor *m;s_cb_data cb={0};s_vpi_time t={0};s_vpi_value v={0};
        if(monitor_count==MAX_MONITORS){fail("too many monitors");return;}
        m=&monitors[monitor_count];m->scope=scope;m->index=monitor_count++;
        m->signal=vpi_handle_by_name("notifier",scope);
        if(!m->signal){fail("missing monitor notifier");return;}
        m->expected=(unsigned)parameter(scope,"COUNT");
        if(m->expected>2){fail("invalid monitor count");return;}
        m->expected_times[0]=parameter(scope,"T0");m->expected_times[1]=parameter(scope,"T1");
        m->initial=scalar(vpi_handle_by_name("INITIAL",scope));
        m->before=scalar(m->signal);
        t.type=vpiSimTime;v.format=vpiScalarVal;
        cb.reason=cbValueChange;cb.cb_rtn=changed;cb.obj=m->signal;
        cb.time=&t;cb.value=&v;cb.user_data=(PLI_BYTE8 *)&m->index;
        if(!vpi_register_cb(&cb)){fail("monitor callback registration");return;}
    }
    iter=vpi_iterate(vpiModule,scope);
    while(iter&&(child=vpi_scan(iter)))scan_scope(child);
}
static PLI_INT32 start(p_cb_data unused)
{
    vpiHandle iter=vpi_iterate(vpiModule,NULL),scope;(void)unused;
    while(iter&&(scope=vpi_scan(iter)))scan_scope(scope);
    return 0;
}
static PLI_INT32 complete(PLI_BYTE8 *unused)
{
    vpiHandle call=vpi_handle(vpiSysTfCall,NULL),scope=vpi_handle(vpiScope,call);
    vpiHandle args=vpi_iterate(vpiArgument,call),arg=args?vpi_scan(args):NULL;
    unsigned n,i,first=0,seen=0;PLI_INT32 previous;struct monitor *m;(void)unused;
    while(scope && vpi_get(vpiType,scope)!=vpiModule) scope=vpi_handle(vpiScope,scope);
    for(n=0;n<monitor_count;++n)if(monitors[n].scope==scope)break;
    if(n==monitor_count){fail("monitor was not observed from startup");return 0;}
    m=&monitors[n];
    if(!arg || vpi_scan(args) || scalar(arg)!=scalar(m->signal)) {
        fail("monitor argument/current value");return 0;
    }
    if(m->completed++){fail("duplicate monitor completion");return 0;}
    previous=m->initial;
    if(m->before!=m->initial) {
        if(!m->count||m->times[0]!=0||m->values[0]!=m->initial) {
            fail("declared initialization missing");return 0;
        }
        first=1;
    }
    for(i=first;i<m->count;++i) {
        PLI_INT32 value=m->values[i];
        if(seen>=m->expected||m->times[i]!=m->expected_times[seen]||
           (value!=vpi0&&value!=vpi1)||previous==vpiZ||
           (previous==vpi0&&value!=vpi1)||(previous==vpi1&&value!=vpi0)) {
            fail("extra/wrong notifier history");return 0;
        }
        previous=value;++seen;
    }
    if(seen!=m->expected||scalar(m->signal)!=previous)fail("missing notifier history/final value");
    return 0;
}
void pulse_history_register(void)
{
    s_cb_data cb={0};s_vpi_systf_data tf={0};
    cb.reason=cbStartOfSimulation;cb.cb_rtn=start;
    if(!vpi_register_cb(&cb))fail("startup registration");
    tf.type=vpiSysTask;tf.tfname="$pulse_history_complete";tf.calltf=complete;
    vpi_register_systf(&tf);
}

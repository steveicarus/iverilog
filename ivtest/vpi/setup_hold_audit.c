/* Copyright (c) 2026 Stephen Williams (steve@icarus.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Independent fixture expectations: identify both terminals and check actual
 * timing callbacks separately from notifier changes. No DUT decision is used
 * to calculate an expected violation time. Watch arguments include the declared
 * initial notifier scalar before the expected callback count and times. */
#include <vpi_user.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef AUDIT_LABEL
#define AUDIT_LABEL "SETUP/HOLD"
#define AUDIT_TAG "SH"
#define AUDIT_PREFIX "sh"
#define AUDIT_CHECK_TYPE(s) (!strcmp(s,"setup")?vpiSetup:!strcmp(s,"hold")?vpiHold:0)
#define AUDIT_LIMIT_COUNT(t) 1
#endif
#define MAX_CHECKS 64
#define MAX_EVENTS 32
static unsigned failed, started, finished, check_count, notifier_count;
#ifdef vpiTchk
struct watched_check {
    vpiHandle object;
    unsigned expected, seen;
    unsigned long long times[MAX_EVENTS];
};
struct watched_notifier {
    vpiHandle signal;
    unsigned expected, seen;
    unsigned long long times[MAX_CHECKS * MAX_EVENTS];
    PLI_INT32 previous, initial;
};
static struct watched_check checks[MAX_CHECKS];
static struct watched_notifier notifiers[MAX_CHECKS];
static unsigned notifier_indices[MAX_CHECKS];
/* Capture initialization before HDL initial blocks, while watches may wait for
 * a cold SDF annotation. The fixture still supplies the initial value. */
struct initial_notifier {
    vpiHandle signal;
    PLI_INT32 before, values[MAX_EVENTS];
    unsigned changes, index;
    unsigned long long times[MAX_EVENTS];
};
static struct initial_notifier initial_notifiers[MAX_CHECKS];
static unsigned initial_notifier_count, initial_violation_count;
static struct { vpiHandle object; unsigned long long time; }
    initial_violations[MAX_CHECKS * MAX_EVENTS];

#endif
static void fail(const char *why)
{
    failed = 1;
    vpi_printf("FAILED " AUDIT_LABEL " audit: %s\n", why);
    vpi_control(vpiFinish, 1);
}
static unsigned long long now(void)
{
    s_vpi_time t = {0}; t.type = vpiSimTime;
    vpi_get_time(NULL, &t);
    return ((unsigned long long)t.high << 32) | t.low;
}
static PLI_INT32 scalar(vpiHandle h)
{
    s_vpi_value v = {0}; v.format = vpiScalarVal;
    vpi_get_value(h, &v); return v.value.scalar;
}
#ifdef vpiTchk
static char *string_arg(vpiHandle iter)
{
    vpiHandle a = vpi_scan(iter); s_vpi_value v = {0};
    char *s; size_t n;
    if (!a) { fail("missing string argument"); return NULL; }
    v.format = vpiStringVal; vpi_get_value(a, &v);
    n = strlen(v.value.str)+1; s = (char *)malloc(n);
    if (!s) { fail("allocation"); return NULL; }
    memcpy(s, v.value.str, n); return s;
}
static double number_arg(vpiHandle iter)
{
    vpiHandle a = vpi_scan(iter); s_vpi_value v = {0};
    if (!a) { fail("missing numerical argument"); return 0; }
    v.format = vpiRealVal; vpi_get_value(a, &v); return v.value.real;
}
static PLI_INT32 scalar_arg(vpiHandle iter)
{
    vpiHandle a = vpi_scan(iter);
    if (!a) { fail("missing initial notifier value"); return vpiX; }
    return scalar(a);
}
static int edge_code(const char *s)
{
    if (!strcmp(s,"posedge")) return vpiPosedge;
    if (!strcmp(s,"negedge")) return vpiNegedge;
    if (!strcmp(s,"any")) return vpiNoEdge;
    fail("unknown fixture edge"); return -1;
}
static int terminal(vpiHandle obj, int role, vpiHandle scope,
                    const char *name, int edge)
{
    vpiHandle t = vpi_handle(role,obj), e;
    if (!t || t != vpi_handle(role,obj) || vpi_get(vpiEdge,t) != edge) return 0;
    e = vpi_handle(vpiExpr,t);
    return e && vpi_handle(vpiScope,e)==scope &&
           !strcmp(vpi_get_str(vpiName,e),name);
}
static PLI_INT32 initial_changed(p_cb_data cb)
{
    struct initial_notifier *n = &initial_notifiers[*(unsigned *)cb->user_data];
    if (started) return 0;
    if (cb->obj != n->signal) { fail("initial notifier callback identity"); return 0; }
    if (n->changes == MAX_EVENTS) { fail("too many initial notifier changes"); return 0; }
    n->values[n->changes] = scalar(cb->obj); n->times[n->changes++] = now();
    return 0;
}
static void observe_initial_scope(vpiHandle scope)
{
    vpiHandle iter = vpi_iterate(vpiTchk, scope), obj;
    while (iter && (obj = vpi_scan(iter))) {
        vpiHandle signal = vpi_handle(vpiTchkNotifier, obj);
        unsigned i; s_cb_data cb = {0}; s_vpi_time time = {0};
        struct initial_notifier *n;
        if (!signal) continue;
        for (i = 0; i < initial_notifier_count; ++i)
            if (initial_notifiers[i].signal == signal) break;
        if (i != initial_notifier_count) continue;
        if (i == MAX_CHECKS) { fail("too many initial notifiers"); return; }
        n = &initial_notifiers[initial_notifier_count++];
        n->signal = signal; n->before = scalar(signal); n->index = i;
        time.type = vpiSimTime;
        cb.reason = cbValueChange; cb.cb_rtn = initial_changed; cb.obj = signal;
        cb.time = &time; cb.user_data = (PLI_BYTE8 *)&n->index;
        if (!vpi_register_cb(&cb)) { fail("initial notifier registration"); return; }
    }
    iter = vpi_iterate(vpiModule, scope);
    while (iter && (obj = vpi_scan(iter))) observe_initial_scope(obj);
}
static PLI_INT32 watch(PLI_BYTE8 *unused)
{
    vpiHandle call = vpi_handle(vpiSysTfCall,NULL), args, scope, iter, obj, match=NULL;
    char *owner,*kind,*ref,*re,*data,*de,*nname;
    double expected_limit[2]={0,0}; unsigned long long expected_ticks[2]={0,0};
    unsigned limits, k;
    int type, redge, dedge; unsigned expected, i, count=0, ni=0;
    vpiHandle notifier; PLI_INT32 expected_initial;
    (void)unused;
    if (started || check_count==MAX_CHECKS) { fail("late/too many watches"); return 0; }
    args=vpi_iterate(vpiArgument,call);
    if (!args) { fail("missing watch arguments"); return 0; }
    owner=string_arg(args); kind=string_arg(args); ref=string_arg(args); re=string_arg(args);
    data=string_arg(args); de=string_arg(args);
    type=AUDIT_CHECK_TYPE(kind); limits=AUDIT_LIMIT_COUNT(type);
    for(k=0;k<limits;++k) { expected_limit[k]=number_arg(args);
        expected_ticks[k]=(unsigned long long)number_arg(args); }
    nname=string_arg(args);
    expected_initial=scalar_arg(args);
    expected=(unsigned)number_arg(args);
    if (failed || expected>MAX_EVENTS) { fail("invalid expectation"); return 0; }
    redge=edge_code(re); dedge=edge_code(de);
    scope=vpi_handle_by_name(owner,NULL);
    if (!scope || !type) { fail("scope/check type"); return 0; }
    iter=vpi_iterate(vpiTchk,scope);
    if (!iter) { fail("missing compiled " AUDIT_LABEL " objects"); return 0; }
    while ((obj=vpi_scan(iter))) {
        s_vpi_time t[2]={{0},{0}}; s_vpi_delay d={0};
        if (vpi_get(vpiTchkType,obj)!=type || !terminal(obj,vpiTchkRefTerm,scope,ref,redge) ||
            !terminal(obj,vpiTchkDataTerm,scope,data,dedge)) continue;
        d.da=t; d.no_of_delays=limits; d.time_type=vpiScaledRealTime; vpi_get_delays(obj,&d);
        for(k=0;k<limits;++k) if(fabs(t[k].real-expected_limit[k])>1e-10) break;
        if(k!=limits) continue;
        notifier=vpi_handle(vpiTchkNotifier,obj);
        if ((*nname && (!notifier || vpi_handle(vpiScope,notifier)!=scope ||
                        strcmp(vpi_get_str(vpiName,notifier),nname))) ||
            (!*nname && notifier)) continue;
        d.time_type=vpiSimTime; vpi_get_delays(obj,&d);
        for(k=0;k<limits;++k) if((((unsigned long long)t[k].high<<32)|t[k].low)!=expected_ticks[k]) break;
        if (k!=limits || vpi_handle(vpiScope,obj)!=scope || vpi_handle(vpiModule,obj)!=scope) {
            fail("limit units or scope identity"); return 0;
        }
        ++count; match=obj;
    }
    if (count!=1) { fail("exact two-terminal object/limit matching"); return 0; }
    count=0;iter=vpi_iterate(vpiTchk,scope);
    if(iter) while((obj=vpi_scan(iter))) if(obj==match) ++count;
    if(count!=1) { fail("unstable combined/timing handle");return 0; }
    for (i=0;i<check_count;++i) if (checks[i].object==match) { fail("duplicate watch");return 0; }
    notifier=vpi_handle(vpiTchkNotifier,match);
    if ((*nname && (!notifier || vpi_handle(vpiScope,notifier)!=scope ||
                        strcmp(vpi_get_str(vpiName,notifier),nname))) ||
        (!*nname && notifier)) { fail("notifier binding");return 0; }
    checks[check_count].object=match; checks[check_count].expected=expected;
    if (notifier) {
        for (ni=0;ni<notifier_count;++ni) if (notifiers[ni].signal==notifier) break;
        if (ni==notifier_count) {
            if (notifier_count==MAX_CHECKS) { fail("too many notifiers");return 0; }
            notifiers[ni].signal=notifier;
            notifiers[ni].previous=notifiers[ni].initial=expected_initial;
            ++notifier_count;
        } else if (notifiers[ni].initial != expected_initial) {
            fail("inconsistent initial notifier expectation"); return 0;
        }
    }
    for (i=0;i<expected;++i) {
        unsigned long long t=(unsigned long long)number_arg(args);
        checks[check_count].times[i]=t;
        if (i && t<checks[check_count].times[i-1]) { fail("unordered fixture times");return 0; }
        if (notifier && notifiers[ni].previous!=vpiZ)
            notifiers[ni].times[notifiers[ni].expected++]=t;
    }
    if (vpi_scan(args)) { fail("extra watch argument");return 0; }
    vpi_printf(AUDIT_TAG "_OBJECT %s %s %s/%s %s/%s", owner,kind,ref,re,data,de);
    for(k=0;k<limits;++k) vpi_printf(" limit=%g ticks=%llu",expected_limit[k],expected_ticks[k]);
    vpi_printf(" events=%u\n",expected);
    ++check_count;
    free(owner);free(kind);free(ref);free(re);free(data);free(de);free(nname);
    return 0;
}
#ifdef AUDIT_SETUPHOLD
static PLI_INT32 population(PLI_BYTE8 *unused)
{
    vpiHandle args=vpi_iterate(vpiArgument,vpi_handle(vpiSysTfCall,NULL));
    char *owner=string_arg(args); unsigned want=(unsigned)number_arg(args), count=0;
    vpiHandle scope=vpi_handle_by_name(owner,NULL), it, obj;
    (void)unused;
    if(!scope || vpi_scan(args)) { fail("population arguments");return 0; }
    it=vpi_iterate(vpiTchk,scope);
    if(it) while((obj=vpi_scan(it))) ++count;
    if(count!=want || !want) fail("compiled population mismatch or empty expectation");
    free(owner);return 0;
}
static PLI_INT32 put_limits(PLI_BYTE8 *unused)
{
    vpiHandle args=vpi_iterate(vpiArgument,vpi_handle(vpiSysTfCall,NULL));
    char *owner=string_arg(args), *mode;
    double a=number_arg(args), b=number_arg(args);
    vpiHandle scope=vpi_handle_by_name(owner,NULL), it, obj, match=NULL;
    s_vpi_time time[2]={{0},{0}};s_vpi_delay d={0};unsigned count=0,k;
    double values[2]={a,b};(void)unused;mode=string_arg(args);
    if(!scope || vpi_scan(args)) { fail("put arguments");return 0; }
    it=vpi_iterate(vpiTchk,scope);
    if(it) while((obj=vpi_scan(it))) if(vpi_get(vpiTchkType,obj)==vpiSetupHold) { match=obj;++count; }
    if(count!=1) { fail("put requires one combined object");return 0; }
    d.da=time;d.no_of_delays=2;
    if(!strcmp(mode,"real")) { d.time_type=vpiScaledRealTime;time[0].real=a;time[1].real=b; }
    else if(!strcmp(mode,"ticks")) { d.time_type=vpiSimTime;
        for(k=0;k<2;++k) { unsigned long long n=(unsigned long long)values[k];
            time[k].high=(PLI_UINT32)(n>>32);time[k].low=(PLI_UINT32)n; }
    } else { fail("put units");return 0; }
    vpi_put_delays(match,&d);
    free(owner);free(mode);return 0;
}
#endif
static void timing_event(vpiHandle object, unsigned long long time)
{
    unsigned i;
    for (i=0;i<check_count;++i) if (checks[i].object==object) break;
    if (i==check_count || checks[i].seen>=checks[i].expected ||
        time!=checks[i].times[checks[i].seen]) {
        fail("extra/wrong identity/type/time timing callback"); return;
    }
    ++checks[i].seen;
    vpi_printf(AUDIT_TAG "_VIOLATION %u %llu\n",i,time);
}
static PLI_INT32 violation(p_cb_data cb)
{
    if (!started) {
        if (initial_violation_count == MAX_CHECKS * MAX_EVENTS) {
            fail("too many initial timing callbacks"); return 0;
        }
        initial_violations[initial_violation_count].object = cb->obj;
        initial_violations[initial_violation_count++].time = now();
    } else timing_event(cb->obj, now());
    return 0;
}
static void notifier_event(unsigned i, vpiHandle object,
                           unsigned long long time, PLI_INT32 value)
{
    struct watched_notifier *n;
    if (i>=notifier_count) { fail("notifier callback index");return; }
    n=&notifiers[i];
    if (object!=n->signal || n->seen>=n->expected || time!=n->times[n->seen] ||
        (value!=vpi0 && value!=vpi1) ||
        (n->previous==vpi0 && value!=vpi1) || (n->previous==vpi1 && value!=vpi0) ||
        n->previous==vpiZ) { fail("notifier transition identity/time/value");return; }
    n->previous=value; ++n->seen;
}
static PLI_INT32 changed(p_cb_data cb)
{
    notifier_event(*(unsigned *)cb->user_data, cb->obj, now(), scalar(cb->obj));
    return 0;
}
static int cmp_time(const void *a,const void *b)
{
    unsigned long long x=*(const unsigned long long *)a,y=*(const unsigned long long *)b;
    return x>y?1:x<y?-1:0;
}
static PLI_INT32 observe_initial(p_cb_data unused)
{
    vpiHandle iter = vpi_iterate(vpiModule, NULL), scope;
    s_cb_data cb = {0}; s_vpi_time time = {0};
    (void)unused;
    while (iter && (scope = vpi_scan(iter))) observe_initial_scope(scope);
    time.type = vpiSimTime;
    cb.reason = cbTchkViolation; cb.cb_rtn = violation; cb.time = &time;
    if (!vpi_register_cb(&cb)) fail("timing callback registration");
    return 0;
}
static int complete_population(void)
{
    unsigned i, j;
    for (i = 0; i < check_count; ++i) {
        unsigned seen[MAX_CHECKS] = {0};
        vpiHandle scope = vpi_handle(vpiScope, checks[i].object), iter, obj;
        for (j = 0; j < i; ++j)
            if (vpi_handle(vpiScope, checks[j].object) == scope) break;
        if (j != i) continue;
        iter = vpi_iterate(vpiTchk, scope);
        while (iter && (obj = vpi_scan(iter))) {
            for (j = 0; j < check_count; ++j)
                if (checks[j].object == obj) break;
            if (j == check_count || vpi_handle(vpiScope, obj) != scope ||
                seen[j]++) return 0;
        }
        for (j = 0; j < check_count; ++j)
            if (vpi_handle(vpiScope, checks[j].object) == scope && !seen[j])
                return 0;
    }
    return 1;
}
static int replay_initial_notifier(unsigned index)
{
    struct watched_notifier *expected = &notifiers[index];
    unsigned i, first = 0;
    struct initial_notifier *n;
    for (i = 0; i < initial_notifier_count; ++i)
        if (initial_notifiers[i].signal == expected->signal) break;
    if (i == initial_notifier_count) return 0;
    n = &initial_notifiers[i];
    if (n->before != expected->initial) {
        if (!n->changes || n->times[0] != 0 || n->values[0] != expected->initial)
            return 0;
        first = 1;
    }
    for (i = first; i < n->changes; ++i) {
        notifier_event(index, n->signal, n->times[i], n->values[i]);
        if (failed) return 0;
    }
    return scalar(n->signal) == expected->previous;
}
static PLI_INT32 start(PLI_BYTE8 *unused)
{
    s_cb_data cb={0}; s_vpi_time time={0}; unsigned i;
    (void)unused;
    if (started || !check_count) { fail("empty/repeated start");return 0; }
    if (!complete_population()) { fail("timing object set mismatch in watched scope"); return 0; }
    started=1; time.type=vpiSimTime;
    for (i=0;i<notifier_count;++i) {
        qsort(notifiers[i].times,notifiers[i].expected,sizeof(unsigned long long),cmp_time);
        if (!replay_initial_notifier(i)) { fail("initial notifier value/history"); return 0; }
        notifier_indices[i]=i; memset(&cb,0,sizeof(cb));cb.reason=cbValueChange;
        cb.obj=notifiers[i].signal;cb.cb_rtn=changed;cb.time=&time;
        cb.user_data=(PLI_BYTE8 *)&notifier_indices[i];
        if (!vpi_register_cb(&cb)) { fail("notifier callback registration");return 0; }
    }
    for (i = 0; i < initial_violation_count; ++i) {
        timing_event(initial_violations[i].object, initial_violations[i].time);
        if (failed) return 0;
    }
    return 0;
}
static PLI_INT32 finish(PLI_BYTE8 *unused)
{
    unsigned i,total=0; (void)unused;
    if (!started || finished || failed) { fail("missing/repeated start/finish");return 0; }
    for (i=0;i<check_count;++i) {
        if (checks[i].seen!=checks[i].expected) { fail("missing timing callbacks");return 0; }
        total+=checks[i].seen;
    }
    for (i=0;i<notifier_count;++i) if (notifiers[i].seen!=notifiers[i].expected ||
        scalar(notifiers[i].signal)!=notifiers[i].previous) {
        fail("missing notifier callbacks/final value");return 0;
    }
    finished=1;vpi_printf(AUDIT_TAG "_COMPLETE objects=%u callbacks=%u\nPASSED\n",check_count,total);
    return 0;
}
#else
static PLI_INT32 watch(PLI_BYTE8 *u) { (void)u;fail("timing API unavailable");return 0; }
static PLI_INT32 start(PLI_BYTE8 *u) { (void)u;fail("timing API unavailable");return 0; }
static PLI_INT32 finish(PLI_BYTE8 *u) { (void)u;fail("timing API unavailable");return 0; }
#endif
static void register_all(void)
{
    s_vpi_systf_data tf={0};tf.type=vpiSysTask;
#ifdef vpiTchk
    s_cb_data cb={0};cb.reason=cbStartOfSimulation;cb.cb_rtn=observe_initial;
    if (!vpi_register_cb(&cb)) fail("start-of-simulation registration");
#endif
#if defined(AUDIT_SETUPHOLD) && defined(vpiTchk)
    tf.tfname="$shc_population";tf.calltf=population;vpi_register_systf(&tf);
    tf.tfname="$shc_put";tf.calltf=put_limits;vpi_register_systf(&tf);
#endif
    tf.tfname="$" AUDIT_PREFIX "_watch";tf.calltf=watch;vpi_register_systf(&tf);
    tf.tfname="$" AUDIT_PREFIX "_start";tf.calltf=start;vpi_register_systf(&tf);
    tf.tfname="$" AUDIT_PREFIX "_finish";tf.calltf=finish;vpi_register_systf(&tf);
}
void (*vlog_startup_routines[])(void)={register_all,0};

/* Independent mixed-object and actual callback audit. */
#include <vpi_user.h>
#include <string.h>

static vpiHandle objects[5],self_handle;
static unsigned seen[5],filtered_seen,self_seen,removed_seen;
static const unsigned counts[5]={1,2,1,1,0};
static const unsigned long times[5][2]={{109,0},{109,119},{102,0},{109,0},{0,0}};
static const double limits[5]={10,20,5,10,0};

static void fail(const char *why)
{
    vpi_printf("FAIL PERIOD object audit: %s\n",why);
    vpi_control(vpiFinish,1);
}
static unsigned long long now(void)
{
    s_vpi_time t={0};t.type=vpiSimTime;vpi_get_time(NULL,&t);
    return ((unsigned long long)t.high<<32)|t.low;
}
static PLI_INT32 violation(p_cb_data cb)
{
    unsigned i;
    for(i=0;i<5;++i)if(objects[i]==cb->obj)break;
    if(i==5 || seen[i]>=counts[i] || now()!=times[i][seen[i]]) {
        fail("wrong/extra timing callback");return 0;
    }
    if(vpi_get(vpiTchkType,cb->obj)!=(i==2?vpiWidth:vpiPeriod)) {
        fail("wrong callback type");return 0;
    }
    ++seen[i];
    vpi_printf("PERIOD_TCHK %u %llu\n",i,now());
    return 0;
}
static PLI_INT32 filtered(p_cb_data cb)
{
    if(cb->obj!=objects[1] || filtered_seen>=2 || now()!=times[1][filtered_seen]) {
        fail("object-filtered callback");return 0;
    }
    ++filtered_seen;return 0;
}
static PLI_INT32 removed(p_cb_data unused)
{
    (void)unused;++removed_seen;fail("removed callback fired");return 0;
}
static PLI_INT32 self_removing(p_cb_data cb)
{
    if(self_seen || cb->obj!=objects[2] || now()!=102) {fail("self-removing callback");return 0;}
    ++self_seen;
    if(!vpi_remove_cb(self_handle))fail("self-removal failed");
    return 0;
}
static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    static const char *names[]={"tb.same","tb.omit","tb.zero"};
    static const unsigned expected[]={3,1,1};
    unsigned s;
    s_cb_data cb={0};vpiHandle removed_handle;
    (void)unused;
    for(s=0;s<3;++s) {
        vpiHandle scope=vpi_handle_by_name((PLI_BYTE8 *)names[s],NULL),iter,obj;
        unsigned count=0;
        if(!scope || !(iter=vpi_iterate(vpiTchk,scope))) {fail("missing scope/objects");return 0;}
        while((obj=vpi_scan(iter))) {
            vpiHandle notifier=vpi_handle(vpiTchkNotifier,obj),term,expr;
            s_vpi_time value={0};s_vpi_delay delay={0};unsigned i;
            ++count;
            if(s==0) {
                const char *name;
                if(!notifier) {fail("missing notifier");return 0;}
                name=vpi_get_str(vpiName,notifier);
                if(!strcmp(name,"n10"))i=0;
                else if(!strcmp(name,"n20"))i=1;
                else if(!strcmp(name,"nw"))i=2;
                else {fail("wrong notifier");return 0;}
            } else {
                if(notifier) {fail("fabricated optional notifier");return 0;}
                i=s+2;
            }
            if(objects[i] || vpi_get(vpiType,obj)!=vpiTchk ||
               vpi_get(vpiTchkType,obj)!=(i==2?vpiWidth:vpiPeriod) ||
               vpi_handle(vpiScope,obj)!=scope) {fail("object identity/type");return 0;}
            objects[i]=obj;
            term=vpi_handle(vpiTchkRefTerm,obj);
            if(!term || term!=vpi_handle(vpiTchkRefTerm,obj) || vpi_get(vpiEdge,term)!=vpiPosedge) {
                fail("reference identity/edge");return 0;
            }
            expr=vpi_handle(vpiExpr,term);
            if(!expr || strcmp(vpi_get_str(vpiName,expr),"CLK")) {fail("reference expression");return 0;}
            if(vpi_handle(vpiTchkDataTerm,obj)) {fail("fabricated data terminal");return 0;}
            delay.da=&value;delay.no_of_delays=1;delay.time_type=vpiScaledRealTime;
            vpi_get_delays(obj,&delay);
            if(value.real!=limits[i]) {fail("wrong limit");return 0;}
            delay.time_type=vpiSimTime;memset(&value,0,sizeof(value));vpi_get_delays(obj,&delay);
            if(value.high || value.low!=(unsigned)limits[i]) {fail("wrong tick limit");return 0;}
        }
        if(count!=expected[s]) {fail("object cardinality");return 0;}
    }
    cb.reason=cbTchkViolation;cb.cb_rtn=violation;
    if(!vpi_register_cb(&cb)) {fail("callback registration");return 0;}
    cb.obj=objects[1];cb.cb_rtn=filtered;
    if(!vpi_register_cb(&cb)) {fail("filtered registration");return 0;}
    cb.obj=NULL;cb.cb_rtn=removed;removed_handle=vpi_register_cb(&cb);
    if(!removed_handle || !vpi_remove_cb(removed_handle)) {fail("pre-event removal");return 0;}
    cb.cb_rtn=self_removing;self_handle=vpi_register_cb(&cb);
    if(!self_handle) {fail("self-removal registration");return 0;}
    vpi_printf("PERIOD_OBJECTS 5\n");return 0;
}
static PLI_INT32 finish(PLI_BYTE8 *unused)
{
    unsigned i;(void)unused;
    for(i=0;i<5;++i)if(seen[i]!=counts[i]) {fail("missing timing callbacks");return 0;}
    if(filtered_seen!=2 || self_seen!=1 || removed_seen) {fail("callback lifetime totals");return 0;}
    vpi_printf("PERIOD_CALLBACKS 5 filter=2 self=1 removed=0\n");return 0;
}
static void register_all(void)
{
    s_vpi_systf_data tf={0};tf.type=vpiSysTask;tf.tfname="$period_objects";tf.calltf=audit;
    vpi_register_systf(&tf);tf.tfname="$period_objects_finish";tf.calltf=finish;vpi_register_systf(&tf);
}
void (*vlog_startup_routines[])(void)={register_all,0};

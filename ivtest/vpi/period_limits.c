/* Read-only exact type/reference/edge/value checks supplied by the HDL oracle. */
#include <vpi_user.h>
#include <stdlib.h>
#include <string.h>

static void fail(const char *why)
{
    vpi_printf("FAIL PERIOD limit audit: %s\n",why);
    vpi_control(vpiFinish,1);
}

static char *string_arg(vpiHandle args)
{
    vpiHandle arg=vpi_scan(args);
    s_vpi_value value={0};
    if (!arg) return NULL;
    value.format=vpiStringVal;
    vpi_get_value(arg,&value);
    return value.value.str ? strdup(value.value.str) : NULL;
}

static PLI_INT32 audit(PLI_BYTE8 *unused)
{
    vpiHandle call=vpi_handle(vpiSysTfCall,NULL),args=vpi_iterate(vpiArgument,call);
    char *owner,*kind,*reference,*edge_name;
    vpiHandle arg,scope,iter,obj;
    s_vpi_value expected={0},ticks={0};
    int type,edge;
    unsigned matches=0;
    (void)unused;
    if (!args) { fail("missing arguments"); return 0; }
    owner=string_arg(args);kind=string_arg(args);
    reference=string_arg(args);edge_name=string_arg(args);
    if (!owner || !kind || !reference || !edge_name) {
        fail("missing string argument");return 0;
    }
    arg=vpi_scan(args);
    if (!arg) { fail("missing real limit");return 0; }
    expected.format=vpiRealVal;vpi_get_value(arg,&expected);
    arg=vpi_scan(args);
    if (!arg) { fail("missing tick limit");return 0; }
    ticks.format=vpiIntVal;vpi_get_value(arg,&ticks);
    if (vpi_scan(args)) { fail("extra argument");return 0; }
    type=!strcmp(kind,"period")?vpiPeriod:!strcmp(kind,"width")?vpiWidth:0;
    edge=!strcmp(edge_name,"posedge")?vpiPosedge:!strcmp(edge_name,"negedge")?vpiNegedge:0;
    if (!type || !edge) { fail("unknown expected type/edge");return 0; }
    scope=vpi_handle_by_name(owner,NULL);
    if (!scope) { fail("missing scope");return 0; }
    iter=vpi_iterate(vpiTchk,scope);
    if (!iter) { fail("missing timing objects");return 0; }
    while ((obj=vpi_scan(iter))) {
        vpiHandle term,expr;
        s_vpi_time value={0};s_vpi_delay delay={0};
        if (vpi_get(vpiTchkType,obj)!=type) continue;
        if (vpi_handle(vpiScope,obj)!=scope) { fail("wrong owner");return 0; }
        term=vpi_handle(vpiTchkRefTerm,obj);
        if (!term || term!=vpi_handle(vpiTchkRefTerm,obj)) { fail("reference lifetime");return 0; }
        expr=vpi_handle(vpiExpr,term);
        if (!expr || strcmp(vpi_get_str(vpiName,expr),reference) || vpi_get(vpiEdge,term)!=edge) continue;
        ++matches;
        delay.da=&value;delay.no_of_delays=1;delay.time_type=vpiScaledRealTime;
        vpi_get_delays(obj,&delay);
        if (value.real!=expected.value.real) { fail("wrong scaled limit");return 0; }
        memset(&value,0,sizeof(value));delay.time_type=vpiSimTime;
        vpi_get_delays(obj,&delay);
        if (value.high || value.low!=(unsigned)ticks.value.integer) { fail("wrong tick limit");return 0; }
    }
    if (matches!=1) { fail("missing/duplicate expected timing object");return 0; }
    vpi_printf("PERIOD_LIMIT %s %s %s %s %g %d\n",owner,kind,reference,edge_name,
               expected.value.real,ticks.value.integer);
    free(owner);free(kind);free(reference);free(edge_name);
    return 0;
}

static void register_all(void)
{
    s_vpi_systf_data tf={0};
    tf.type=vpiSysTask;tf.tfname="$period_limit";tf.calltf=audit;
    vpi_register_systf(&tf);
}
void pulse_history_register(void);
void (*vlog_startup_routines[])(void)={register_all,pulse_history_register,0};

#include <string.h>
#include <vpi_user.h>

static vpiHandle find_port(const char *scope, const char *name)
{
      vpiHandle s=vpi_handle_by_name((PLI_BYTE8*)scope,0),it,p,result=0;
      if (!s) return 0;
      it=vpi_iterate(vpiPort,s);
      while (it && (p=vpi_scan(it))) {
            const char *n=vpi_get_str(vpiName,p);
            if (n && !strcmp(n,name)) result=p;
      }
      return result;
}
/* Twelve distinct initial delays ensure increments are expanded before addition.
 * For derived X entries, a missing required operand supplies no update.
 */
static PLI_INT32 check_cardinality(PLI_BYTE8 *user_data)
{
      static const unsigned expected[9][12] = {
            {13, 28, 39, 43, 58, 72, 73, 85, 102, 106, 118, 129},
            {13, 28, 40, 43, 56, 72, 73, 85, 100, 106, 116, 129},
            {13, 28, 40, 48, 64, 80, 73, 90, 102, 114, 124, 134},
            {12, 25, 40, 45, 58, 73, 78, 91, 106, 111, 124, 139},
            {13, 23, 39, 43, 53, 67, 73, 85, 97, 101, 113, 127},
            {13, 23, 40, 48, 53, 80, 73, 90, 97, 101, 113, 134},
            {12, 23, 40, 41, 58, 67, 78, 83, 106, 101, 124, 127},
            {11, 23, 37, 41, 53, 67, 71, 83, 97, 101, 113, 127},
            {11, 23, 37, 41, 53, 67, 71, 83, 97, 101, 113, 127}
      };
      vpiHandle call=vpi_handle(vpiSysTfCall,0),args=vpi_iterate(vpiArgument,call);
      vpiHandle source=find_port("test.src","Y"),load=find_port("test.sink","A");
      vpiHandle path=source && load ? vpi_handle_multi(vpiInterModPath,source,load) : 0;
      s_vpi_time times[12];s_vpi_delay delays;s_vpi_value value;
      unsigned phase,i;int ok=path!=0;
      (void)user_data;
      value.format=vpiIntVal;
      vpi_get_value(vpi_scan(args),&value);phase=value.value.integer;
      vpi_free_object(args);
      if(phase>=9)ok=0;
      memset(times,0,sizeof times);memset(&delays,0,sizeof delays);
      delays.da=times;delays.no_of_delays=12;delays.time_type=vpiSimTime;
      if(path)vpi_get_delays(path,&delays);
      if(phase<9)for(i=0;i<12;i++)if(times[i].high || times[i].low!=expected[phase][i]) {
            vpi_printf("FAIL phase %u delay %u: %u expected %u\n",phase,i,times[i].low,expected[phase][i]);ok=0;
      }
      value.value.integer=ok;vpi_put_value(call,&value,0,vpiNoDelay);return 0;
}
static void register_check(void)
{
      s_vpi_systf_data data;memset(&data,0,sizeof data);
      data.type=vpiSysFunc;data.sysfunctype=vpiIntFunc;
      data.tfname="$check_increment_cardinality";data.calltf=check_cardinality;vpi_register_systf(&data);
}
void (*vlog_startup_routines[])(void)={register_check,0};

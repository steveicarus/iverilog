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
static PLI_INT32 check_pair(PLI_BYTE8 *user_data)
{
      vpiHandle call=vpi_handle(vpiSysTfCall,0),args=vpi_iterate(vpiArgument,call);
      vpiHandle source=find_port("test.src","Y"),load=find_port("test.sink","A");
      vpiHandle path=source && load ? vpi_handle_multi(vpiInterModPath,source,load) : 0;
      s_vpi_time times[2];s_vpi_delay delays;s_vpi_value value;
      unsigned expected[2],i;int ok=path!=0;
      (void)user_data;
      value.format=vpiIntVal;
      for(i=0;i<2;i++){vpi_get_value(vpi_scan(args),&value);expected[i]=value.value.integer;}
      vpi_free_object(args);
      memset(times,0,sizeof times);memset(&delays,0,sizeof delays);
      delays.da=times;delays.no_of_delays=2;delays.time_type=vpiSimTime;
      if(path)vpi_get_delays(path,&delays);
      for(i=0;i<2;i++)if(times[i].high || times[i].low!=expected[i]) {
            vpi_printf("FAIL delay %u: %u expected %u\n",i,times[i].low,expected[i]);ok=0;
      }
      value.value.integer=ok;vpi_put_value(call,&value,0,vpiNoDelay);return 0;
}
static void register_check(void)
{
      s_vpi_systf_data data;memset(&data,0,sizeof data);
      data.type=vpiSysFunc;data.sysfunctype=vpiIntFunc;
      data.tfname="$check_increment_pair";data.calltf=check_pair;vpi_register_systf(&data);
}
void (*vlog_startup_routines[])(void)={register_check,0};

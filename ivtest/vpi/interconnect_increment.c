#include <string.h>
#include <vpi_user.h>

static vpiHandle find_port(const char *scope,const char *name)
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

static PLI_INT32 check_paths(PLI_BYTE8 *user_data)
{
      static vpiHandle previous[2];
      const char *scopes[2]={"test.sink","test.sibling"};
      vpiHandle call=vpi_handle(vpiSysTfCall,0);
      vpiHandle args=vpi_iterate(vpiArgument,call),arg;
      vpiHandle source=find_port("test.src","Y");
      s_vpi_value value;
      unsigned expected[4],i;
      int ok=1;
      (void)user_data;
      value.format=vpiIntVal;
      for(i=0;i<4;i++) {
            arg=vpi_scan(args);
            vpi_get_value(arg,&value);
            expected[i]=value.value.integer;
      }
      vpi_free_object(args);
      for(i=0;i<2;i++) {
            vpiHandle load=find_port(scopes[i],"A");
            vpiHandle path=source && load ? vpi_handle_multi(vpiInterModPath,source,load) : 0;
            s_vpi_time times[2];s_vpi_delay delays;
            if(!path){ok=0;continue;}
            if(previous[i] && previous[i]!=path)ok=0;
            previous[i]=path;
            memset(times,0,sizeof times);memset(&delays,0,sizeof delays);
            delays.da=times;delays.no_of_delays=2;delays.time_type=vpiSimTime;
            vpi_get_delays(path,&delays);
            if(times[0].high || times[1].high || times[0].low!=expected[2*i] ||
               times[1].low!=expected[2*i+1]) {
                  vpi_printf("FAIL %s readback %u/%u expected %u/%u\n",scopes[i],
                             times[0].low,times[1].low,expected[2*i],expected[2*i+1]);
                  ok=0;
            }
      }
      if(previous[0]==previous[1])ok=0;
      value.value.integer=ok;
      vpi_put_value(call,&value,0,vpiNoDelay);
      return 0;
}

static void register_check(void)
{
      s_vpi_systf_data data;memset(&data,0,sizeof data);
      data.type=vpiSysFunc;data.sysfunctype=vpiIntFunc;
      data.tfname="$check_increment_paths";data.calltf=check_paths;
      vpi_register_systf(&data);
}
void (*vlog_startup_routines[])(void)={register_check,0};

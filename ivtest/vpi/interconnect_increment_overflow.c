#include <string.h>
#include <vpi_user.h>

static vpiHandle port(const char *scope,const char *name)
{
      vpiHandle s=vpi_handle_by_name((PLI_BYTE8*)scope,0),it,p,result=0;
      if(!s)return 0;
      it=vpi_iterate(vpiPort,s);
      while(it && (p=vpi_scan(it))) {
            const char *n=vpi_get_str(vpiName,p);
            if(n && !strcmp(n,name))result=p;
      }
      return result;
}
static PLI_INT32 check(PLI_BYTE8 *u)
{
      const char *scopes[2]={"test.sink","test.coarse"};
      unsigned rises[2]={5,5000};
      vpiHandle source=port("test.src","Y"),call=vpi_handle(vpiSysTfCall,0);
      s_vpi_value value;
      int ok=1;
      (void)u;
      for(unsigned i=0;i<2;i++) {
            vpiHandle load=port(scopes[i],"A");
            vpiHandle h=source && load ? vpi_handle_multi(vpiInterModPath,source,load) : 0;
            s_vpi_time t[2];s_vpi_delay d;
            if(!h){ok=0;continue;}
            memset(t,0,sizeof t);memset(&d,0,sizeof d);
            d.da=t;d.no_of_delays=2;d.time_type=vpiSimTime;vpi_get_delays(h,&d);
            // 16,000,000,000,000,000,000 ticks, independently written as two words.
            if(t[0].high || t[0].low!=rises[i] ||
               t[1].high!=0xde0b6b3aU || t[1].low!=0x76400000U) {
                  vpi_printf("FAIL atomic overflow rejection %s %x:%x %x:%x\n",
                             scopes[i],t[0].high,t[0].low,t[1].high,t[1].low);
                  ok=0;
            }
      }
      value.format=vpiIntVal;value.value.integer=ok;vpi_put_value(call,&value,0,vpiNoDelay);
      return 0;
}
static void reg(void)
{
      s_vpi_systf_data d;memset(&d,0,sizeof d);
      d.type=vpiSysFunc;d.sysfunctype=vpiIntFunc;
      d.tfname="$check_increment_overflow";d.calltf=check;vpi_register_systf(&d);
}
void (*vlog_startup_routines[])(void)={reg,0};

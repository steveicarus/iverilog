#include <math.h>
#include <stdint.h>
#include <string.h>
#include <vpi_user.h>

/* Literal expectations are supplied by HDL, independently of delay readback. */
static vpiHandle retained;

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

static vpiHandle path(void)
{
      vpiHandle source=find_port("test.src","Y");
      vpiHandle load=find_port("test.sink","A");
      vpiHandle h=source && load ? vpi_handle_multi(vpiInterModPath,source,load) : 0;
      if (!h || (retained && h!=retained)) {
            vpi_printf("FAIL accumulated delay path identity\n");
            vpi_control(vpiFinish,1);
            return 0;
      }
      retained=h;
      return h;
}

static int arguments(vpiHandle *args, unsigned count)
{
      vpiHandle call=vpi_handle(vpiSysTfCall,0);
      vpiHandle it=vpi_iterate(vpiArgument,call);
      for (unsigned i=0;i<count;i+=1) {
            if (!it || !(args[i]=vpi_scan(it))) return 0;
      }
      if (vpi_scan(it)) { vpi_free_object(it); return 0; }
      return 1;
}

static double real_arg(vpiHandle arg)
{
      s_vpi_value value;
      value.format=vpiRealVal;
      vpi_get_value(arg,&value);
      return value.value.real;
}

static uint64_t integer_arg(vpiHandle arg)
{
      s_vpi_value value;
      uint64_t result;
      value.format=vpiVectorVal;
      vpi_get_value(arg,&value);
      result=(uint32_t)value.value.vector[0].aval;
      if (vpi_get(vpiSize,arg)>32)
            result|=(uint64_t)(uint32_t)value.value.vector[1].aval<<32;
      return result;
}

static void result(int ok)
{
      s_vpi_value value;
      value.format=vpiIntVal;value.value.integer=ok;
      vpi_put_value(vpi_handle(vpiSysTfCall,0),&value,0,vpiNoDelay);
}

static int check_values(const double *raw, const uint64_t *ticks, unsigned count)
{
      vpiHandle h=path();
      s_vpi_time values[12];s_vpi_delay delays;
      int ok=h!=0;
      if (!h) return 0;
      memset(values,0,sizeof values);memset(&delays,0,sizeof delays);
      delays.da=values;delays.no_of_delays=count;delays.time_type=vpiScaledRealTime;
      vpi_get_delays(h,&delays);
      for (unsigned i=0;i<count;i+=1) {
            if (!isfinite(values[i].real) || fabs(values[i].real-raw[i])>1e-12) {
                  vpi_printf("FAIL raw delay %u: %.17g expected %.17g\n",i,values[i].real,raw[i]);
                  ok=0;
            }
      }
      memset(values,0,sizeof values);delays.time_type=vpiSimTime;
      vpi_get_delays(h,&delays);
      for (unsigned i=0;i<count;i+=1) {
            uint64_t actual=((uint64_t)values[i].high<<32)|values[i].low;
            if (actual!=ticks[i]) {
                  vpi_printf("FAIL integer delay %u: %x:%x expected %x:%x\n",i,
                             values[i].high,values[i].low,(unsigned)(ticks[i]>>32),(unsigned)ticks[i]);
                  ok=0;
            }
      }
      return ok;
}

static PLI_INT32 check_pair(PLI_BYTE8 *unused)
{
      vpiHandle args[4];double raw[2];uint64_t ticks[2];
      (void)unused;
      if (!arguments(args,4)) { result(0); return 0; }
      for (unsigned i=0;i<2;i+=1) { raw[i]=real_arg(args[i]);ticks[i]=integer_arg(args[i+2]); }
      result(check_values(raw,ticks,2));return 0;
}

/* Existing Icarus twelve-slot access; this is not a new standard VPI claim. */
static PLI_INT32 check_slots(PLI_BYTE8 *unused)
{
      static const double raw[5][12]={
        {-.4,-.4,-.4,-.4,-.4,-.4,-.4,-.4,-.4,-.4,-.4,-.4},
        {-1.4,2.6,-1.4,-1.4,2.6,2.6,-1.4,-1.4,2.6,2.6,2.6,-1.4},
        {-1.4,2.6,-3.4,-1.4,-3.4,2.6,-3.4,-1.4,-3.4,2.6,-3.4,-1.4},
        {-1.4,2.6,-3.4,4.6,-5.4,6.6,-3.4,4.6,-5.4,6.6,-3.4,4.6},
        {-1.4,2.6,-3.4,4.6,-5.4,6.6,-7.4,8.6,-9.4,10.6,-11.4,12.6}
      };
      static const uint64_t ticks[5][12]={
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,3,0,0,3,3,0,0,3,3,3,0},
        {0,3,0,0,0,3,0,0,0,3,0,0},
        {0,3,0,5,0,7,0,5,0,7,0,5},
        {0,3,0,5,0,7,0,9,0,11,0,13}
      };
      vpiHandle args[1];unsigned which;
      (void)unused;
      if (!arguments(args,1)) { result(0);return 0; }
      which=(unsigned)integer_arg(args[0]);
      result(which<5 && check_values(raw[which],ticks[which],12));return 0;
}

static void write_pair(int integer, const double *raw, const uint64_t *ticks)
{
      vpiHandle h=path();s_vpi_time values[2];s_vpi_delay delays;
      if (!h) return;
      memset(values,0,sizeof values);memset(&delays,0,sizeof delays);
      delays.da=values;delays.no_of_delays=2;
      delays.time_type=integer ? vpiSimTime : vpiScaledRealTime;
      for (unsigned i=0;i<2;i+=1) {
            values[i].type=delays.time_type;
            if (integer) { values[i].high=ticks[i]>>32;values[i].low=ticks[i]; }
            else values[i].real=raw[i];
      }
      vpi_put_delays(h,&delays);
}

static PLI_INT32 write_real(PLI_BYTE8 *unused)
{
      vpiHandle args[2];double raw[2];
      (void)unused;
      if (!arguments(args,2)) { vpi_control(vpiFinish,1);return 0; }
      raw[0]=real_arg(args[0]);raw[1]=real_arg(args[1]);write_pair(0,raw,0);return 0;
}

static PLI_INT32 write_integer(PLI_BYTE8 *unused)
{
      vpiHandle args[2];uint64_t ticks[2];
      (void)unused;
      if (!arguments(args,2)) { vpi_control(vpiFinish,1);return 0; }
      ticks[0]=integer_arg(args[0]);ticks[1]=integer_arg(args[1]);write_pair(1,0,ticks);return 0;
}

static PLI_INT32 write_invalid(PLI_BYTE8 *unused)
{
      vpiHandle args[1];double raw[2]={9.0,0.0};unsigned which;
      (void)unused;
      if (!arguments(args,1)) { vpi_control(vpiFinish,1);return 0; }
      which=(unsigned)integer_arg(args[0]);
      if (which>3) { vpi_control(vpiFinish,1);return 0; }
      raw[1]=which==0 ? NAN : which==1 ? INFINITY : which==2 ? 2e19 : 2e16;
      if (which==3) raw[0]=1.0;
      write_pair(0,raw,0);return 0;
}

static void register_accumulated(void)
{
      s_vpi_systf_data tf;
      memset(&tf,0,sizeof tf);tf.type=vpiSysFunc;tf.sysfunctype=vpiIntFunc;
      tf.tfname="$accum_check";tf.calltf=check_pair;vpi_register_systf(&tf);
      tf.tfname="$accum_check_slots";tf.calltf=check_slots;vpi_register_systf(&tf);
      tf.type=vpiSysTask;
      tf.tfname="$accum_write_real";tf.calltf=write_real;vpi_register_systf(&tf);
      tf.tfname="$accum_write_integer";tf.calltf=write_integer;vpi_register_systf(&tf);
      tf.tfname="$accum_write_invalid";tf.calltf=write_invalid;vpi_register_systf(&tf);
}
void (*vlog_startup_routines[])(void)={register_accumulated,0};

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vpi_user.h>

/*
 * This file exercises an error
 */

static int chkvpierr(void)
{
  s_vpi_error_info info;
  int level;

  if ((level = vpi_chk_error(&info)) != 0) {
    fprintf(stderr, "+++ VPI ERROR +++ level %d\n", level);
    fprintf(stderr, "+++ MESS: %s\n", info.message);
    fprintf(stderr, "+++ PROD: %s\n", info.product);
    fprintf(stderr, "+++ CODE: %s\n", info.code);
    fprintf(stderr, "+++ FILE: %s\n", info.file);
    fprintf(stderr, "+++\n");
  }
  return level;
}

static PLI_INT32 xxx_compiletf(PLI_BYTE8 *user_data)
{
  (void)user_data;  /* Parameter is not used. */
  return 0;
}

char		charbuf[100];

static PLI_INT32 xxx_calltf(PLI_BYTE8 *user_data)
{
  vpiHandle             systf_h;
  s_vpi_value		vpival; /* get/set register values */
  s_vpi_time		t;
  vpiHandle	arg_iterator;
  int		i;
  vpiHandle	argi[100];

  (void)user_data;  /* Parameter is not used. */

  /* Get handle to this instance, look up our workarea */
  systf_h = vpi_handle(vpiSysTfCall, NULL);
  chkvpierr();

  arg_iterator = vpi_iterate(vpiArgument, systf_h);
  chkvpierr();
  i = 0;

  if (arg_iterator == NULL) {
    fprintf(stderr, "ERROR: missing argument list to $example(...)");
  }

  /* Copy args pointers into argi array */
  while ((argi[i] = vpi_scan(arg_iterator)) != NULL) {
    chkvpierr();
    i++;
  }
  /* iterator is exhausted, no need to free */

  /* Fill in the time struct */
  t.type = vpiScaledRealTime;
  t.high = 0;
  t.low = 0;
  t.real = 10.0;

  /* Fill in the value struct */
  vpival.format = vpiBinStrVal;
  vpival.value.str = charbuf;

  /*
   * This is where the real work happens.  We are called in an intial
   * block and we schedule three "set-values" at times 10, 20 and 30
   * to args 0, 1 and 2.  The charbuf gets shared among the three
   * calls, even though it shouldn't and the values are not distinct.
   */

  /* Write this value to argi[0] at time 10.0 */
  strcpy(charbuf, "01010101");
  vpi_put_value(argi[0], &vpival, &t, vpiTransportDelay);

  /* Write this value to argi[1] at time 20.0 */
  strcpy(charbuf, "x1x1x1x1");
  t.real = 20.0;
  vpi_put_value(argi[1], &vpival, &t, vpiTransportDelay);

  /* Write this value to argi[2] at time 30.0 */
  strcpy(charbuf, "0xz101xz");
  t.real = 30.0;
  vpi_put_value(argi[2], &vpival, &t, vpiTransportDelay);

  return 0;
}


static void xxx_register(void)
{
  s_vpi_systf_data      tfdata;

  vpi_printf("+++ in XXX_REGISTER\n");

  tfdata.type           = vpiSysFunc;

  /*
   * TOM: sysfunctype field seems to be problematic for different simulators!
   *  some simulators simply don't register callback if subtype missing.
   *
   * Icarus - doesn't implement vpiSizedFunc, so use vpiIntFunct.
   * CVER - use vpiIntFunc
   * MTI - use vpiSizedFunc or vpiIntFunc.
   * XL/NC - doesn't matter
   * VCS - doesn't matter
   *
   */

  tfdata.sysfunctype    = vpiIntFunc;
  /* tfdata.sysfunctype    = vpiSizedFunc; */

  tfdata.tfname         = "$example";
  tfdata.calltf         = xxx_calltf;
  tfdata.compiletf      = xxx_compiletf;
  /* tfdata.sizetf         = xxx_sizetf; */
  tfdata.sizetf         = 0;
  tfdata.user_data      = 0;

  vpi_register_systf(&tfdata);
  chkvpierr();
}




static void xxx_startup(void)
{
  vpi_printf("*** Registering XXX PLI functions.\n");
  xxx_register();
}

/**
 *
 *
 **/

void (*vlog_startup_routines[])(void) = {
    xxx_startup,
    0
};

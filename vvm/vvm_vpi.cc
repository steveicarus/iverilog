#include <stdarg.h>
#include "vpi_user.h"
#include "vpithunk.h"

vpi_thunk vvmt;

void vvm_vpi_init()
{
  vvmt.magic = VPI_THUNK_MAGIC;
  vvmt.vpi_register_systf = vpi_register_systf;
  vvmt.vpi_vprintf = vpi_vprintf;
  vvmt.vpi_mcd_close = vpi_mcd_close;
  vvmt.vpi_mcd_name = vpi_mcd_name;
  vvmt.vpi_mcd_open = vpi_mcd_open;
  vvmt.vpi_mcd_open_x = vpi_mcd_open_x;
  vvmt.vpi_mcd_vprintf = vpi_mcd_vprintf;
  vvmt.vpi_mcd_fputc = vpi_mcd_fputc;
  vvmt.vpi_mcd_fgetc = vpi_mcd_fgetc;
  vvmt.vpi_register_cb = vpi_register_cb;
  vvmt.vpi_remove_cb = vpi_remove_cb;
  vvmt.vpi_sim_vcontrol = vpi_sim_vcontrol;
  vvmt.vpi_handle = vpi_handle;
  vvmt.vpi_iterate = vpi_iterate;
  vvmt.vpi_scan = vpi_scan;
  vvmt.vpi_handle_by_index = vpi_handle_by_index;
  vvmt.vpi_get_time = vpi_get_time;
  vvmt.vpi_get = vpi_get;
  vvmt.vpi_get_str = vpi_get_str;
  vvmt.vpi_get_value = vpi_get_value;
  vvmt.vpi_put_value = vpi_put_value;
  vvmt.vpi_free_object= vpi_free_object;
  vvmt.vpi_get_vlog_info = vpi_get_vlog_info;
}

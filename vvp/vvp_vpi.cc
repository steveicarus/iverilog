#include <stdarg.h>
#include "vpi_user.h"
#include "vpithunk.h"

vpi_thunk vvpt;

void vvp_vpi_init()
{
  vvpt.magic = VPI_THUNK_MAGIC;
  vvpt.vpi_register_systf = vpi_register_systf;
  vvpt.vpi_vprintf = vpi_vprintf;
  vvpt.vpi_mcd_close = vpi_mcd_close;
  vvpt.vpi_mcd_name = vpi_mcd_name;
  vvpt.vpi_mcd_open = vpi_mcd_open;
  vvpt.vpi_mcd_open_x = vpi_mcd_open_x;
  vvpt.vpi_mcd_vprintf = vpi_mcd_vprintf;
  vvpt.vpi_mcd_fputc = vpi_mcd_fputc;
  vvpt.vpi_mcd_fgetc = vpi_mcd_fgetc;
  vvpt.vpi_register_cb = vpi_register_cb;
  vvpt.vpi_remove_cb = vpi_remove_cb;
  vvpt.vpi_sim_vcontrol = vpi_sim_vcontrol;
  vvpt.vpi_handle = vpi_handle;
  vvpt.vpi_iterate = vpi_iterate;
  vvpt.vpi_scan = vpi_scan;
  vvpt.vpi_handle_by_index = vpi_handle_by_index;
  vvpt.vpi_get_time = vpi_get_time;
  vvpt.vpi_get = vpi_get;
  vvpt.vpi_get_str = vpi_get_str;
  vvpt.vpi_get_value = vpi_get_value;
  vvpt.vpi_put_value = vpi_put_value;
  vvpt.vpi_free_object= vpi_free_object;
  vvpt.vpi_get_vlog_info = vpi_get_vlog_info;
}

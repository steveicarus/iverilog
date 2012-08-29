/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "fpga_priv.h"
# include  <string.h>
# include  <assert.h>

extern const struct device_s d_generic;
extern const struct device_s d_generic_edif;
extern const struct device_s d_lpm_edif;
extern const struct device_s d_virtex_edif;
extern const struct device_s d_virtex2_edif;


const struct device_table_s {
      const char*name;
      device_t driver;
} device_table[] = {
      { "generic-edif", &d_generic_edif },
      { "generic-xnf",  &d_generic },
      { "lpm",          &d_lpm_edif },
      { "virtex",       &d_virtex_edif },
      { "virtex2",      &d_virtex2_edif },
      { 0, 0 }
};

device_t device_from_arch(const char*arch)
{
      unsigned idx;

      assert(arch);

      for (idx = 0 ;  device_table[idx].name ;  idx += 1) {
	    if (strcmp(arch, device_table[idx].name) == 0)
		  return device_table[idx].driver;

      }

      return 0;
}

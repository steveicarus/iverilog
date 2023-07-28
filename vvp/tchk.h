#ifndef IVL_tchk_H
#define IVL_tchk_H
/*
 * Copyright (c) 2023 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2023 Leo Moser (leo.moser@pm.me)
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

# include  <stddef.h>
# include  "vvp_net.h"
# include  "schedule.h"
# include  "event.h"

class __vpiTchkWidth;

/*
 * The vvp_fun_tchk_width functor implements the $width timing check
 * The reference event is connected to port 0, the data event is the
 * inverse of the referenc event
 */
class vvp_fun_tchk_width : public vvp_net_fun_t {

    public:
      typedef unsigned short edge_t;
      explicit vvp_fun_tchk_width(edge_t start_edge, vvp_time64_t limit, vvp_time64_t threshold);
      virtual ~vvp_fun_tchk_width();

      __vpiTchkWidth* vpi_tchk;

    protected:
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);

      vvp_bit4_t bit_;

    private:
      edge_t start_edge_;
        // Time values already scaled for
        // higher performance
      vvp_time64_t limit_;
      vvp_time64_t threshold_;
      vvp_time64_t t1_;
      vvp_time64_t t2_;
      vvp_time64_t width_;

      void violation();

      friend __vpiTchkWidth;
};

#endif /* IVL_tchk_H */

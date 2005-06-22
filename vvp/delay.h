#ifndef __delay_H
#define __delay_H
/*
 * Copyright 2005 Stephen Williams
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: delay.h,v 1.8 2005/06/22 00:04:49 steve Exp $"
#endif

/*
 */

# include  "vvp_net.h"
# include  "schedule.h"

/*
 * Instances of this object are functions that calculate the delay for
 * the transition from the source vvp_bit4_t value to the destination
 * value.
 */
class vvp_delay_t {

    public:
      vvp_delay_t(vvp_time64_t rise, vvp_time64_t fall);
      vvp_delay_t(vvp_time64_t rise, vvp_time64_t fall, vvp_time64_t decay);
      ~vvp_delay_t();

      vvp_time64_t get_delay(vvp_bit4_t from, vvp_bit4_t to);

    private:
      vvp_time64_t rise_, fall_, decay_;
      vvp_time64_t min_delay_;
};

/* vvp_fun_delay
 * This is a lighter weight version of vvp_fun_drive, that only
 * carries delays. The output that it propagates is vvp_vector4_t so
 * drive strengths are lost, but then again it doesn't go through the
 * effort of calculating strength values either.
 *
 * The node needs a pointer to the vvp_net_t input so that it knows
 * how to find its output when propaging delayed output.
 */
class vvp_fun_delay  : public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      vvp_fun_delay(vvp_net_t*net, vvp_bit4_t init, const vvp_delay_t&d);
      ~vvp_fun_delay();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);
	//void recv_long(vvp_net_ptr_t port, long bit);

    private:
      virtual void run_run();

    private:
      vvp_net_t*net_;
      vvp_delay_t delay_;
      vvp_vector4_t cur_;
};

/*
 * $Log: delay.h,v $
 * Revision 1.8  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.7  2005/06/02 16:02:11  steve
 *  Add support for notif0/1 gates.
 *  Make delay nodes support inertial delay.
 *  Add the %force/link instruction.
 *
 * Revision 1.6  2005/05/14 19:43:23  steve
 *  Move functor delays to vvp_delay_fun object.
 *
 * Revision 1.5  2005/04/03 05:45:51  steve
 *  Rework the vvp_delay_t class.
 *
 */
#endif // __delay_H

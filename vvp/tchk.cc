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

# include  "tchk.h"

# include  "compile.h"
# include  "schedule.h"
# include  "vvp_net_sig.h"
# include  "event.h"

#include <iostream>

// Trigger cbTchkViolation
extern void vpiTchkViolation(vpiHandle tchk);

vvp_fun_tchk_width::vvp_fun_tchk_width(edge_t start_edge, vvp_time64_t limit, vvp_time64_t threshold)
: bit_(BIT4_X), start_edge_(start_edge), limit_(limit), threshold_(threshold), t1_(0), t2_(0), width_(0)
{
      if (start_edge != vvp_edge_posedge && start_edge != vvp_edge_negedge) {
	    std::cout << "tchk error: invalid reference edge" << std::endl;
	    assert(0);
      }
}

vvp_fun_tchk_width::~vvp_fun_tchk_width()
{
}

void vvp_fun_tchk_width::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                   vvp_context_t)
{
        // Port 0 is reference input
      if (port.port() == 0) {

	      // See what kind of edge this represents.
	    edge_t mask = VVP_EDGE(bit_, bit.value(0));

	      // Save the current input for the next time around.
	    bit_ = bit.value(0);

	    if (start_edge_ & mask) {
		  t1_ = schedule_simtime();
	    }

	    if (!(start_edge_ & mask)) {
		  t2_ = schedule_simtime();

		  width_ = t2_ - t1_;
		  if (width_ < limit_ && width_ > threshold_) {
			violation();
		  }
	    }
      }
}

void vvp_fun_tchk_width::violation()
{
      vpiHandle scope = vpi_tchk->vpi_handle(vpiScope);
      char* fullname = vpi_get_str(vpiFullName, scope);
      std::string violation_message;

      assert(vpi_tchk->file_idx_ < file_names.size());

      std::string time_unit("ps"); // For now let's just print everything in ps

	// Build the violation message
      violation_message += "Timing violation!\n";
      violation_message += "\t$width( ";
      if (start_edge_ == vvp_edge_posedge) violation_message += "posedge ";
      else violation_message += "negedge ";
      violation_message += vpi_get_str(vpiName, vpi_tchk->vpi_reference_);
      violation_message += ":" + std::to_string(t1_) + " " + time_unit + ", ";
      violation_message += std::to_string(limit_) + " " + time_unit + " : ";
      violation_message += std::to_string(width_);
      violation_message +=" "+time_unit+", ";
      violation_message += std::to_string(threshold_) + " " + time_unit + ", ";
      violation_message += vpi_get_str(vpiName, vpi_tchk->vpi_notifier_);
      violation_message += " );\n";
      violation_message += "\tfile_idx = ";
      violation_message += file_names[vpi_tchk->file_idx_];
      violation_message += " line = " + std::to_string(vpi_tchk->lineno_) + "\n";
      violation_message += "\tScope: ";
      violation_message += fullname;
      violation_message += "\n";
      violation_message += "\tTime: ";
      violation_message += std::to_string(t2_);
      violation_message += " "+time_unit;

	// Print the violation message
      std::cout << violation_message << std::endl;

      if (vpi_tchk->vpi_notifier_) {

	    __vpiSignal* vpi_notifier_sig = dynamic_cast<__vpiSignal*> (vpi_tchk->vpi_notifier_);

	    if (vpi_notifier_sig == 0) {
		  std::cout << "tchk error: notifier not a signal?" << std::endl;
		  assert(vpi_notifier_sig);
	    }

	    vvp_vector4_t sig_value;

	      // Is it a signal functor?
	    vvp_signal_value* sig = dynamic_cast<vvp_signal_value*> (vpi_notifier_sig->node->fil);
	    if (sig == 0) {
		  std::cout << "tchk error: notifier not a signal?" << std::endl;
		  assert(sig);
	    }

	      // Extract the value from the signal
	    sig->vec4_value(sig_value);

	      // If notifier is z, nothing is to do
	    if (sig_value.value(0) == BIT4_Z) return;

	      // Update notifier value
	    if (sig_value.value(0) != BIT4_1) {
		  vvp_net_ptr_t ptr (vpi_notifier_sig->node, 0);
		  vvp_send_vec4(ptr, vvp_vector4_t(1, BIT4_1), nullptr);
	    } else {
		  vvp_net_ptr_t ptr (vpi_notifier_sig->node, 0);
		  vvp_send_vec4(ptr, vvp_vector4_t(1, BIT4_0), nullptr);
	    }
      }

      // Trigger cbTchkViolation
      vpiTchkViolation(vpi_tchk);
}

/*
 * Copyright (c) 2023-2026 Stephen Williams (steve@icarus.com)
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

# include "tchk.h"
# include "compile.h"
# include "schedule.h"
# include "vpi_priv.h"
# include "vvp_net_sig.h"
# include <cmath>

bool vpip_tchk_delay_to_ticks(double value, const __vpiScope* scope,
                               vvp_time64_t& ticks)
{
      if (!scope || !std::isfinite(value) || value < 0) return false;
      int local_shift = scope->time_units - scope->time_precision;
      int global_shift = scope->time_precision - vpip_get_time_precision();
      if (local_shift < 0 || global_shift < 0) return false;
      double scaled = value * pow(10.0, local_shift);
      if (!std::isfinite(scaled)) return false;
      // Match the ordinary module-delay rounding before checking overflow.
      double rounded = floor(scaled + 0.5);
      long double total = (long double) rounded * powl(10.0L, global_shift);
      if (!std::isfinite(total) ||
          total >= ldexpl(1.0L, 64))
            return false;
      ticks = vpip_scaled_real_to_time64(value, scope);
      return true;
}

vvp_fun_tchk_width::vvp_fun_tchk_width(int start_edge,
                                      vvp_time64_t limit,
                                      vvp_time64_t threshold)
: vpi_tchk(nullptr), previous_(BIT4_X), start_edge_(start_edge),
  close_edge_(start_edge == 1 ? -1 : 1),
  limit_(limit), threshold_(threshold), opening_time_(0), have_opening_(false)
{
      assert(start_edge == 1 || start_edge == -1);
}

void vvp_fun_tchk_width::recv_vec4(vvp_net_ptr_t port,
                                  const vvp_vector4_t& value, vvp_context_t)
{
      assert(port.port() == 0 && value.size() == 1);
      int transition = edge(previous_, value.value(0));
      previous_ = value.value(0);
      if (transition == start_edge_) {
            opening_time_ = schedule_simtime();
            have_opening_ = true;
      } else if ((transition == close_edge_) && have_opening_) {
            vvp_time64_t now = schedule_simtime();
            vvp_time64_t width = now - opening_time_;
            if (threshold_ < width && width < limit_)
                  violation(now, width);
      }
}

/* Both timing checks deliver the declared notifier and actual timing callback.
 * Retain #999's notifier convention without making a notifier mandatory. */
static void notify_timing_violation(vpiHandle check, vpiHandle notifier_handle)
{
      if (notifier_handle) {
            __vpiSignal* notifier = dynamic_cast<__vpiSignal*>(notifier_handle);
            assert(notifier);
            vvp_signal_value* signal =
                  dynamic_cast<vvp_signal_value*>(notifier->node->fil);
            assert(signal);
            vvp_vector4_t old;
            signal->vec4_value(old);
            assert(old.size() == 1);
            // A Z notifier does not toggle, but must not suppress the callback.
            if (old.value(0) != BIT4_Z) {
                  vvp_bit4_t next = old.value(0) == BIT4_1 ? BIT4_0 : BIT4_1;
                  vvp_send_vec4(vvp_net_ptr_t(notifier->node, 0),
                                vvp_vector4_t(1, next), nullptr);
            }
      }
      vpip_tchk_violation(check);
}

void vvp_fun_tchk_width::violation(vvp_time64_t now, vvp_time64_t width)
{
      assert(vpi_tchk && vpi_tchk->file_idx_ < file_names.size());
      __vpiScope* scope = vpi_tchk->scope_;
      vpi_printf("Timing violation: $width at %g (module units), "
                 "width=%g limit=%g threshold=%g, %s:%u, scope=%s\n",
                 vpip_time_to_scaled_real(now, scope),
                 vpip_time_to_scaled_real(width, scope),
                 vpip_time_to_scaled_real(limit_, scope),
                 vpip_time_to_scaled_real(threshold_, scope),
                 file_names[vpi_tchk->file_idx_], vpi_tchk->lineno_,
                 vpi_get_str(vpiFullName, scope));

      notify_timing_violation(vpi_tchk, vpi_tchk->notifier_);
}

vvp_fun_tchk_period::vvp_fun_tchk_period(int reference_edge, vvp_time64_t limit)
: vpi_tchk(nullptr), previous_(BIT4_X), reference_edge_(reference_edge),
  limit_(limit), reference_time_(0), have_reference_(false)
{
      assert(reference_edge == 1 || reference_edge == -1);
}

void vvp_fun_tchk_period::recv_vec4(vvp_net_ptr_t port,
                                   const vvp_vector4_t& value, vvp_context_t)
{
      assert(port.port() == 0 && value.size() == 1);
      int transition = edge(previous_, value.value(0));
      previous_ = value.value(0);
      if (transition != reference_edge_) return;

      vvp_time64_t now = schedule_simtime();
      if (have_reference_) {
            vvp_time64_t period = now - reference_time_;
            if (period < limit_) violation(now, period);
      }
      reference_time_ = now;
      have_reference_ = true;
}

void vvp_fun_tchk_period::violation(vvp_time64_t now, vvp_time64_t period)
{
      assert(vpi_tchk && vpi_tchk->file_idx_ < file_names.size());
      __vpiScope* scope = vpi_tchk->scope_;
      vpi_printf("Timing violation: $period at %g (module units), "
                 "period=%g limit=%g, %s:%u, scope=%s\n",
                 vpip_time_to_scaled_real(now, scope),
                 vpip_time_to_scaled_real(period, scope),
                 vpip_time_to_scaled_real(limit_, scope),
                 file_names[vpi_tchk->file_idx_], vpi_tchk->lineno_,
                 vpi_get_str(vpiFullName, scope));
      notify_timing_violation(vpi_tchk, vpi_tchk->notifier_);
}

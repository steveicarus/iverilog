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

/* Timing checks deliver the declared notifier and actual timing callback.
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


vvp_fun_tchk_two_event::vvp_fun_tchk_two_event(int kind, int reference_edge,
                                              int data_edge,
                                              vvp_time64_t limit,
                                              unsigned reference_condition_value,
                                              unsigned data_condition_value)
: vpi_tchk(nullptr), kind_(kind), previous_{BIT4_X, BIT4_X},
  condition_values_{reference_condition_value, data_condition_value},
  reference_edge_(reference_edge), data_edge_(data_edge), limit_(limit),
  timestamp_(0), have_timestamp_(false)
{
      assert(kind == vpiSetup || kind == vpiHold ||
             kind == vpiRecovery || kind == vpiRemoval);
      assert(reference_edge >= -1 && reference_edge <= 1);
      assert(data_edge >= -1 && data_edge <= 1);
      assert(reference_condition_value <= 1 && data_condition_value <= 1);
}

// Qualifiers do not have event inputs. Read the scalar at the physical event,
// avoiding a separately cached condition whose value would depend on fanout
// order when the qualifier aliases a terminal.
static bool two_event_condition_satisfied(vpiHandle condition, unsigned wanted)
{
      if (!condition) return true;
      s_vpi_value value;
      value.format = vpiScalarVal;
      vpi_get_value(condition, &value);
      return value.value.scalar == (wanted ? vpi1 : vpi0);
}

const char* vvp_fun_tchk_two_event::name() const
{
      switch (kind_) {
          case vpiSetup: return "setup";
          case vpiHold: return "hold";
          case vpiRecovery: return "recovery";
          case vpiRemoval: return "removal";
          default: assert(0); return "unknown";
      }
}

static bool tchk_event_matches(int selected_edge, vvp_bit4_t before,
                               vvp_bit4_t after)
{
      if (before == after) return false;
      return selected_edge == 0 || edge(before, after) == selected_edge;
}

void vvp_fun_tchk_two_event::recv_vec4(vvp_net_ptr_t port,
                                       const vvp_vector4_t& value,
                                       vvp_context_t)
{
      assert(port.port() < 2 && value.size() == 1 && vpi_tchk);
      unsigned index = port.port();
      vvp_bit4_t next = value.value(0);
      __vpiSignal* reference = dynamic_cast<__vpiSignal*>(vpi_tchk->reference_);
      __vpiSignal* data = dynamic_cast<__vpiSignal*>(vpi_tchk->data_);
      assert(reference && data);

      // If both terminals name the same signal, one physical delivery supplies
      // both roles. Do not let the duplicate fanout callbacks choose their order.
      if (reference->node == data->node) {
            if (index != 0) return;
            bool ref_event = tchk_event_matches(reference_edge_, previous_[0], next);
            bool data_event = tchk_event_matches(data_edge_, previous_[0], next);
            previous_[0] = previous_[1] = next;
            // Sample both roles before a comparison can change a notifier.
            ref_event = ref_event && two_event_condition_satisfied(
                  vpi_tchk->reference_condition_, condition_values_[0]);
            data_event = data_event && two_event_condition_satisfied(
                  vpi_tchk->data_condition_, condition_values_[1]);
            process_events(ref_event, data_event);
            return;
      }

      int selected_edge = index == 0 ? reference_edge_ : data_edge_;
      bool matched = tchk_event_matches(selected_edge, previous_[index], next);
      previous_[index] = next;
      vpiHandle condition = index == 0 ? vpi_tchk->reference_condition_
                                      : vpi_tchk->data_condition_;
      matched = matched && two_event_condition_satisfied(condition,
                                                         condition_values_[index]);
      if (matched) process_events(index == 0, index == 1);
}

void vvp_fun_tchk_two_event::process_events(bool reference_event, bool data_event)
{
      // SETUP and REMOVAL timestamp data, then check reference. HOLD and
      // RECOVERY timestamp reference, then check data. RR reference is the
      // asynchronous control; data is the clock, matching the HDL/VPI roles.
      bool timestamp_data = kind_ == vpiSetup || kind_ == vpiRemoval;
      bool timestamp_event = timestamp_data ? data_event : reference_event;
      bool timecheck_event = timestamp_data ? reference_event : data_event;
      vvp_time64_t now = schedule_simtime();
      if (timestamp_event) {
            timestamp_ = now;
            have_timestamp_ = true;
      }
      if (!timecheck_event || !have_timestamp_) return;

      vvp_time64_t interval = now - timestamp_;
      // SETUP/REMOVAL exclude zero; HOLD/RECOVERY include it. Upper equality
      // is safe for all four. Missing history is distinct from a real t=0 event.
      bool include_zero = kind_ == vpiHold || kind_ == vpiRecovery;
      if ((include_zero || interval != 0) && interval < limit_)
            violation(now, interval);
}

void vvp_fun_tchk_two_event::violation(vvp_time64_t now, vvp_time64_t interval)
{
      assert(vpi_tchk && vpi_tchk->file_idx_ < file_names.size());
      __vpiScope* scope = vpi_tchk->scope_;
      vpi_printf("Timing violation: $%s at %g (module units), "
                 "interval=%g limit=%g, %s:%u, scope=%s\n",
                 name(),
                 vpip_time_to_scaled_real(now, scope),
                 vpip_time_to_scaled_real(interval, scope),
                 vpip_time_to_scaled_real(limit_, scope),
                 file_names[vpi_tchk->file_idx_], vpi_tchk->lineno_,
                 vpi_get_str(vpiFullName, scope));
      notify_timing_violation(vpi_tchk, vpi_tchk->notifier_);
}


vvp_fun_tchk_recrem::vvp_fun_tchk_recrem(int reference_edge, int data_edge,
                                               vvp_time64_t recovery_limit,
                                               vvp_time64_t removal_limit)
: vpi_tchk(nullptr), previous_{BIT4_X, BIT4_X},
  reference_edge_(reference_edge), data_edge_(data_edge),
  recovery_limit_(recovery_limit), removal_limit_(removal_limit),
  reference_time_(0), data_time_(0), have_reference_(false), have_data_(false)
{
      assert(reference_edge >= -1 && reference_edge <= 1);
      assert(data_edge >= -1 && data_edge <= 1);
}

void vvp_fun_tchk_recrem::recv_vec4(vvp_net_ptr_t port,
                                       const vvp_vector4_t& value,
                                       vvp_context_t)
{
      assert(port.port() < 2 && value.size() == 1 && vpi_tchk);
      unsigned index = port.port();
      vvp_bit4_t next = value.value(0);
      __vpiSignal* reference = dynamic_cast<__vpiSignal*>(vpi_tchk->reference_);
      __vpiSignal* data = dynamic_cast<__vpiSignal*>(vpi_tchk->data_);
      assert(reference && data);

      // As in the standalone checks, aliases name one physical delivery.
      // Combine both roles before notifying, not two duplicate fanout visits.
      if (reference->node == data->node) {
            if (index != 0) return;
            bool ref_event = tchk_event_matches(reference_edge_, previous_[0], next);
            bool data_event = tchk_event_matches(data_edge_, previous_[0], next);
            previous_[0] = previous_[1] = next;
            process_events(ref_event, data_event);
            return;
      }

      int selected_edge = index == 0 ? reference_edge_ : data_edge_;
      bool matched = tchk_event_matches(selected_edge, previous_[index], next);
      previous_[index] = next;
      if (matched) process_events(index == 0, index == 1);
}

void vvp_fun_tchk_recrem::process_events(bool reference_event, bool data_event)
{
      vvp_time64_t now = schedule_simtime();
      if (reference_event) {
            reference_time_ = now;
            have_reference_ = true;
      }
      if (data_event) {
            data_time_ = now;
            have_data_ = true;
      }

      // With two positive limits, simultaneous events violate in either
      // order. With a zero limit, use the corresponding standalone window:
      // recovery includes zero; removal excludes it. Both-zero never fails.
      bool recovery_violation = false;
      bool removal_violation = false;
      vvp_time64_t recovery_interval = 0, removal_interval = 0;
      if (data_event && have_reference_) {
            recovery_interval = now - reference_time_;
            recovery_violation = recovery_interval < recovery_limit_;
      }
      if (reference_event && have_data_) {
            removal_interval = now - data_time_;
            bool include_zero = recovery_limit_ != 0 && removal_limit_ != 0;
            removal_violation = (include_zero || removal_interval != 0)
                              && removal_interval < removal_limit_;
      }
      // A declaration produces one notification even when an aliased
      // delivery supplies both roles and violates both positive windows.
      if (recovery_violation || removal_violation)
            violation(now, recovery_violation,
                      recovery_violation ? recovery_interval : removal_interval);
}

void vvp_fun_tchk_recrem::violation(vvp_time64_t now, bool recovery,
                                       vvp_time64_t interval)
{
      assert(vpi_tchk && vpi_tchk->file_idx_ < file_names.size());
      __vpiScope* scope = vpi_tchk->scope_;
      vpi_printf("Timing violation: $recrem %s at %g (module units), "
                 "interval=%g limit=%g, %s:%u, scope=%s\n",
                 recovery ? "recovery" : "removal",
                 vpip_time_to_scaled_real(now, scope),
                 vpip_time_to_scaled_real(interval, scope),
                 vpip_time_to_scaled_real(recovery ? recovery_limit_ : removal_limit_, scope),
                 file_names[vpi_tchk->file_idx_], vpi_tchk->lineno_,
                 vpi_get_str(vpiFullName, scope));
      notify_timing_violation(vpi_tchk, vpi_tchk->notifier_);
}

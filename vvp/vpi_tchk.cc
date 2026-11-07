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

# include "tchk.h"
# include "vpi_priv.h"

__vpiTchkWidth::__vpiTchkWidth(__vpiScope* scope, unsigned file_idx,
                               unsigned lineno, vvp_fun_tchk_width* fun)
: reference_(nullptr), notifier_(nullptr), scope_(scope), file_idx_(file_idx),
  lineno_(lineno), fun_(fun), reference_term_(nullptr)
{
}

__vpiTchkWidth::~__vpiTchkWidth()
{
      delete reference_term_;
}

int __vpiTchkWidth::vpi_get(int code)
{
      switch (code) {
          case vpiTchkType: return vpiWidth;
          case vpiLineNo: return lineno_;
          default: return vpiUndefined;
      }
}

vpiHandle __vpiTchkWidth::vpi_handle(int code)
{
      switch (code) {
          case vpiScope: return scope_;
          case vpiModule: return scope_;
          case vpiTchkNotifier: return notifier_;
          case vpiTchkRefTerm:
            if (!reference_term_ && reference_)
                  reference_term_ = new __vpiTchkRefTerm(
                        fun_->start_edge_ == 1 ? vpiPosedge : vpiNegedge,
                        reference_);
            return reference_term_;
          case vpiTchkDataTerm: return nullptr;
          default: return nullptr;
      }
}

void __vpiTchkWidth::vpi_get_delays(p_vpi_delay delays)
{
      if (!delays || delays->no_of_delays != 1 || !delays->da) return;
      delays->da[0].type = delays->time_type;
      if (delays->time_type == vpiScaledRealTime) {
            delays->da[0].real = vpip_time_to_scaled_real(fun_->limit_, scope_);
      } else if (delays->time_type == vpiSimTime) {
            delays->da[0].high = fun_->limit_ >> 32;
            delays->da[0].low = fun_->limit_ & 0xffffffff;
      }
}

void __vpiTchkWidth::vpi_put_delays(p_vpi_delay delays)
{
      if (!delays || delays->no_of_delays != 1 || !delays->da ||
          delays->mtm_flag || delays->append_flag || delays->pulsere_flag) return;
      vvp_time64_t value;
      if (delays->time_type == vpiScaledRealTime) {
            if (!vpip_tchk_delay_to_ticks(delays->da[0].real, scope_, value)) {
                  vpi_printf("VPI ERROR: invalid WIDTH limit\n");
                  return;
            }
      } else if (delays->time_type == vpiSimTime) {
            value = ((vvp_time64_t) delays->da[0].high << 32) | delays->da[0].low;
      } else return;
      fun_->limit_ = value;
}

__vpiTchkPeriod::__vpiTchkPeriod(__vpiScope* scope, unsigned file_idx,
                               unsigned lineno, vvp_fun_tchk_period* fun)
: reference_(nullptr), notifier_(nullptr), scope_(scope), file_idx_(file_idx),
  lineno_(lineno), fun_(fun), reference_term_(nullptr)
{
}

__vpiTchkPeriod::~__vpiTchkPeriod()
{
      delete reference_term_;
}

int __vpiTchkPeriod::vpi_get(int code)
{
      switch (code) {
          case vpiTchkType: return vpiPeriod;
          case vpiLineNo: return lineno_;
          default: return vpiUndefined;
      }
}

vpiHandle __vpiTchkPeriod::vpi_handle(int code)
{
      switch (code) {
          case vpiScope: return scope_;
          case vpiModule: return scope_;
          case vpiTchkNotifier: return notifier_;
          case vpiTchkRefTerm:
            if (!reference_term_ && reference_)
                  reference_term_ = new __vpiTchkRefTerm(
                        fun_->reference_edge_ == 1 ? vpiPosedge : vpiNegedge,
                        reference_);
            return reference_term_;
          case vpiTchkDataTerm: return nullptr;
          default: return nullptr;
      }
}

void __vpiTchkPeriod::vpi_get_delays(p_vpi_delay delays)
{
      if (!delays || delays->no_of_delays != 1 || !delays->da) return;
      delays->da[0].type = delays->time_type;
      if (delays->time_type == vpiScaledRealTime) {
            delays->da[0].real = vpip_time_to_scaled_real(fun_->limit_, scope_);
      } else if (delays->time_type == vpiSimTime) {
            delays->da[0].high = fun_->limit_ >> 32;
            delays->da[0].low = fun_->limit_ & 0xffffffff;
      }
}

void __vpiTchkPeriod::vpi_put_delays(p_vpi_delay delays)
{
      if (!delays || delays->no_of_delays != 1 || !delays->da ||
          delays->mtm_flag || delays->append_flag || delays->pulsere_flag) return;
      vvp_time64_t value;
      if (delays->time_type == vpiScaledRealTime) {
            if (!vpip_tchk_delay_to_ticks(delays->da[0].real, scope_, value)) {
                  vpi_printf("VPI ERROR: invalid PERIOD limit\n");
                  return;
            }
      } else if (delays->time_type == vpiSimTime) {
            value = ((vvp_time64_t) delays->da[0].high << 32) | delays->da[0].low;
      } else return;
      fun_->limit_ = value;
}

int __vpiTchkRefTerm::vpi_get(int code)
{
      return code == vpiEdge ? edge_ : vpiUndefined;
}

vpiHandle __vpiTchkRefTerm::vpi_handle(int code)
{
      return code == vpiExpr ? expr_ : nullptr;
}

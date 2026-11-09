#ifndef IVL_tchk_H
#define IVL_tchk_H
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

# include "vvp_net.h"

class __vpiScope;
class __vpiTchkWidth;
class __vpiTchkPeriod;
class __vpiTchkTwoEvent;
class __vpiTchkRecRem;

// The record values use the declaring module's units. Reject values that
// cannot be rounded to its precision and represented in simulator ticks.
bool vpip_tchk_delay_to_ticks(double value, const __vpiScope* scope,
                               vvp_time64_t& ticks);

class vvp_fun_tchk_width : public vvp_net_fun_t {
    public:
      vvp_fun_tchk_width(int start_edge, vvp_time64_t limit,
                         vvp_time64_t threshold);
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t& value,
                     vvp_context_t) override;
      __vpiTchkWidth* vpi_tchk;

    private:
      vvp_bit4_t previous_;
      int start_edge_;
      int close_edge_;
      vvp_time64_t limit_;
      vvp_time64_t threshold_;
      vvp_time64_t opening_time_;
      bool have_opening_;
      void violation(vvp_time64_t now, vvp_time64_t width);
      friend class __vpiTchkWidth;
};

class vvp_fun_tchk_period : public vvp_net_fun_t {
    public:
      vvp_fun_tchk_period(int reference_edge, vvp_time64_t limit);
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t& value,
                     vvp_context_t) override;
      __vpiTchkPeriod* vpi_tchk;

    private:
      vvp_bit4_t previous_;
      int reference_edge_;
      vvp_time64_t limit_;
      vvp_time64_t reference_time_;
      bool have_reference_;
      void violation(vvp_time64_t now, vvp_time64_t period);
      friend class __vpiTchkPeriod;
};

/* Standalone SETUP/HOLD and RECOVERY/REMOVAL share terminal/limit storage.
 * Each kind selects its timestamp role and lower endpoint explicitly. */
class vvp_fun_tchk_two_event : public vvp_net_fun_t {
    public:
      vvp_fun_tchk_two_event(int kind, int reference_edge, int data_edge,
                             vvp_time64_t limit, unsigned reference_condition_value,
                             unsigned data_condition_value);
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t& value,
                     vvp_context_t) override;
      __vpiTchkTwoEvent* vpi_tchk;

    private:
      int kind_;
      const char* name() const;
      vvp_bit4_t previous_[2];
      unsigned condition_values_[2];
      int reference_edge_;
      int data_edge_;
      vvp_time64_t limit_;
      vvp_time64_t timestamp_;
      bool have_timestamp_;
      void process_events(bool reference_event, bool data_event);
      void violation(vvp_time64_t now, vvp_time64_t interval);
      friend class __vpiTchkTwoEvent;
};

/* A combined declaration is one observable timing check, with independent
 * reference/data histories and recovery/removal limits. */
class vvp_fun_tchk_recrem : public vvp_net_fun_t {
    public:
      vvp_fun_tchk_recrem(int reference_edge, int data_edge,
                             vvp_time64_t recovery_limit, vvp_time64_t removal_limit);
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t& value,
                     vvp_context_t) override;
      __vpiTchkRecRem* vpi_tchk;

    private:
      vvp_bit4_t previous_[2];
      int reference_edge_, data_edge_;
      vvp_time64_t recovery_limit_, removal_limit_;
      vvp_time64_t reference_time_, data_time_;
      bool have_reference_, have_data_;
      void process_events(bool reference_event, bool data_event);
      void violation(vvp_time64_t now, bool recovery, vvp_time64_t interval);
      friend class __vpiTchkRecRem;
};

#endif

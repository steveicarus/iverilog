/*
 * Copyright (c) 2006-2023 Stephen Williams <steve@icarus.com>
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

# include  "PTimingCheck.h"

PRecRem::PRecRem(event_t reference_event,
                    event_t data_event,
                    //PExpr setup_limit,
                    //PExpr hold_limit,
                    pform_name_t* notifier,
                    pform_name_t* timestamp_cond,
                    pform_name_t* timecheck_cond,
                    pform_name_t* delayed_reference,
                    pform_name_t* delayed_data)
                    :
                    reference_event_ (reference_event),
                    data_event_ (data_event),
                    //setup_limit (setup_limit),
                    //hold_limit (hold_limit),
                    notifier_ (notifier),
                    timestamp_cond_ (timestamp_cond),
                    timecheck_cond_ (timecheck_cond),
                    delayed_reference_ (delayed_reference),
                    delayed_data_ (delayed_data)
{
}

PRecRem::~PRecRem()
{
      // Delete optional arguments
      if (reference_event_.condition) delete reference_event_.condition;
      if (data_event_.condition) delete data_event_.condition;

      if(notifier_) delete notifier_;

      if(timestamp_cond_) delete timestamp_cond_;
      if(timecheck_cond_) delete timecheck_cond_;

      if(delayed_reference_) delete delayed_reference_;
      if(delayed_data_) delete delayed_data_;
}

PSetupHold::PSetupHold(event_t reference_event,
                    event_t data_event,
                    //PExpr setup_limit,
                    //PExpr hold_limit,
                    pform_name_t* notifier,
                    pform_name_t* timestamp_cond,
                    pform_name_t* timecheck_cond,
                    pform_name_t* delayed_reference,
                    pform_name_t* delayed_data)
                    :
                    reference_event_ (reference_event),
                    data_event_ (data_event),
                    //setup_limit (setup_limit),
                    //hold_limit (hold_limit),
                    notifier_ (notifier),
                    timestamp_cond_ (timestamp_cond),
                    timecheck_cond_ (timecheck_cond),
                    delayed_reference_ (delayed_reference),
                    delayed_data_ (delayed_data)
{
}

PSetupHold::~PSetupHold()
{
      // Delete optional arguments
      if (reference_event_.condition) delete reference_event_.condition;
      if (data_event_.condition) delete data_event_.condition;

      if(notifier_) delete notifier_;

      if(timestamp_cond_) delete timestamp_cond_;
      if(timecheck_cond_) delete timecheck_cond_;

      if(delayed_reference_) delete delayed_reference_;
      if(delayed_data_) delete delayed_data_;
}

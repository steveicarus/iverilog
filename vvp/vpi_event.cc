/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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

# include  "compile.h"
# include  "vpi_priv.h"
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <cassert>

static int named_event_get(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNamedEvent));

      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)ref;

      switch (code) {

	  case vpiAutomatic:
	    return (int) obj->scope->is_automatic;
      }

      return 0;
}

static char* named_event_get_str(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNamedEvent));

      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)ref;

      if (code == vpiFile) {  // Not implemented for now!
	    return simple_set_rbuf_str(file_names[0]);
      }
      return generic_get_str(code, &obj->scope->base, obj->name, NULL);
}

static vpiHandle named_event_get_handle(int code, vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNamedEvent));

      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)ref;

      switch (code) {
	  case vpiScope:
	    return &obj->scope->base;

	  case vpiModule:
	    return vpip_module(obj->scope);
      }

      return 0;
}

static const struct __vpirt vpip_named_event_rt = {
      vpiNamedEvent,

      named_event_get,
      named_event_get_str,
      0,
      0,

      named_event_get_handle,
      0,
      0,

      0
};

vpiHandle vpip_make_named_event(const char*name, vvp_net_t*funct)
{
      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)
	    malloc(sizeof(struct __vpiNamedEvent));

      obj->base.vpi_type = &vpip_named_event_rt;
      obj->name = vpip_name_string(name);
      obj->scope = vpip_peek_current_scope();
      obj->funct = funct;
      obj->callbacks = 0;

      return &obj->base;
}

/*
 * This function runs the callbacks for a named event. All the
 * callbacks are listed in the callback member of the event handle,
 * this function scans that list.
 *
 * This also handles the case where the callback has been removed. The
 * vpi_remove_cb doesn't actually remove any callbacks, it marks them
 * as canceled by clearing the cb_rtn function. This function reaps
 * those marked handles when it scans the list.
 *
 * We can not use vpi_free_object() here since it does not really
 * delete the callback.
 */
void vpip_run_named_event_callbacks(vpiHandle ref)
{
      assert((ref->vpi_type->type_code==vpiNamedEvent));

      struct __vpiNamedEvent*obj = (struct __vpiNamedEvent*)ref;

      struct __vpiCallback*next = obj->callbacks;
      struct __vpiCallback*prev = 0;
      while (next) {
	    struct __vpiCallback*cur = next;
	    next = cur->next;

	    if (cur->cb_data.cb_rtn != 0) {
		  callback_execute(cur);
		  prev = cur;

	    } else if (prev == 0) {
		  obj->callbacks = next;
		  cur->next = 0;
		  delete cur;

	    } else {
		  assert(prev->next == cur);
		  prev->next = next;
		  cur->next = 0;
		  delete cur;
	    }
      }
}

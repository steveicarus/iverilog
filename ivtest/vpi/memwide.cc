/*
 * Copyright (c) 2002  Michael Ruff (mruff at chiaro.com)
 *                     Michael Runyan (mrunyan at chiaro.com)
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

/*
 * This test verifies named events can be peeked and poked.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vpi_user.h"

static s_vpi_time suppress_time = { vpiSuppressTime, 0, 0, 0 };

static vpiHandle findReg(const char *name_);
static vpiHandle findMem(const char *name_);

extern "C" PLI_INT32
CallbackPeek(s_cb_data * /*data*/)
{
	vpiHandle handle;

	vpi_printf("!!!C++:     callback\n");

	// Find big_reg
	if((handle=findReg("big_reg"))) {
		unsigned size=vpi_get(vpiSize,handle);
		vpi_printf("!!!C++:    %s size is %d\n",vpi_get_str(vpiName,handle),size);

		s_vpi_value value;

		value.format=vpiVectorVal;
		value.value.vector=NULL;

		vpi_get_value(handle,&value);

		for(unsigned i=0;(i*32)<size;i++) {
			vpi_printf("Vec %d) %08x %08x\n",i,(int)value.value.vector[i].aval,(int)value.value.vector[i].bval);
		}
	} else {
		vpi_printf("***ERROR: can't find register - big_reg\n");
	}

	// Find my_mem
	if((handle=findMem("my_mem"))) {
		unsigned size=vpi_get(vpiSize,handle);
		vpi_printf("!!!C++:    %s size is %d\n",vpi_get_str(vpiName,handle),size);
		vpi_printf("!!!C++:    fullname is %s\n", vpi_get_str(vpiFullName,handle));

		s_vpi_value value;

		value.format=vpiVectorVal;
		value.value.vector=NULL;

		vpi_get_value(handle,&value);

		for(unsigned i=0;(i*32)<size;i++) {
			vpi_printf("Vec %d) %08x %08x\n",i,(int)value.value.vector[i].aval,(int)value.value.vector[i].bval);
		}
	} else {
		vpi_printf("***ERROR: can't find register - big_reg\n");
	}

	return 0;
}

static vpiHandle
findReg(const char *name_)
{
	vpiHandle mod_i, mod_h, reg_i, reg_h = NULL;
	char full[8096];

	// get top module handle
	mod_i = vpi_iterate(vpiModule, NULL);
	if (mod_i != NULL) {
		mod_h = vpi_scan(mod_i);
		vpi_free_object(mod_i);

		// find named event
		reg_i = vpi_iterate(vpiReg, mod_h);
		if (reg_i != NULL) {
			while ((reg_h = vpi_scan(reg_i))) {
				if (!strcmp(name_,
				    vpi_get_str(vpiName, reg_h))) {
				    strcpy(full,vpi_get_str(vpiFullName,reg_h));
				    vpi_printf("!!!C++:    %s fullname is %s\n",
					vpi_get_str(vpiName,reg_h), full);
				    vpi_free_object(reg_i);
				    break;
				}
			}
		}
	}
	return (reg_h);
}

static vpiHandle
findMem(const char *name_)
{
    vpiHandle mod_i, mod_h, hand, mem_i, word_i, word_h, mem_h = NULL;
    char full[8096];

    mod_i = vpi_iterate(vpiModule, NULL);
    mod_h = vpi_scan(mod_i);
    vpi_free_object(mod_i);

    mem_i = vpi_iterate(vpiMemory, mod_h);
    if (mem_i != NULL) {
        while ((hand = vpi_scan(mem_i))) {
            if (!strcmp(name_, vpi_get_str(vpiName, hand))) {
		strcpy(full,vpi_get_str(vpiFullName,hand));
		vpi_printf("!!!C++:    %s fullname is %s\n",
		    vpi_get_str(vpiName,hand), full);
                mem_h = hand;
                vpi_free_object(mem_i);
                break;
            }
        }
    }

    word_i = vpi_iterate(vpiMemoryWord, mem_h);
    vpi_scan(word_i);
    word_h = vpi_scan(word_i);
    vpi_free_object(word_i);
    return word_h;
}

extern "C" PLI_INT32 SetupTrigger(s_cb_data * /*cb_data*/)
{
	s_cb_data vc_cb_data;
	vpiHandle handle;

	if ((handle=findReg("event_trigger"))) {
		// Register callback
		vc_cb_data.time = NULL;
		vc_cb_data.value = NULL;
		vc_cb_data.user_data = NULL;
		vc_cb_data.obj = handle;
		vc_cb_data.reason = cbValueChange;
		vc_cb_data.cb_rtn = CallbackPeek;
		vpi_register_cb(&vc_cb_data);
		vpi_printf("!!!C++:     Registered Value Change Callback for %s\n",
				   vpi_get_str(vpiName, handle));
	} else {
		vpi_printf("***ERROR: can't find register - event_trigger\n");
	}

	return (0);
}

extern "C" void my_Register(void)
{
	s_cb_data cb_data;
	cb_data.time = &suppress_time;
	cb_data.value = NULL;
	cb_data.user_data = (char *) NULL;
	cb_data.obj = NULL;

	vpi_printf("!!!C++:     Registering Callbacks\n");

	cb_data.reason = cbEndOfCompile;
	cb_data.cb_rtn = SetupTrigger;
	vpi_register_cb(&cb_data);
}

#ifdef __SUNPRO_CC
extern "C"
#endif
void (*vlog_startup_routines[]) () = {
my_Register, 0};

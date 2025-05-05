// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Based on http://en.wikipedia.org/wiki/Verilog_Procedural_Interface.
//
// Simple VPI plug-in to test the toolchain.

#include "vpi_user.h"

// Implements the increment system task
static PLI_INT32 increment(PLI_BYTE8 *userdata) {
  // Obtains a handle to the argument list
  vpiHandle systfref = vpi_handle(vpiSysTfCall, NULL);
  vpiHandle args_iter = vpi_iterate(vpiArgument, systfref);

  // Grabs the value of the first argument
  vpiHandle argh = vpi_scan(args_iter);
  struct t_vpi_value argval;
  argval.format = vpiIntVal;
  vpi_get_value(argh, &argval);

  int value = argval.value.integer;
  vpi_printf("Input %d\n", value);

  // Increments the value and puts it back as first argument
  argval.value.integer = value + 1;
  vpi_put_value(argh, &argval, NULL, vpiNoDelay);

  // Cleans up and returns.
  vpi_free_object(args_iter);
  return 0;
}

// Registers the $increment task with the system.
static void registerIncrementTask() {
  s_vpi_systf_data task;
  task.type = vpiSysTask;
  task.tfname = "$increment";
  task.calltf = increment;
  task.compiletf = 0;

  vpi_register_systf(&task);
}

// Registers the new system task here.
void (*vlog_startup_routines[]) () = {
  registerIncrementTask,
  0
};

//----------------------------------------------------------------------
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc. 
//   Copyright 2010-2011 Synopsys, Inc.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//----------------------------------------------------------------------

`ifndef UVM_VERSION_DEFINES_SVH
`define UVM_VERSION_DEFINES_SVH

// Version numbers to be used in creating version strings for printing
// or programmatic tesing against version numbers
`define UVM_NAME UVM
`define UVM_MAJOR_REV 1
`define UVM_MINOR_REV 1
`define UVM_FIX_REV d


// Whole version identifiers that can be used in `ifdefs and `ifndefs
// to do conditional compilation
`define UVM_VERSION_1_1
`define UVM_MAJOR_VERSION_1_1
`define UVM_FIX_VERSION_1_1_d
`define UVM_MAJOR_REV_1
`define UVM_MINOR_REV_1
`define UVM_FIX_REV_d

// When there is a FIX_REV, print as "M.mf"
// When there is NO FIX_REV, print as "M.m".
// Fix rev kind of string:
`define UVM_VERSION_STRING `"`UVM_NAME``-```UVM_MAJOR_REV``.```UVM_MINOR_REV`UVM_FIX_REV`"
// No fix rev kind of string:
//`define UVM_VERSION_STRING `"`UVM_NAME``-```UVM_MAJOR_REV``.```UVM_MINOR_REV```"

`endif // UVM_VERSION_DEFINES_SVH

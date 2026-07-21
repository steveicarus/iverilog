//----------------------------------------------------------------------
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
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
// This file undefs all macros that are defined by the UVM library. This can
// be used to load uvm into multiple scopes using a single compilation.

`undef UVM_BLOCKING_GET_IMP
`undef UVM_BLOCKING_GET_IMP_SFX
`undef UVM_BLOCKING_GET_PEEK_IMP
`undef UVM_BLOCKING_PEEK_IMP
`undef UVM_BLOCKING_PEEK_IMP_SFX
`undef UVM_BLOCKING_PUT_IMP
`undef UVM_BLOCKING_PUT_IMP_SFX
`undef UVM_BLOCKING_TRANSPORT_IMP
`undef UVM_BLOCKING_TRANSPORT_IMP_SFX
`undef DODEEPCOPY
`undef DOREFERENCECOPY
`undef DOSHALLOWCOPY
`undef UVM_FUNCTION_ERROR
`undef UVM_GET_IMP
`undef UVM_GET_PEEK_IMP
`undef M_RESIZE_QUEUE_COPY
`undef M_RESIZE_QUEUE_NOCOPY
`undef M_RESIZE_QUEUE_OBJECT_COPY
`undef M_RESIZE_QUEUE_OBJECT_NOCOPY
`undef m_uvm_record_any_object
`undef m_uvm_record_array_int
`undef m_uvm_record_array_object
`undef m_uvm_record_array_string
`undef m_uvm_record_int
`undef m_uvm_record_object
`undef m_uvm_record_qda_enum
`undef m_uvm_record_string
`undef UVM_NONBLOCKING_GET_IMP
`undef UVM_NONBLOCKING_GET_IMP_SFX
`undef UVM_NONBLOCKING_GET_PEEK_IMP
`undef UVM_NONBLOCKING_PEEK_IMP
`undef UVM_NONBLOCKING_PEEK_IMP_SFX
`undef UVM_NONBLOCKING_PUT_IMP
`undef UVM_NONBLOCKING_PUT_IMP_SFX
`undef UVM_NONBLOCKING_TRANSPORT_IMP
`undef UVM_NONBLOCKING_TRANSPORT_IMP_SFX
`undef UVM_PEEK_IMP
`undef print_enum_field
`undef print_integral_field
`undef _protected
`undef UVM_PUT_IMP
`undef UVM_SEQ_ITEM_FUNCTION_ERROR
`undef UVM_SEQ_ITEM_GET_MASK
`undef UVM_SEQ_ITEM_GET_NEXT_ITEM_MASK
`undef UVM_SEQ_ITEM_HAS_DO_AVAILABLE_MASK
`undef UVM_SEQ_ITEM_ITEM_DONE_MASK
`undef UVM_SEQ_ITEM_PEEK_MASK
`undef UVM_SEQ_ITEM_PULL_IMP
`undef UVM_SEQ_ITEM_PULL_MASK
`undef UVM_SEQ_ITEM_PUSH_MASK
`undef UVM_SEQ_ITEM_PUT_MASK
`undef UVM_SEQ_ITEM_PUT_RESPONSE_MASK
`undef UVM_SEQ_ITEM_TASK_ERROR
`undef UVM_SEQ_ITEM_TRY_NEXT_ITEM_MASK
`undef UVM_SEQ_ITEM_UNI_PULL_MASK
`undef UVM_SEQ_ITEM_WAIT_FOR_SEQUENCES_MASK
`undef UVM_TASK_ERROR
`undef UVM_TRANSPORT_IMP
`undef _UVM_CB_MSG_NO_CBS
`undef _UVM_CB_MSG_NOT_REG
`undef _UVM_CB_MSG_NULL_CB
`undef _UVM_CB_MSG_NULL_OBJ

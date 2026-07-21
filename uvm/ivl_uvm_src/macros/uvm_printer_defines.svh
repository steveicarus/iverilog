//----------------------------------------------------------------------
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc. 
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

//------------------------------------------------------------------------------
//
// MACROS for uvm_printer usage
//
// Provides a set of printing macros that will call appropriate print methods
// inside of a uvm_printer object. All macros have two versions: one assumes
// a printer named printer is available in scope; the other takes a printer
// argument.
//
//------------------------------------------------------------------------------

`ifndef UVM_PRINTER_DEFINES_SVH
`define UVM_PRINTER_DEFINES_SVH

// uvm_print_int*
// --------------

`define uvm_print_int(F, R) \
  `uvm_print_int3(F, R, uvm_default_printer)

`define uvm_print_int3(F, R, P) \
   do begin \
     uvm_printer p__; \
     if(P!=null) p__ = P; \
     else p__ = uvm_default_printer; \
     `uvm_print_int4(F, R, `"F`", p__) \
   end while(0);

`define uvm_print_int4(F, R, NM, P) \
    P.print_int(NM, F, $bits(F), R, "[");


// uvm_print_enum
// --------------

`define uvm_print_enum(T, F, NM, P) \
    P.print_generic(NM, `"T`", $bits(F), F.name(), "[");


// uvm_print_object*
// -----------------

`define uvm_print_object(F) \
  `uvm_print_object2(F, uvm_default_printer)

`define uvm_print_object2(F, P) \
   do begin \
     uvm_printer p__; \
     if(P!=null) p__ = P; \
     else p__ = uvm_default_printer; \
     p__.print_object(`"F`", F, "["); \
   end while(0);


// uvm_print_string*
// -----------------

`define uvm_print_string(F) \
  `uvm_print_string2(F, uvm_default_printer)

`define uvm_print_string2(F, P) \
   do begin \
     uvm_printer p__; \
     if(P!=null) p__ = P; \
     else p__ = uvm_default_printer; \
     p__.print_string(`"F`", F, "["); \
   end while(0);


// uvm_print_array*
// ----------------

`define uvm_print_array_int(F, R) \
  `uvm_print_array_int3(F, R, uvm_default_printer)
   
`define uvm_print_array_int3(F, R, P) \
  `uvm_print_qda_int4(F, R, P, da)


// uvm_print_sarray*
// -----------------

`define uvm_print_sarray_int3(F, R, P) \
  `uvm_print_qda_int4(F, R, P, sa)

`define uvm_print_qda_int4(F, R, P, T) \
  begin \
    uvm_printer p__; \
    uvm_printer_knobs k__; \
    int curr, max__; max__=0; curr=0; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
    foreach(F[i]) max__ = i+1; \
//    max__=$size(F); \
    p__.print_array_header (`"F`", max__,`"T``(integral)`"); \
    k__ = p__.knobs; \
    if((p__.knobs.depth == -1) || (p__.m_scope.depth() < p__.knobs.depth+1)) \
    begin \
      foreach(F[i__]) begin \
        if(k__.begin_elements == -1 || k__.end_elements == -1 || curr < k__.begin_elements ) begin \
          `uvm_print_int4(F[curr], R, p__.index_string(curr), p__) \
        end \
        else break; \
        curr++; \
      end \
      if(curr<max__) begin \
        if((max__-k__.end_elements) > curr) curr = max__-k__.end_elements; \
        if(curr<k__.begin_elements) curr = k__.begin_elements; \
        else begin \
          p__.print_array_range(k__.begin_elements, curr-1); \
        end \
        for(curr=curr; curr<max__; ++curr) begin \
          `uvm_print_int4(F[curr], R, p__.index_string(curr), p__) \
        end \
      end \
    end \
    p__.print_array_footer(max__); \
    //p__.print_footer(); \
  end
 
`define uvm_print_qda_enum(F, P, T, ET) \
  begin \
    uvm_printer p__; \
    uvm_printer_knobs k__; \
    int curr, max__; max__=0; curr=0; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
    foreach(F[i]) max__ = i+1; \
    //max__=$size(F); \
    p__.print_array_header (`"F`", max__,`"T``(``ET``)`"); \
    k__ = p__.knobs; \
    if((p__.knobs.depth == -1) || (p__.m_scope.depth() < p__.knobs.depth+1)) \
    begin \
      foreach(F[i__]) begin \
        if(k__.begin_elements == -1 || k__.end_elements == -1 || curr < k__.begin_elements ) begin \
          `uvm_print_enum(ET, F[curr], p__.index_string(curr), p__) \
        end \
        else break; \
        curr++; \
      end \
      if(curr<max__) begin \
        if((max__-k__.end_elements) > curr) curr = max__-k__.end_elements; \
        if(curr<k__.begin_elements) curr = k__.begin_elements; \
        else begin \
          p__.print_array_range(k__.begin_elements, curr-1); \
        end \
        for(curr=curr; curr<max__; ++curr) begin \
          `uvm_print_enum(ET, F[curr], p__.index_string(curr), p__) \
        end \
      end \
    end \
    p__.print_array_footer(max__); \
    //p__.print_footer(); \
  end
 
`define uvm_print_queue_int(F, R) \
  `uvm_print_queue_int3(F, R, uvm_default_printer)

`define uvm_print_queue_int3(F, R, P) \
  `uvm_print_qda_int3(F, R, P, queue)

`define uvm_print_array_object(F,FLAG) \
  `uvm_print_array_object3(F, uvm_default_printer,FLAG)
   
`define uvm_print_sarray_object(F,FLAG) \
  `uvm_print_sarray_object3(F, uvm_default_printer,FLAG)
   
`define uvm_print_array_object3(F, P,FLAG) \
  `uvm_print_object_qda4(F, P, da,FLAG)

`define uvm_print_sarray_object3(F, P,FLAG) \
  `uvm_print_object_qda4(F, P, sa,FLAG)

`define uvm_print_object_qda4(F, P, T,FLAG) \
  do begin \
    int curr, max__; \
    uvm_printer p__; \
    max__=0; curr=0; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
    //max__=$size(F); \
    foreach(F[i]) max__ = i+1; \
\
    //p__.print_header();\
\
    p__.m_scope.set_arg(`"F`");\
    p__.print_array_header(`"F`", max__, `"T``(object)`");\
    if((p__.knobs.depth == -1) || (p__.knobs.depth+1 > p__.m_scope.depth())) \
    begin\
      for(curr=0; curr<max__ && (p__.knobs.begin_elements == -1 || \
         p__.knobs.end_elements == -1 || curr<p__.knobs.begin_elements); ++curr) begin \
        if(((FLAG)&UVM_REFERENCE) == 0) \
          p__.print_object(p__.index_string(curr), F[curr], "[");\
        else \
          p__.print_object_header(p__.index_string(curr), F[curr], "[");\
      end \
      if(curr<max__) begin\
        curr = max__-p__.knobs.end_elements;\
        if(curr<p__.knobs.begin_elements) curr = p__.knobs.begin_elements;\
        else begin\
          p__.print_array_range(p__.knobs.begin_elements, curr-1);\
        end\
        for(curr=curr; curr<max__; ++curr) begin\
          if(((FLAG)&UVM_REFERENCE) == 0) \
            p__.print_object(p__.index_string(curr), F[curr], "[");\
          else \
            p__.print_object_header(p__.index_string(curr), F[curr], "[");\
        end \
      end\
    end \
\
    p__.print_array_footer(max__); \
    //p__.print_footer(); \
  end while(0);
 
`define uvm_print_object_queue(F,FLAG) \
  `uvm_print_object_queue3(F, uvm_default_printer,FLAG)
   
`define uvm_print_object_queue3(F, P,FLAG) \
  do begin \
    `uvm_print_object_qda4(F,P, queue,FLAG); \
  end while(0);
 
`define uvm_print_array_string(F) \
  `uvm_print_array_string2(F, uvm_default_printer)
   
`define uvm_print_array_string2(F, P) \
   `uvm_print_string_qda3(F, P, da)

`define uvm_print_sarray_string2(F, P) \
   `uvm_print_string_qda3(F, P, sa)

`define uvm_print_string_qda3(F, P, T) \
  do begin \
    int curr, max__; \
    uvm_printer p__; \
    max__=0; curr=0; \
    //max__=$size(F); \
    foreach(F[i]) max__ = i+1; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
\
    //p__.print_header();\
\
    p__.m_scope.set_arg(`"F`");\
    p__.print_array_header(`"F`", max__, `"T``(string)`");\
    if((p__.knobs.depth == -1) || (p__.knobs.depth+1 > p__.m_scope.depth())) \
    begin\
      for(curr=0; curr<max__ && curr<p__.knobs.begin_elements; ++curr) begin\
        p__.print_string(p__.index_string(curr), F[curr], "[");\
      end \
      if(curr<max__) begin\
        curr = max__-p__.knobs.end_elements;\
        if(curr<p__.knobs.begin_elements) curr = p__.knobs.begin_elements;\
        else begin\
          p__.print_array_range(p__.knobs.begin_elements, curr-1);\
        end\
        for(curr=curr; curr<max__; ++curr) begin\
          p__.print_string(p__.index_string(curr), F[curr], "[");\
        end \
      end\
    end \
\
    p__.print_array_footer(max__); \
    //p__.print_footer(); \
  end while(0);
 
`define uvm_print_string_queue(F) \
  `uvm_print_string_queue2(F, uvm_default_printer)
   
`define uvm_print_string_queue2(F, P) \
  do begin \
    `uvm_print_string_qda3(F,P, queue); \
  end while(0);

//-----------------------------------------------------------------------------
//
// Associative array printing methods
//
//-----------------------------------------------------------------------------
`define uvm_print_aa_string_int(F) \
  `uvm_print_aa_string_int3(F, R, uvm_default_printer)


`define uvm_print_aa_string_int3(F, R, P) \
  begin \
    uvm_printer p__; \
    uvm_printer_knobs k__; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
    p__.print_array_header (`"F`", F.num(), "aa(int,string)"); \
    k__ = p__.knobs; \
    if((p__.knobs.depth == -1) || (p__.m_scope.depth() < p__.knobs.depth+1)) \
    begin \
      foreach(F[string_aa_key]) \
          `uvm_print_int4(F[string_aa_key], R,  \
                                {"[", string_aa_key, "]"}, p__) \
    end \
    p__.print_array_footer(F.num()); \
    //p__.print_footer(); \
  end

`define uvm_print_aa_string_object(F,FLAG) \
  `uvm_print_aa_string_object_3(F, uvm_default_printer,FLAG)
  
`define uvm_print_aa_string_object3(F, P,FLAG) \
  begin \
    uvm_printer p__; \
    uvm_printer_knobs k__; \
    uvm_object u__; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
    p__.print_array_header (`"F`", F.num(), "aa(object,string)"); \
    k__ = p__.knobs; \
    if((p__.knobs.depth == -1) || (p__.m_scope.depth() < p__.knobs.depth+1)) \
    begin \
      foreach(F[string_aa_key]) begin \
          if(((FLAG)&UVM_REFERENCE)==0) \
            p__.print_object({"[", string_aa_key, "]"}, F[string_aa_key], "[");\
          else \
            p__.print_object_header({"[", string_aa_key, "]"}, F[string_aa_key], "[");\
      end \
    end \
    p__.print_array_footer(F.num()); \
    //p__.print_footer(); \
  end

`define uvm_print_aa_string_string(F) \
  `uvm_print_aa_string_string_2(F, uvm_default_printer)
  
`define uvm_print_aa_string_string2(F, P) \
  begin \
    uvm_printer p__; \
    uvm_printer_knobs k__; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
    p__.print_array_header (`"F`", F.num(), "aa(string,string)"); \
    k__ = p__.knobs; \
    if((p__.knobs.depth == -1) || (p__.m_scope.depth() < p__.knobs.depth+1)) \
    begin \
      foreach(F[string_aa_key]) \
          p__.print_string({"[", string_aa_key, "]"}, F[string_aa_key], "["); \
    end \
    p__.print_array_footer(F.num()); \
    //p__.print_footer(); \
  end

`define uvm_print_aa_int_object(F,FLAG) \
  `uvm_print_aa_int_object_3(F, uvm_default_printer,FLAG)

`define uvm_print_aa_int_object3(F, P,FLAG) \
  begin \
    uvm_printer p__; \
    uvm_printer_knobs k__; \
    uvm_object u__; \
    int key; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
    p__.print_array_header (`"F`", F.num(), "aa(object,int)"); \
    k__ = p__.knobs; \
    if((p__.knobs.depth == -1) || (p__.m_scope.depth() < p__.knobs.depth+1)) \
    begin \
      foreach(F[key]) begin \
          $swrite(__m_uvm_status_container.stringv, "[%0d]", key); \
          if(((FLAG)&UVM_REFERENCE)==0) \
            p__.print_object(__m_uvm_status_container.stringv, F[key], "[");\
          else \
            p__.print_object_header(__m_uvm_status_container.stringv, F[key], "[");\
      end \
    end \
    p__.print_array_footer(F.num()); \
    //p__.print_footer(); \
  end

`define uvm_print_aa_int_key4(KEY, F, R, P) \
  begin \
    uvm_printer p__; \
    uvm_printer_knobs k__; \
    if(P!=null) p__ = P; \
    else p__ = uvm_default_printer; \
    __m_uvm_status_container.stringv = "aa(int,int)"; \
    for(int i=0; i<__m_uvm_status_container.stringv.len(); ++i) \
      if(__m_uvm_status_container.stringv[i] == " ") \
        __m_uvm_status_container.stringv[i] = "_"; \
    p__.print_array_header (`"F`", F.num(), __m_uvm_status_container.stringv); \
    k__ = p__.knobs; \
    if((p__.knobs.depth == -1) || (p__.m_scope.depth() < p__.knobs.depth+1)) \
    begin \
      foreach(F[aa_key]) begin \
          `uvm_print_int4(F[aa_key], R,  \
                                {"[", $sformatf("%0d",aa_key), "]"}, p__) \
      end \
    end \
    p__.print_array_footer(F.num()); \
    //p__.print_footer(); \
  end

`endif

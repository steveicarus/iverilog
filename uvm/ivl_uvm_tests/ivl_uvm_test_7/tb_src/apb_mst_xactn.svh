class apb_mst_xactn; // extends uvm_sequence_item;
  
   rand bit   [31:0] addr;
   rand logic [31:0] data;
   rand kind_e kind;  
 
   /*
   `uvm_object_utils_begin(apb_rw)
     `uvm_field_int(addr, UVM_ALL_ON | UVM_NOPACK);
     `uvm_field_int(data, UVM_ALL_ON | UVM_NOPACK);
     `uvm_field_enum(kind_e,kind, UVM_ALL_ON | UVM_NOPACK);
   `uvm_object_utils_end
  */

   function new (string name = "apb_mst_xactn");
      // super.new(name);
   endfunction : new

   function string convert2string();
     return $sformatf("kind: %0d addr: 0x%0h data: 0x%0h",kind,addr,data);
   endfunction : convert2string 
endclass : apb_mst_xactn



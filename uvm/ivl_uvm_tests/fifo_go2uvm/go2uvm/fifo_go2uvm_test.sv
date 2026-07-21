// Copyright (c) 2004-2017 VerifWorks, Bangalore, India
// http://www.verifworks.com 
// Contact: support@verifworks.com 
// 
// This program is part of Go2UVM at www.go2uvm.org
// Some portions of Go2UVM are free software.
// You can redistribute it and/or modify  
// it under the terms of the GNU Lesser General Public License as   
// published by the Free Software Foundation, version 3.
//
// VerifWorks reserves the right to obfuscate part or full of the code
// at any point in time. 
// We also support a comemrical licensing option for an enhanced version
// of Go2UVM, please contact us via support@verifworks.com
//
// This program is distributed in the hope that it will be useful, but 
// WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
// Lesser General Lesser Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

// Generating Go2UVM Test for module: fifo
// ---------------------------------------------------------

// Automatically generated from VerifWorks's DVCreate-Go2UVM product
// Thanks for using VerifWorks products, see http://www.verifworks.com for more

import uvm_pkg::*;
`include "vw_go2uvm_macros.svh"
// Import Go2UVM Package
import vw_go2uvm_pkg::*;

// Use the base class provided by the vw_go2uvm_pkg
`G2U_TEST_BEGIN(fifo_test)

  // Connect to a design interface
  `G2U_GET_VIF(fifo_if)

  task reset;
    `g2u_display ("Start of reset", UVM_MEDIUM)
     this.vif.cb.rst_n <= 1'b0;
     repeat (5) @ (this.vif.cb);
     this.vif.cb.rst_n <= 1'b1;
     repeat (1) @ (this.vif.cb);
    `g2u_display ("End of reset", UVM_MEDIUM)
  endtask : reset

  task main ();
    byte unsigned push_data, pop_data;
    `g2u_display ("Start of main", UVM_MEDIUM)
     push_data = $urandom();
     this.vif.cb.push <= 1'b1;
     this.vif.cb.data_in <= push_data;
     repeat (1) @ (this.vif.cb);
     this.vif.cb.push <= 1'b0;
     repeat (5) @ (this.vif.cb);
     this.vif.cb.pop <= 1'b1;
     repeat (1) @ (this.vif.cb);
     this.vif.cb.pop <= 1'b0;
     repeat (1) @ (this.vif.cb);
     pop_data = vif.cb.data_out;
     a_data_integ : assert (push_data == pop_data) else
       `g2u_error ($sformatf ("Data mismtach, push_data: 0x%0x pop_data: 0x%0x",
           push_data, pop_data));
     repeat (1) @ (this.vif.cb);
    `g2u_display ("End of main", UVM_MEDIUM)
  endtask : main

`G2U_TEST_END


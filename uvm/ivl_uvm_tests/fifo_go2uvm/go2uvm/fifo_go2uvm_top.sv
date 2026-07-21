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


// Generating Go2UVM top module for DUT: fifo
// ---------------------------------------------------------
`include "fifo_if.sv"
`include "fifo_go2uvm_test.sv"
module go2uvm_fifo;
  parameter VW_CLK_PERIOD = 10;

  // Simple clock generator
  bit `VW_CLK ;
  always # (VW_CLK_PERIOD/2) `VW_CLK <= ~`VW_CLK;

  // Interface instance
  fifo_if fifo_if_0 (.*);

  // Connect TB clk to Interface instance clk

  // DUT instance
  fifo fifo_0 (
    .clk(fifo_if_0.clk),
    .data_in(fifo_if_0.data_in),
    .pop(fifo_if_0.pop),
    .push(fifo_if_0.push),
    .rst_n(fifo_if_0.rst_n),
    .data_out(fifo_if_0.data_out),
    .empty(fifo_if_0.empty),
    .full(fifo_if_0.full),
    .pop_err_on_empty(fifo_if_0.pop_err_on_empty),
    .push_err_on_full(fifo_if_0.push_err_on_full)
  );


  initial begin : go2uvm_test
    // Connect virtual interface to physical interface
    `G2U_SET_VIF (fifo_if, fifo_if_0)
    // Kick start standard UVM phasing
    run_test ();
  end : go2uvm_test
endmodule : go2uvm_fifo


 
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


// Generating SystemVerilog interface for module: fifo
// ---------------------------------------------------------
// Using VW_CLK as a text macro for clock 
// If your clock signal is named other than clk, change the macro below
`define VW_CLK clk
interface fifo_if (input logic `VW_CLK);
  logic  [7:0] data_in;
  logic   pop;
  logic   push;
  logic   rst_n;
  logic  [7:0] data_out;
  logic   empty;
  logic   full;
  logic   pop_err_on_empty;
  logic   push_err_on_full;
  // End of interface signals 


  `ifndef IVL_UVM
  // Start of clocking block definition 
  clocking cb @(posedge `VW_CLK);
    output data_in;
    output pop;
    output push;
    output rst_n;
    input data_out;
    input empty;
    input full;
    input pop_err_on_empty;
    input push_err_on_full;
  endclocking : cb
  // End of clocking block definition 
  `endif // IVL_UVM

endinterface : fifo_if

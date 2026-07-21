/**********************************************************************
 *  IVL_UVM example - SystemVerilog Interface generator 
 *
 *
 * Verilog DUT to test the $ivl_uvm_intf_gen 
 *
*********************************************************************/

`timescale 1ns / 1ns
module register_slice ( s_data_in, s_valid_in, s_ready_out, m_data_out, m_valid_out, m_ready_in);

		parameter WIDTH = 64;

		input [WIDTH-1:0] s_data_in;
		input s_valid_in;
		output s_ready_out;
		output [WIDTH-1:0] m_data_out;
		output m_valid_out;
		input m_ready_in;


		`ifdef IVL_UVM_INTF_GEN
			initial #1 $ivl_uvm_intf_gen();
		`endif // IVL_UVM_INTF_GEN

endmodule

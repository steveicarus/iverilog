
`include "apb_sl_dut.sv"
`include "apb_mst_inc.svh"


module apb_master_top();
  import ivl_uvm_pkg::*;
  import apb_pkg::*;


  parameter CLK_PERIOD = 10;

  bit pclk;
  apb_mst_xactn tr;

  //Instantiation of Interface
  apb_if if_0(); // .pclk(pclk));
  apb_mst_driver u_bfm ();


  //Instantiation of DUT
  apb_sl_dut u_apb_sl_dut (
			 .pclk ( u_bfm.sigs.pclk ),
	                 .presetn ( u_bfm.sigs.presetn ),
			 .psel ( u_bfm.sigs.psel ),
			 .penable ( u_bfm.sigs.penable ),
	                 .pwrite ( u_bfm.sigs.pwrite ),
			 .paddr ( u_bfm.sigs.paddr ),
			 .pwdata ( u_bfm.sigs.pwdata ),
			 .prdata ( u_bfm.sigs.prdata ),
                         .pready ( u_bfm.sigs.pready ),
                         .pslverr ( u_bfm.sigs.pslverr )
			 );

  initial begin:clk_gen
    pclk = 1'b1;
    forever #(CLK_PERIOD/2) pclk = ~ pclk;
  end : clk_gen

  assign u_bfm.sigs.pclk = pclk;

  initial begin:test
    run_test();
  end:test

  initial begin
    u_bfm.sigs.presetn = 0;
    #100;
    u_bfm.sigs.presetn = 1;
    #100;
    do_wr_rd_chk ();
  end

  task do_wr_rd_chk();
    tr = new ("tr");
    tr.kind = APB_WRITE;
    tr.addr = $urandom();
    tr.data = $urandom();
    u_bfm.seq_item_port.put(tr);
    #100;
    tr.kind = APB_READ;
    u_bfm.seq_item_port.put(tr);
    #100;
  endtask : do_wr_rd_chk
    

endmodule:apb_master_top

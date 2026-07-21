
module apb_mst_driver ();
  import ivl_uvm_pkg::*;
  import apb_pkg::*;
  
   event trig;
   apb_if sigs ();
   apb_mst_xactn tr;
   uvm_phase u_ph_0;

   ivl_uvm_tlm_pull_port seq_item_port;
   initial begin
     seq_item_port = new ("seq_item_port", null);
     u_ph_0 = new ();
     run_phase (u_ph_0);
   end

   task run_phase(uvm_phase phase);

      sigs.psel    <= '0;
      sigs.penable <= '0;

      forever begin
         @ (posedge sigs.pclk);

	 `g2u_display ("Inside Driver run_phase")
         seq_item_port.get_next_item(tr);
	 `g2u_display ("Got a new REQ")

         @ (posedge sigs.pclk);
         
	 case (tr.kind)
           APB_READ:  read(tr.addr, tr.data);  
           APB_WRITE: write(tr.addr, tr.data);
         endcase
         
         seq_item_port.item_done();
	 ->trig ;
      end
   endtask: run_phase

   task read(input  bit   [7:0] addr,
                               output logic [7:0] data);

      `g2u_printf ( ("APB: Start of READ: addr: 0x%0h ", addr))
      sigs.paddr   <= addr;
      sigs.pwrite  <= '0;
      sigs.psel    <= '1;
      @ (posedge sigs.pclk);
      sigs.penable <= '1;
      @ (posedge sigs.pclk);
      @ (posedge sigs.pclk);
      @ (posedge sigs.pclk);
      data = sigs.prdata;
      sigs.psel    <= '0;
      sigs.penable <= '0;
      `g2u_printf ( ("APB: End of READ: addr: 0x%0h data: 0x%0h", addr, data))
   endtask: read

   task write(input bit [7:0] addr,
                                input bit [7:0] data);

      `g2u_printf ( ("APB: Start of WRITE: addr: 0x%0h data: 0x%0h", addr, data))
      sigs.paddr   <= addr;
      sigs.pwdata  <= data;
      sigs.pwrite  <= '1;
      sigs.psel    <= '1;
      @ (posedge sigs.pclk);
      sigs.penable <= '1;
      @ (posedge sigs.pclk);
      sigs.psel    <= '0;
      sigs.penable <= '0;
      `g2u_printf ( ("APB: End of WRITE: addr: 0x%0h data: 0x%0h", addr, data))
   endtask: write

endmodule : apb_mst_driver



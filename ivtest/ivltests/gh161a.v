

module test();
   typedef struct packed {
      logic [31:0] sub_local;
   } row_entry_t;  

   typedef struct  packed {
      logic [31:0] row_local;
      row_entry_t         sub;
      row_entry_t [1:0]   sub_list;
   } row_t; 
    
   row_t main;

   initial begin
      main.row_local = 32'hCAFE;	
      main.sub.sub_local = 32'h00000001;
      main.sub_list[0].sub_local = 32'hACE;
      main.sub_list[1].sub_local = 32'hECA;
      $display("main=0x%08X", main);
      if (main !== 128'h0000cafe0000000100000eca00000ace) begin
	 $display("FAILED -- main != 128'h0000cafe0000000100000eca00000ace");
	 $finish;
      end
      $display("main.row_local=0x%08X", main.row_local);    
      $display("main.sub=0x%08X",       main.sub);    
      //$display("0x%08X", main.sub.sub_local);    
      //$display("0x%08X", main.sub_list[0].sub_local);    
      $display("PASSED");
      $finish();
   end

endmodule

// ivl-bugs PR#307

module top;
      reg [127:0] in1;
      reg [127:0] in2;
      wire [128:0] out1;
      reg [128:0] out2;

      assign out1 = in1 + in2;


   task r;
      integer   errors;
      begin
      out2 = in1 + in2;
      $display("\n   %h\n+  %h", in1,in2);
      $display("= %h", out1);
      $display("= %h", out2);
      if (out1 != out2)
	begin
           $display("MISMATCH");
	   errors = errors + 1;
	end
      end
   endtask

   initial begin
      r.errors = 0;

      in1 = 128'hffffffffffffffffffffffffffffffff;
      in2 = 128'hfffffffffffffffffffffffffffffff7;
      r;

      in1 = 128'hffffffffffffffffffffffffffffffff;
      in2 = 128'h00000000000000000000000000000001;
      r;

      in1 = 128'h00000000000000000000000000000001;
      in2 = 128'hffffffffffffffffffffffffffffffff;
      r;

      in1 = 128'h00000000000000000000000000000000;
      in2 = 128'hffffffffffffffffffffffffffffffff;
      r;

      in1 = 128'hffffffffffffffffffffffffffffffff;
      in2 = 128'hffffffffffffffffffffffffffffffff;
      r;

      in1 = 128'h00000000000000000000000000000000;
      in2 = 128'h00000000000000000000000000000000;
      r;

      in1 = 128'h80000000000000000000000000000000;
      in2 = 128'h80000000000000000000000000000000;
      r;

      in1 = 128'h08000000000000000000000000000000;
      in2 = 128'h08000000000000000000000000000000;
      r;

      in1 = 128'h00000000000000008000000000000000;
      in2 = 128'h00000000000000008000000000000000;
      r;

      in1 = 128'h55555555555555555555555555555555;
      in2 = 128'h55555555555555555555555555555555;
      r;

      in1 = 128'haaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa;
      in2 = 128'haaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa;
      r;

      if (r.errors)
	$display("FAILED: %d errors", r.errors);
      else
	$display("PASSED");
   end
endmodule

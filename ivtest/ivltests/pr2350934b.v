module test ();
    parameter param = 3;
    reg [2:0] dummy;

    initial dummy = block.f(0);

    generate
       case (param)
	 1, 2: if (param==1) begin : block
            function [2:0] f;
               input i;
               begin
                  $display ("if param==1");
                  f = param;
               end
            endfunction
         end else begin : block
            function [2:0] f;
               input i;
               begin
                  $display ("else if param==2");
                  f = param;
               end
            endfunction
         end

	 4: begin : block
            function [2:0] f;
               input i;
               begin
                  $display ("if param==4");
                  f = param;
               end
            endfunction // f
	 end
       endcase
    endgenerate
endmodule

module top ();
   test #(1) a();
   test #(2) b();
   test #(4) c();

   initial begin
      #1 if (a.dummy !== 1) begin
	 $display("FAILED -- a.dummy = %d", a.dummy);
	 $finish;
      end
      if (b.dummy !== 2) begin
	 $display("FAILED -- b.dummy = %d", b.dummy);
	 $finish;
      end
      if (c.dummy !== 4) begin
	 $display("FAILED -- c.dummy = %d", c.dummy);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule

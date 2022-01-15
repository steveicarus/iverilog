module test ();
    parameter param = 3;
    reg [2:0] dummy;

    initial dummy = block.f(0);

    generate
        if (param==1) begin : block
            function [2:0] f;
                input i;
                begin
                    $display ("if param==1");
                    f = param;
                end
            endfunction
        end else if (param==2) begin : block
            function [2:0] f;
                input i;
                begin
                    $display ("else if param==2");
                    f = param;
                end
            endfunction
        end
    endgenerate
endmodule

module top ();
    test #(1) a();
    test #(2) b();

   initial begin
      #1 if (a.dummy !== 1) begin
	 $display("FAILED -- a.dummy = %d", a.dummy);
	 $finish;
      end
      if (b.dummy !== 2) begin
	 $display("FAILED -- b.dummy = %d", b.dummy);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule

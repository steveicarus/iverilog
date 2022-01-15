module main;

   reg a, b;

   triand net;

   assign net = a;
   assign net = b;

   initial begin
      a = 'b0;
      b = 'b0;
      #1 if (net !== 1'b0) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'b0;
      b = 'b1;
      #1 if (net !== 1'b0) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'b0;
      b = 'bx;
      #1 if (net !== 1'b0) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'b0;
      b = 'bz;
      #1 if (net !== 1'b0) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'b1;
      b = 'b1;
      #1 if (net !== 1'b1) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'b1;
      b = 'bx;
      #1 if (net !== 1'bx) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'b1;
      b = 'bz;
      #1 if (net !== 1'b1) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'bx;
      b = 'bx;
      #1 if (net !== 1'bx) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'bx;
      b = 'bz;
      #1 if (net !== 1'bx) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      a = 'bz;
      b = 'bz;
      #1 if (net !== 1'bz) begin
	 $display("FAILED -- a=%b, b=%b, net=%b", a, b, net);
	 $finish;
      end

      $display("PASSED");
    end
endmodule // main

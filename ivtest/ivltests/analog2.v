nature Voltage;
  units      = "V";
  access     = V;
  idt_nature = Flux;
  abstol     = 1e-6;
endnature

discipline voltage;
  potential Voltage;
enddiscipline

nature Flux;
  units      = "Wb";
  access     = Phi;
  ddt_nature = Voltage;
  abstol     = 1e-9;
endnature

`timescale 1s/1s
module main;

   real value;
   voltage in, out;
   analog V(out) <+ transition(value, 0, 4);

   initial begin
      value = 0.0;
      #10 if (V(out) != value) begin
	 $display("FAILED -- value=%g, res=%g", value, V(out));
	 $finish;
      end

      // Halfway through the rise time, the output should have
      // half the input.
      value = 2.0;
      //#2 if (V(out) != value/2) begin
      #2 if (abs(V(out) - value/2) > 1e-6) begin
	 $display("FAILED -- value=%g, value/2=%g, res=%f", value, value/2, V(out));
	 $finish;
      end

      // After the full transition time, the output should match
      // the input.
      #2 if (V(out) != value) begin
	 $display("FAILED -- value=%g, res=%f", value, V(out));
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

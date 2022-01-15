nature Voltage;
  units      = "V";
  access     = V;
  idt_nature = Flux;
  abstol     = 1e-6;
endnature

nature Flux;
  units      = "Wb";
  access     = Phi;
  ddt_nature = Voltage;
  abstol     = 1e-9;
endnature

discipline voltage;
  potential Voltage;
enddiscipline


module main;

   real value;
   voltage out;
   analog V(out) <+ abs(value);

   initial begin
      value = 1.0;
      #1 if (V(out) != abs(value)) begin
	 $display("FAILED -- value=%g, res=%g", value, V(out));
	 $finish;
      end

      value = -1.0;
      #1 if (V(out) != abs(value)) begin
	 $display("FAILED -- value=%g, res=%f", value, V(out));
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

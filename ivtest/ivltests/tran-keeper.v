module main;


   // Model a pin with a weak keeper circuit. The way this works:
   // If the pin value is 1, then attach a weak1 pullup, but
   // if the pin value is 0, attach a weak0 pulldown.
   wire pin;
   pullup   (weak1) (keep1);
   pulldown (weak0) (keep0);
   tranif1 (pin, keep1, pin);
   tranif0 (pin, keep0, pin);

   // Logic to drive a value onto a pin.
   reg	value, enable;
   bufif1 (pin, value, enable);

   initial begin
      value = 0;
      enable = 1;
      #1 if (pin !== 0) begin
	 $display("FAILED -- value=%b, enable=%b, pin=%b", value, enable, pin);
	 $finish;
      end

      // pin should hold its value after the drive is removed.
      enable = 0;
      #1 if (pin !== 0) begin
	 $display("FAILED -- value=%b, enable=%b, pin=%b", value, enable, pin);
	 $finish;
      end

      value = 1;
      enable = 1;
      #1 if (pin !== 1) begin
	 $display("FAILED -- value=%b, enable=%b, pin=%b", value, enable, pin);
	 $finish;
      end

      // pin should hold its value after the drive is removed.
      enable = 0;
      #1 if (pin !== 1) begin
	 $display("FAILED -- value=%b, enable=%b, pin=%b", value, enable, pin);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

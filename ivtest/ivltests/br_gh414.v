
module tb;
   string txt_i, txt_r, txt_h;

   int	  val_i;
   int    val_h;
   real   val_r;


   initial begin
      txt_i = "123";
      txt_r = "1.25";
      txt_h = "dead";
      val_i = txt_i.atoi();
      val_r = txt_r.atoreal();
      val_h = txt_h.atohex();

      $display("txt_i=%s, val_i=%0d", txt_i, val_i);
      if (val_i !== 123) begin
	 $display("FAILED");
	 $finish;
      end

      $display("txt_r=%s, val_r=%0f", txt_r, val_r);
      if (val_r != 1.25) begin
	 $display("FAILED");
	 $finish;
      end

      $display("txt_h=%s, val_h=%0h", txt_h, val_h);
      if (val_h !== 'hdead) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endmodule

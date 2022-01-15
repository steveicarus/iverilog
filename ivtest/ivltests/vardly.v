module main ();

reg [31:0] tdelay;
reg [3:0] var1,var2;
reg error;

always @(var1)
  #tdelay var2 = var1;

initial
  begin
    error = 0;
    tdelay = 5;
    # 1;			// This removes race between tdelay5 and var2.
    var2 = 0;
    #1 ;
    var1 = 1;		// Now twiddle var1 to cause var2 change 5ns later.
    #1 ;
    if(var2 != 0)
      begin
        $display("FAILED at %t",$time);
        error = 1;
      end
    #1;
    if(var2 != 0)
      begin
        $display("FAILED at %t",$time);
        error = 1;
      end
    #1;
    if(var2 != 0)
      begin
        $display("FAILED at %t",$time);
        error = 1;
      end
    #1;
    if(var2 != 0)
      begin
        $display("FAILED at %t",$time);
        error = 1;
      end
    #1;
    if(var2 != 1)
      begin
        $display("FAILED at %t",$time);
        error = 1;
      end

    if(error == 0)
       $display("PASSED");
  end

endmodule

// Check that when timing checks are disabled (or in the case of Icarus Verilog not supported)
// that the delayed reference and data signals become copies of the original reference and data signals

module test;

  wire sig1, sig2, del_sig1, del_sig2, del_sig3, del_sig4, notifier, cond1, cond2;

  assign sig1 = 1'b0;
  assign sig2 = 1'b1;

  specify

    $setuphold(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier , cond1 , cond2 , del_sig1 , del_sig2 ) ;

    /*
    Internally the simulator does the following:
    assign del_sig1 = sig1;
    assign del_sig2 = sig2;
    */

    $recrem(posedge sig1, negedge sig2 , 0:0:0 , 0:0:0 , notifier, cond1 , cond2 , del_sig3 , del_sig4 );

    /*
    Internally the simulator does the following:
    assign del_sig3 = sig1;
    assign del_sig4 = sig2;
    */

  endspecify

  initial begin

      if (del_sig1 == 1'b0 && del_sig2 == 1'b1 && del_sig3 == 1'b0 && del_sig4 == 1'b1)
        $display("PASSED");
      else
        $display("FAILED");
  end

endmodule

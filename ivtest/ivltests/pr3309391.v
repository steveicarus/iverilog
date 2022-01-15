module top;
  specify
    specparam Tdelay = 1.0;
  endspecify

  initial if (Tdelay != 1.0) $display("FAILED:, got %f", Tdelay);
          else $display("PASSED");
endmodule

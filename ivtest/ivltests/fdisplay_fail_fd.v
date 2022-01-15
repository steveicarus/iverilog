module top;
  initial begin
    // This will fail at run time.
    $fdisplay(32'h8000_000f, "write to invalid FD");
  end
endmodule

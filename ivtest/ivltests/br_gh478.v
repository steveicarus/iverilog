module bug;
  enum logic[1:0] {
    RST[2],
    IDLE,
    ACTIVE
  } state;

  initial begin
       state = RST0;
    #1 state = IDLE; // A non-blocking works, but IDLE is still a net
    #1 state = ACTIVE;
    #1 $display("PASSED");
  end

  other other(state == IDLE); // This is treating IDLE as a net
endmodule

module other(input logic val);
  always @(val) $display("%0t %b", $time, val);
endmodule

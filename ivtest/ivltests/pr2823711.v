// The basic example is from 1364-2005
module top;
  reg pass;
  reg [3:0] a;
  reg [5:0] b;
  reg [15:0] c;

  initial begin
    pass = 1'b1;

    a = 4'hF;
    b = 6'hA;

    // Self-determined context so the width is the same as a (4 bits).
    c = { a**b };
    if (c !== 16'h0001) begin
      $display("FAILED self-determined power, expected 0001, got %h", c);
      pass = 1'b0;
    end

    // The width is determined by a and c so use 16 bits here.
    c = a**b;
    if (c !== 16'hac61) begin
      $display("FAILED context-determined power, expected ac61, got %h", c);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

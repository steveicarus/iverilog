module top;
  parameter a_res = 16'b000001xx0xxx0xxx;
  parameter o_res = 16'b01xx1111x1xxx1xx;
  parameter x_res = 16'b01xx10xxxxxxxxxx;
  reg pass;
  reg [15:0] y, z, a, o, x;
  reg [127:0] yl, zl, al, ol, xl;
  initial begin
    pass = 1'b1;

    y = 16'b01xz01xz01xz01xz;
    z = 16'b00001111xxxxzzzz;
    yl = {8{y}};
    zl = {8{z}};

    // Check the & results
    a = y & z;
    if (a !== a_res) begin
      $display("FAILED: & test, expected %b, got %b", a_res, a);
      pass = 1'b0;
    end

    al = yl & zl;
    if (al !== {8{a_res}}) begin
      $display("FAILED: & (large) test, expected %b, got %b", {8{a_res}}, al);
      pass = 1'b0;
    end

    // Check the | results
    o = y | z;
    if (o !== o_res) begin
      $display("FAILED: | test, expected %b, got %b", o_res, o);
      pass = 1'b0;
    end

    ol = yl | zl;
    if (ol !== {8{o_res}}) begin
      $display("FAILED: | (large) test, expected %b, got %b", {8{o_res}}, ol);
      pass = 1'b0;
    end

    // Check the ^ results
    x = y ^ z;
    if (x !== x_res) begin
      $display("FAILED: | test, expected %b, got %b", x_res, x);
      pass = 1'b0;
    end

    xl = yl ^ zl;
    if (xl !== {8{x_res}}) begin
      $display("FAILED: ^ (large) test, expected %b, got %b", {8{x_res}}, xl);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

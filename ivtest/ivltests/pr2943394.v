module top;
  reg pass;
  reg [3:0] val;
  reg [3:0] pv_val;
  real rval;

  initial begin
    pass = 1'b1;

    // A release of an unforced variable should not change the variable.
    val = 4'b0110;
    release val;
    if (val !== 4'b0110) begin
      $display("Failed release of unforced sig, expected 4'b0110, got %b",
               val);
      pass = 1'b0;
    end

    // Verify that a force/release leaves the variable set correctly.
    force val = 4'b1001;
    release val;
    if (val !== 4'b1001) begin
      $display("Failed release of forced sig, expected 4'b1001, got %b",
               val);
      pass = 1'b0;
    end

    // A release of a currently unforced variable should not change it.
    val = 4'b0110;
    release val;
    if (val !== 4'b0110) begin
      $display("Failed release of unforced sig(2), expected 4'b0110, got %b",
               val);
      pass = 1'b0;
    end

    // A release of an unforced variable should not change the variable.
    pv_val = 4'b1001;
    release pv_val[1];
    if (pv_val !== 4'b1001) begin
      $display("Failed pv release of unforced sig, expected 4'b1001, got %b",
               pv_val);
      pass = 1'b0;
    end

    // Verify that a force/release leaves the variable set correctly.
    force pv_val[1] = 1'b1;
    release pv_val[2:0];
    if (pv_val !== 4'b1011) begin
      $display("Failed pv release of forced sig, expected 4'b1011, got %b",
               pv_val);
      pass = 1'b0;
    end

    // A release of a currently unforced variable should not change it.
    pv_val = 4'b1001;
    release pv_val[1];
    if (pv_val !== 4'b1001) begin
      $display("Failed pv release of unforced sig(2), expected 4'b1001, got %b",
               pv_val);
      pass = 1'b0;
    end

    // A release of an unforced variable should not change the variable.
    rval = 1.0;
    release rval;
    if (rval != 1.0) begin
      $display("Failed release of unforced sig, expected 1.0, got %.1f",
               rval);
      pass = 1'b0;
    end

    // Verify that a force/release leaves the variable set correctly.
    force rval = 2.0;
    release rval;
    if (rval != 2.0) begin
      $display("Failed release of forced sig, expected 2.0, got %.1f",
               rval);
      pass = 1'b0;
    end

    // A release of a currently unforced variable should not change it.
    rval = 1.0;
    release rval;
    if (rval != 1.0) begin
      $display("Failed release of unforced sig(2), expected 1.0, got %.1f",
               rval);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

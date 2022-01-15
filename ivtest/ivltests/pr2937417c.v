module top;
  reg a, pass;
  wire x, y, ab;

  assign (weak1, weak0) x = (a === 1'b1) ? 1'b0 : 1'b1;
  assign y = a;
  // We need this since Icarus currently has a bug when forcing from
  // an expression (it only uses the value when the force ran).
  assign ab = ~a;

  tran (x, y);

  initial begin
    // $monitor ($realtime,, x,y,,a);
    pass = 1'b1;
    // Check matching values.
    #1 if ( x !== 1'bx || y !== 1'bx) begin
      $display("Failed initial value, expected 1'bx, 1'bx, got %b %b", x, y);
      pass = 1'b0;
    end
    #1 a = 1'b0;
    #1 if ( x !== 1'b0 || y !== 1'b0) begin
      $display("Failed same value, expected 1'b0, 1'b0, got %b %b", x, y);
      pass = 1'b0;
    end
    #1 a = 1'b1;
    #1 if ( x !== 1'b1 || y !== 1'b1) begin
      $display("Failed same value, expected 1'b1, 1'b1, got %b %b", x, y);
      pass = 1'b0;
    end
    // Check force with opposite values.
    #1 force x = ab;
    #1 if ( x !== 1'b0 || y !== 1'bx) begin
      $display("Failed force value, expected 1'b0, 1'bx, got %b %b", x, y);
      pass = 1'b0;
    end
    #1 a = 1'b0;
    #1 if ( x !== 1'b1 || y !== 1'bx) begin
      $display("Failed force value, expected 1'b1, 1'bx, got %b %b", x, y);
      pass = 1'b0;
    end
    // Now release.
    #1 release x;
    #1 if ( x !== 1'b0 || y !== 1'b0) begin
      $display("Failed release value, expected 1'b0, 1'b0, got %b %b", x, y);
      pass = 1'b0;
    end
    #1 a = 1'b1;
    #1 if ( x !== 1'b1 || y !== 1'b1) begin
      $display("Failed release value, expected 1'b1, 1'b1, got %b %b", x, y);
      pass = 1'b0;
    end
    // Now force and release the driving signal.
    #1 force a = 1'bx;
    #1 if ( x !== 1'bx || y !== 1'bx) begin
      $display("Failed driver value, expected 1'bx, 1'bx, got %b %b", x, y);
      pass = 1'b0;
    end
    #1 release a;
       a = 1'b0;
    #1 if ( x !== 1'b0 || y !== 1'b0) begin
      $display("Failed driver value, expected 1'b0, 1'b0, got %b %b", x, y);
      pass = 1'b0;
    end
    // Check that the other driver works.
    #1 a = 1'bz;
    #1 if ( x !== 1'b1 || y !== 1'b1) begin
      $display("Failed alt. value, expected 1'b1, 1'b1, got %b %b", x, y);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

module top;
  reg pass;
  reg [7:0] vec;
  integer off;
  time delay;
  event trig;

  initial begin
    pass = 1'b1;
    delay = 1;

    // Assign before the vector (constant delay).
    vec = 8'hff;
    off = -1;
    vec[off] <= #1 1'b0;
    #2 if (vec !== 8'hff) begin
      $display("Failed the before vector (C) test, expected 8'hff, got %h",
               vec);
      pass = 1'b0;
    end

    // Assign after the vector (constant delay).
    vec = 8'hff;
    off = 8;
    vec[off] <= #1 1'b0;
    #2 if (vec !== 8'hff) begin
      $display("Failed the after vector (C) test, expected 8'hff, got %h",
               vec);
      pass = 1'b0;
    end

    // Assign before the vector (variable delay).
    vec = 8'hff;
    off = -1;
    vec[off] <= #(delay) 1'b0;
    #2 if (vec !== 8'hff) begin
      $display("Failed the before vector (V) test, expected 8'hff, got %h",
               vec);
      pass = 1'b0;
    end

    // Assign after the vector (variable delay).
    vec = 8'hff;
    off = 8;
    vec[off] <= #(delay) 1'b0;
    #2 if (vec !== 8'hff) begin
      $display("Failed the after vector (V) test, expected 8'hff, got %h",
               vec);
      pass = 1'b0;
    end

    // Assign before the vector (event trigger).
    vec = 8'hff;
    off = -1;
    vec[off] <= @(trig) 1'b0;
    ->trig;
    #1 if (vec !== 8'hff) begin
      $display("Failed the before vector (E) test, expected 8'hff, got %h",
               vec);
      pass = 1'b0;
    end

    // Assign after the vector (event trigger).
    vec = 8'hff;
    off = 8;
    vec[off] <= @(trig) 1'b0;
    ->trig;
    #1 if (vec !== 8'hff) begin
      $display("Failed the after vector (V) test, expected 8'hff, got %h",
               vec);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

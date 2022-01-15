module top;
  reg pass;
  enum bit signed [7:0] {a = 1, b = 2, c = 3, d = 4} enum_var;

  initial begin
    pass = 1'b1;

// Add another test that a negative value is not valid.
// Also an out of range value stays out of range.
    enum_var = a;
    if (enum_var !== enum_var.first) begin
      $display("FAILED: initialization, expected %d, got %d", a, enum_var);
      pass = 1'b0;
    end

    enum_var = enum_var.next;
    enum_var = enum_var.prev;
    enum_var = enum_var.next();
    if (enum_var !== b) begin
      $display("FAILED: next(), expected %d, got %d", b, enum_var);
      pass = 1'b0;
    end

    enum_var = enum_var.next(0);
    if (enum_var !== b) begin
      $display("FAILED: next(0), expected %d, got %d", b, enum_var);
      pass = 1'b0;
    end

    enum_var = enum_var.next(1);
    if (enum_var !== c) begin
      $display("FAILED: next(1), expected %d, got %d", c, enum_var);
      pass = 1'b0;
    end

    enum_var = enum_var.next(2);
    if (enum_var !== a) begin
      $display("FAILED: next(2), expected %d, got %d", a, enum_var);
      pass = 1'b0;
    end

    enum_var = enum_var.prev();
    if (enum_var !== d) begin
      $display("FAILED: prev(), expected %d, got %d", d, enum_var);
      pass = 1'b0;
    end

    enum_var = enum_var.prev(0);
    if (enum_var !== d) begin
      $display("FAILED: prev(0), expected %d, got %d", d, enum_var);
      pass = 1'b0;
    end

    enum_var = enum_var.prev(1);
    if (enum_var !== c) begin
      $display("FAILED: prev(1), expected %d, got %d", c, enum_var);
      pass = 1'b0;
    end

    enum_var = enum_var.prev(2);
    if (enum_var !== a) begin
      $display("FAILED: prev(2), expected %d, got %d", a, enum_var);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

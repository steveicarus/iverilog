module test();

  reg [31:0] a, b;
  reg [65:0] a_l, b_l;

  wire [31:0] result = a / b;
  wire [31:0] mod = a % b;
  wire [65:0] result_l = a_l / b_l;
  wire [65:0] mod_l = a_l % b_l;

  initial begin
    a = 'h1;
    b = 'h1;
    a_l = 'h1;
    b_l = 'h1;
    #1; // Need some delay for the calculations to run.
//    b_l = 'h0; // This will now fail with an error.
    $display("Using normal math routines.");
    $display("Result:  %0d\nModulus: %h", result, mod);
    $display("\nUsing wide math routines.");
    $display("Result:  %0d\nModulus: %h", result_l, mod_l);
  end

endmodule

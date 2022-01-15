`define DEF1 "string1"
`define DEF2 "string2\""
`define DEF3 a\b
`define DEF4(a) a

module top;
  initial begin
    $display("Using ``celldefine gives: %s", ``celldefine);
    $display("Plain ``celldefine gives: ", ``celldefine);
    $display("Using `DEF1 gives: %s", `DEF1);
    $display("Using ``DEF1 gives: %s", ``DEF1);
    $display("Plain ``DEF1 gives: ", ``DEF1);

    $display("Using `DEF2 gives: %s", `DEF2);
    $display("Using ``DEF2 gives: %s", ``DEF2);
    $display("Plain ``DEF2 gives: ", ``DEF2);

    $display("Using ``DEF3 gives: %s", ``DEF3);
    $display("Plain ``DEF3 gives: ", ``DEF3);

    $display("Using ``DEF4(\"tmp\") gives: %s", ``DEF4("tmp"));
    $display("Plain ``DEF4(\"tmp\") gives: ", ``DEF4("tmp"));
  end
endmodule

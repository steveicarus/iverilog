/*
 * P1800/D8 22.13
 * "`__FILE__ expands to the name of the current input file, in the form of a
 * string literal."
 */
module aaa;
initial begin
    #1;
    $display(`__FILE__);
end
endmodule

/*
 * P1800/D8 22.13
 * "`__LINE__ expands to the current input line number, in the form of
 * a simple decimal number."
 */
module bbb;
initial begin
    #2;
    if(`__LINE__ !== 21 ||
       `__LINE__ !== 22) begin
       $display("FAIL"); $finish;
   end
   $display("PASSED");
end
endmodule

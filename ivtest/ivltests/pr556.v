/*
 * This test is from PR#556.
 *
 * The output should generate signed and unsigned values
 * from -256 to 256. Also, since the $random sequence is
 * repeatable, it should generate the *same* seqnence
 * every time the program is run.
 */
module test_ran;
integer i,j;
initial
  begin
    for (j=0;j<256;j=j+1)
      begin
        i = $random % 256;
        $display ("The random number is %d",i);
      end
    $finish(0);
  end
endmodule

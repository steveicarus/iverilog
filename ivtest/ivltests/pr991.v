/*
 * This test file is based on PR991.
 */

module bug();
  wire _d1,_d2,test,test1,test2,test3;
  assign _d1 = 1;
  assign _d2 = 0;
  assign test = (_d1 && _d2) != 0;
  assign test1 = (_d1 && _d2) == 0;
  assign test2 = (_d1 && _d2) !== 0;
  assign test3 = (_d1 && _d2) === 0;
  initial begin
    #1;
    $displayb(_d2);   // Should be 0
    $displayb(test);  // Should be 0 (1 && 0) != 0 --> 0 != 0
    $displayb(test1); // Should be 1
    $displayb(test2); // Should be 0
    $displayb(test3); // Should be 1
  end

endmodule

package mypackage;

logic [1:0] array1[] = new[4];
logic [1:0] array2[] = new[4]('{0,1,2,3});

endpackage

module test();

import mypackage::*;

logic [1:0] array3[] = new[4];
logic [1:0] array4[] = new[4]('{0,1,2,3});

reg failed = 0;

initial begin
  foreach (array1[i]) begin
    array1[i] = i;
  end
  foreach (array1[i]) begin
    $display(array1[i]);
    if (array1[i] !== i) failed = 1;
  end
  foreach (array2[i]) begin
    $display(array2[i]);
    if (array2[i] !== i) failed = 1;
  end
  foreach (array3[i]) begin
    array3[i] = i;
  end
  foreach (array3[i]) begin
    $display(array3[i]);
    if (array3[i] !== i) failed = 1;
  end
  foreach (array4[i]) begin
    $display(array4[i]);
    if (array4[i] !== i) failed = 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule

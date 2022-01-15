module constfunc1();

function integer median;

input integer a;
input integer b;
input integer c;

begin
  if (a < b)
    begin
      if (a < c)
        median = (b < c) ? b : c;
      else
        median = a;
    end
  else
    begin
      if (a < c)
        median = a;
      else
        median = (b < c) ? c : b;
    end
end

endfunction

localparam value1 = median(1, 2, 3);
localparam value2 = median(1, 3, 2);
localparam value3 = median(2, 1, 3);
localparam value4 = median(2, 3, 1);
localparam value5 = median(3, 1, 2);
localparam value6 = median(3, 2, 1);

initial begin
  $display("value 1 = %0d", value1);
  $display("value 2 = %0d", value2);
  $display("value 3 = %0d", value3);
  $display("value 4 = %0d", value4);
  $display("value 5 = %0d", value5);
  $display("value 6 = %0d", value6);
  if ((value1 === 2)
  &&  (value2 === 2)
  &&  (value3 === 2)
  &&  (value4 === 2)
  &&  (value5 === 2)
  &&  (value6 === 2))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule

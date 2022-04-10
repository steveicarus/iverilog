module test();

function integer array_value(input integer idx);

  reg [31:0] local_array[1:-1];
  integer i;

  begin
    for (i = -2; i <= 2; i = i + 1) local_array[i] = i;
    array_value = local_array[idx];
  end
endfunction

localparam avm2 = array_value(-2);
localparam avm1 = array_value(-1);
localparam av0  = array_value(0);
localparam avp1 = array_value(1);
localparam avp2 = array_value(2);

initial begin
  if (avm2 === 'bx && avm1 === -1 && av0 === 0 && avp1 === 1 && avp2 === 'bx)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule

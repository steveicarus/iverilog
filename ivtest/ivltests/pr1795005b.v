module test;
reg [63:0] i, j;

initial
main;
task main;
integer k, l, m, n;
begin
i = 64'hffff_ffff_ffff_ffff;
j = 64'hffff_ffff_ffff_fffe;

k = $signed(i) < $signed(j);
l = $signed(i) <= $signed(j);
m = $signed(i) > $signed(j);
n = $signed(i) >= $signed(j);

$display("< : %s", k? "Y":"N");
$display("<=: %s", l? "Y":"N");
$display("> : %s", m? "Y":"N");
$display(">=: %s", n? "Y":"N");
end
endtask
endmodule

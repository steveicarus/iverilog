module test();
real r;

initial
begin
r=0.25;
$write("%f %b %f\n",r, $realtobits(r), $bitstoreal($realtobits(r)));
r=0.5;
$write("%f %b %f\n",r, $realtobits(r), $bitstoreal($realtobits(r)));

$display("neg reals don't work");
r=-0.25;
$write("%f %b %f\n",r, $realtobits(r), $bitstoreal($realtobits(r)));
r=-0.5;
$write("%f %b %f\n",r, $realtobits(r), $bitstoreal($realtobits(r)));

end
endmodule

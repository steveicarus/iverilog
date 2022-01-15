module comp1001a;
// extracted from comp1001.v,
// Copyright (c) 2000 Paul Campbell (paul@verifarm.com)
// GPLv2 or later blah blah blah

reg     [4:0] r170;
initial begin
	r170 = (5'h1c % 25'h5b50) - 20'h05818;
	$displayb("r170 = ",r170);
	if (r170 == 5'b00100) $display("PASSED");
	else $display("FAILED");
end

endmodule

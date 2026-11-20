// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module bare_ref(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,D,10,10,notifier);endspecify
endmodule
module bare_data(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C,D &&& E,10,10,notifier);endspecify
endmodule
module bare_both(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,D &&& E,10,10,notifier);endspecify
endmodule
module zero_ref(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E === 1'b0,D,10,10,notifier);endspecify
endmodule
module zero_data(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C,D &&& E === 1'b0,10,10,notifier);endspecify
endmodule
module zero_both(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E === 1'b0,D &&& E === 1'b0,10,10,notifier);endspecify
endmodule
module one_ref(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E === 1'b1,D,10,10,notifier);endspecify
endmodule
module one_data(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C,D &&& E === 1'b1,10,10,notifier);endspecify
endmodule
module one_both(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E === 1'b1,D &&& E === 1'b1,10,10,notifier);endspecify
endmodule
module tb;
reg C,D,E;
bare_ref bare_ref(C,D,E,E);
bare_data bare_data(C,D,E,E);
bare_both bare_both(C,D,E,E);
zero_ref zero_ref(C,D,E,E);
zero_data zero_data(C,D,E,E);
zero_both zero_both(C,D,E,E);
one_ref one_ref(C,D,E,E);
one_data one_data(C,D,E,E);
one_both one_both(C,D,E,E);
initial begin C=0;D=0;E=0;
#100 D=1;#5 C=1;#3 D=0;#22 C=0;#50 E=1;
#20 D=1;#5 C=1;#3 D=0;#22 C=0;#50 E=1'bx;
#20 D=1;#5 C=1;#3 D=0;#22 C=0;#50 E=1'bz;
#20 D=1;#5 C=1;#3 D=0;#22 C=0;end
initial begin #1;
$shc_population("tb.bare_ref",1);
$shc_watch("tb.bare_ref","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,205,208);
$shc_population("tb.bare_data",1);
$shc_watch("tb.bare_data","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,205,208);
$shc_population("tb.bare_both",1);
$shc_watch("tb.bare_both","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,205,208);
$shc_population("tb.zero_ref",1);
$shc_watch("tb.zero_ref","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,105,108);
$shc_population("tb.zero_data",1);
$shc_watch("tb.zero_data","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,105,108);
$shc_population("tb.zero_both",1);
$shc_watch("tb.zero_both","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,105,108);
$shc_population("tb.one_ref",1);
$shc_watch("tb.one_ref","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,205,208);
$shc_population("tb.one_data",1);
$shc_watch("tb.one_data","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,205,208);
$shc_population("tb.one_both",1);
$shc_watch("tb.one_both","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,205,208);
$shc_start; #449; $shc_finish; $finish; end
endmodule

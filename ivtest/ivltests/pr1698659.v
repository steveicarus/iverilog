`timescale 1ns/10ps

module top;
  reg topvar;
  initial begin
    topvar = 0;
    lwr.lowervar = 1;
    lwr.elwr.evenlowervar = 0;
    othertop.othertopvar = 1;
    #10 $display("%m var is (%b)", topvar);
  end

  lower lwr();
endmodule

module lower;
  reg lowervar;
  initial begin
    #11 $display("%m var is (%b)", lowervar);
  end
  evenlower elwr();
endmodule

module evenlower;
  reg evenlowervar;
  initial begin
    #12 $display("%m var is (%b)", evenlowervar);
    $display("Up reference to me (%b)", elwr.evenlowervar);
    $display("Up reference to parent (%b)", lwr.lowervar);
    $display("Up reference is (%b)", lower.lowervar);
  end
endmodule

module othertop;
  reg othertopvar;
  initial begin
    #20 $display("%m var is (%b)", othertopvar);
  end
endmodule

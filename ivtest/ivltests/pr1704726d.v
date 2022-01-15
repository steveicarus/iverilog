module top;
  parameter parm = 1;

  parameter name0_s = 1; // signal

  wire [1:0] out;

  /***********
   * Check signals
   ***********/
  // Check signal/parameter name issues.
  wire name0_s;

  // Check signal/genvar name issues.
  genvar name0_v;
  generate
    for (name0_v = 0; name0_v < 2; name0_v = name0_v + 1) begin
      assign out[name0_v] = name0_v;
    end
  endgenerate
  wire name0_v;

  // Check signal/task name issues.
  task name1_st;
    $display("FAILED in task name1_st");
  endtask
  wire name1_st;

  // Check signal/function name issues.
  function name2_sf;
    input in;
    name2_sf = in;
  endfunction
  wire name2_sf;

  // Check signal/module instance name issues.
  test name3_si(out[0]);
  wire name3_si;

  // Check signal/named block name issues.
  initial begin: name4_sb
    $display("FAILED in name4_sb");
  end
  wire name4_sb;

  // Check signal/named event name issues.
  event name5_se;
  wire name5_se;

  // Check signal/generate loop name issues.
  genvar i;
  generate
    for (i = 0; i < 2 ; i = i + 1) begin: name6_sgl
      assign out[i] = i;
    end
  endgenerate
  wire name6_sgl;

  // Check signal/generate if name issues.
  generate
    if (parm == 1) begin: name7_sgi
      assign out[1] = 1;
    end
  endgenerate
  wire name7_sgi;

  // Check signal/generate case name issues.
  generate
    case (parm)
      1: begin: name8_sgc
        assign out[1] = 1;
      end
      default: begin: name8_sgc
        assign out[1] = 0;
      end
    endcase
  endgenerate
  wire name8_sgc;

  // Check signal/generate block name issues.
  generate
    begin: name9_sgb
      assign out[0] = 0;
    end
  endgenerate
  wire name9_sgb;

  initial $display("FAILED");
endmodule

module test(out);
  output out;
  reg out = 1'b0;
endmodule

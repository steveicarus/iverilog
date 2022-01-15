module top;
  parameter parm = 1;

  parameter name_v = 1; // genvar
  localparam name_lpv = 0; // genvar

  parameter name_t = 1; // task
  parameter name_f = 1; // function
  parameter name_i = 1; // module instance
  parameter name_b = 1; // named block
  parameter name_gl = 1; // generate block loop
  parameter name_gi = 1; // generate block if
  parameter name_gc = 1; // generate block case
  parameter name_gb = 1; // generate block
  parameter name_e = 1; // named event
  parameter name_s = 1; // signal

  wire [1:0] out;

  /***********
   * Check genvars
   ***********/
  // Check genvar/parameter name issues.
  genvar name_v;
  generate
    for (name_v = 0; name_v < 2; name_v = name_v + 1) begin
      assign out[name_v] = name_v;
    end
  endgenerate

  // Check genvar/localparam name issues.
  genvar name_lpv;
  generate
    for (name_lpv = 0; name_lpv < 2; name_lpv = name_lpv + 1) begin
      assign out[name_lpv] = name_lpv;
    end
  endgenerate

  // Check genvar/genvar name issues.
  // This is in a different file since this fails during parsing.

  /***********
   * Check tasks.
   ***********/
  // Check task/parameter name issues.
  task name_t;
    $display("FAILED in task name_t");
  endtask

  // Check task/task name issues.
  // This is in a different file since this fails during parsing.

  // Check task/genvar name issues.
  genvar name_tv;
  generate
    for (name_tv = 0; name_tv < 2; name_tv = name_tv + 1) begin
      assign out[name_tv] = name_tv;
    end
  endgenerate
  task name_tv;
    $display("FAILED in task name_tv");
   endtask

  /***********
   * Check functions.
   ***********/
  // Check function/parameter name issues.
  function name_f;
    input in;
    name_f = in;
  endfunction

  // Check function/task name issues.
  task name_ft;
    $display("FAILED in task name_ft");
  endtask
  function name_ft;
    input in;
    name_tf = in;
  endfunction

  // Check function/function name issues.
  // This is in a different file since this fails during parsing.

  // Check function/genvar name issues.
  genvar name_fv;
  generate
    for (name_fv = 0; name_fv < 2; name_fv = name_fv + 1) begin
      assign out[name_fv] = name_fv;
    end
  endgenerate
  function name_fv;
    input in;
    name_fv = in;
  endfunction

  /***********
   * Check module instances.
   ***********/
  // Check modul instance/parameter name issues.
  test name_i(out[0]);

  // Check module instance/task name issues.
  task name_it;
    $display("FAILED in task name_it");
  endtask
  test name_it(out[0]);

  // Check module instance/function name issues.
  function name_if;
    input in;
    name_if = in;
  endfunction
  test name_if(out[0]);

  // Check module instance/genvar name issues.
  genvar name_iv;
  generate
    for (name_iv = 0; name_iv < 2; name_iv = name_iv + 1) begin
      assign out[name_iv] = name_iv;
    end
  endgenerate
  test name_iv(out[1]);

  // Check module instance/module instance name issues.
  test name_ii(out[0]);
  test name_ii(out[1]);

  /***********
   * Check named blocks.
   ***********/
  // Check named block/parameter name issues.
  initial begin: name_b
    $display("FAILED in name_b");
  end

  // Check named block/task name issues.
  task name_bt;
    $display("FAILED in task name_bt");
  endtask
  initial begin: name_bt
    $display("FAILED in name_bt");
  end

  // Check named block/function name issues.
  function name_bf;
    input in;
    name_bf = in;
  endfunction
  initial begin: name_bf
    $display("FAILED in name_bf");
  end

  // Check named block/genvar name issues.
  genvar name_bv;
  generate
    for (name_bv = 0; name_bv < 2; name_bv = name_bv + 1) begin
      assign out[name_bv] = name_bv;
    end
  endgenerate
  initial begin: name_bv
    $display("FAILED in name_bv");
  end

  // Check named block/module instance name issues.
  test name_bi(out[0]);
  initial begin: name_bi
    $display("FAILED in name_bi");
  end

  // Check named block/named block name issues.
  initial begin: name_bb
    $display("FAILED in name_bb(a)");
  end
  initial begin: name_bb
    $display("FAILED in name_bb(b)");
  end

  /***********
   * Check named events
   ***********/
  // Check named event/parameter name issues.
  event name_e;

  // Check named event/task name issues.
  task name_et;
    $display("FAILED in task name_et");
  endtask
  event name_et;

  // Check named event/function name issues.
  function name_ef;
    input in;
    name_ef = in;
  endfunction
  event name_ef;

  // Check named event/genvar name issues.
  genvar name_ev;
  generate
    for (name_ev = 0; name_ev < 2; name_ev = name_ev + 1) begin
      assign out[name_ev] = name_ev;
    end
  endgenerate
  event name_ev;

  // Check named event/module instance name issues.
  test name_ei(out[0]);
  event name_ei;

  // Check named event/named block name issues.
  initial begin: name_eb
    $display("FAILED in name_eb");
  end
  event name_eb;

  // Check named event/named event name issues.
  // This is in a different file since this fails during parsing.

  /***********
   * Check generate loop blocks
   ***********/
  genvar i;
  // Check generate loop/parameter name issues.
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_gl
      assign out[i] = i;
    end
  endgenerate

  // Check generate loop/task name issues.
  task name_glt;
    $display("FAILED in task name_glt");
  endtask
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_glt
      assign out[i] = i;
    end
  endgenerate

  // Check generate loop/function name issues.
  function name_glf;
    input in;
    name_glf = in;
  endfunction
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_glf
      assign out[i] = i;
    end
  endgenerate

  // Check generate loop/genvar name issues.
  genvar name_glv;
  generate
    for (name_glv = 0; name_glv < 2; name_glv = name_glv + 1) begin
      assign out[name_glv] = name_glv;
    end
  endgenerate
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_glv
      assign out[i] = i;
    end
  endgenerate

  // Check generate loop/module instance name issues.
  test name_gli(out[0]);
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_gli
      assign out[i] = i;
    end
  endgenerate

  // Check generate loop/named block name issues.
  initial begin: name_glb
    $display("FAILED in name_glb");
  end
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_glb
      assign out[i] = i;
    end
  endgenerate

  // Check generate loop/named event name issues.
  event name_gle;
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_gle
      assign out[i] = i;
    end
  endgenerate

  // Check generate loop/generate loop name issues.
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_glgl
      assign out[i] = i;
    end
  endgenerate
  generate
    for (i = 0; i < 2; i = i + 1) begin: name_glgl
      assign out[i] = i;
    end
  endgenerate

  /***********
   * Check generate if blocks
   ***********/
  // Check generate if/parameter name issues.
  generate
    if (parm == 1) begin: name_gi
      assign out[1] = 1;
    end
  endgenerate

  // Check generate if/task name issues.
  task name_git;
    $display("FAILED in task name_git");
  endtask
  generate
    if (parm == 1) begin: name_git
      assign out[1] = 1;
    end
  endgenerate

  // Check generate if/function name issues.
  function name_gif;
    input in;
    name_gif = in;
  endfunction
  generate
    if (parm == 1) begin: name_gif
      assign out[1] = 1;
    end
  endgenerate

  // Check generate if/genvar name issues.
  genvar name_giv;
  generate
    for (name_giv = 0; name_giv < 2; name_giv = name_giv + 1) begin
      assign out[name_giv] = name_giv;
    end
  endgenerate
  generate
    if (parm == 1) begin: name_giv
      assign out[1] = 1;
    end
  endgenerate

  // Check generate if/module instance name issues.
  test name_gii(out);
  generate
    if (parm == 1) begin: name_gii
      assign out[1] = 1;
    end
  endgenerate

  // Check generate if/named block name issues.
  initial begin: name_gib
    $display("FAILED in name_gib");
  end
  generate
    if (parm == 1) begin: name_gib
      assign out[1] = 1;
    end
  endgenerate

  // Check generate if/named event name issues.
  event name_gie;
  generate
    if (parm == 1) begin: name_gie
      assign out[1] = 1;
    end
  endgenerate

  // Check generate if/generate if name issues.
  generate
    if (parm == 1) begin: name_gigi
      assign out[1] = 1;
    end
  endgenerate
  generate
    if (parm == 1) begin: name_gigi
      assign out[1] = 0;
    end
  endgenerate

  /***********
   * Check generate case blocks
   ***********/
  // Check generate case/parameter name issues.
  generate
    case (parm)
      1: begin: name_gc
        assign out[1] = 1;
      end
      default: begin: name_gc
        assign out[1] = 0;
      end
    endcase
  endgenerate

  // Check generate case/task name issues.
  task name_gct;
    $display("FAILED in task name_gct");
  endtask
  generate
    case (parm)
      1: begin: name_gct
        assign out[1] = 1;
      end
      default: begin: name_gct
        assign out[1] = 0;
      end
    endcase
  endgenerate

  // Check generate case/function name issues.
  function name_gcf;
    input in;
    name_gcf = in;
  endfunction
  generate
    case (parm)
      1: begin: name_gcf
        assign out[1] = 1;
      end
      default: begin: name_gcf
        assign out[1] = 0;
      end
    endcase
  endgenerate

  // Check generate case/genvar name issues.
  genvar name_gcv;
  generate
    for (name_gcv = 0; name_gcv < 2; name_gcv = name_gcv + 1) begin
      assign out[name_gcv] = name_gcv;
    end
  endgenerate
  generate
    case (parm)
      1: begin: name_gcv
        assign out[1] = 1;
      end
      default: begin: name_gcv
        assign out[1] = 0;
      end
    endcase
  endgenerate

  // Check generate case/module instance name issues.
  test name_gci(out[0]);
  generate
    case (parm)
      1: begin: name_gci
        assign out[1] = 1;
      end
      default: begin: name_gci
        assign out[1] = 0;
      end
    endcase
  endgenerate

  // Check generate case/named block name issues.
  initial begin: name_gcb
    $display("FAILED in name_gcb");
  end
  generate
    case (parm)
      1: begin: name_gcb
        assign out[1] = 1;
      end
      default: begin: name_gcb
        assign out[1] = 0;
      end
    endcase
  endgenerate

  // Check generate case/named event name issues.
  event name_gce;
  generate
    case (parm)
      1: begin: name_gce
        assign out[1] = 1;
      end
      default: begin: name_gce
        assign out[1] = 0;
      end
    endcase
  endgenerate

  // Check generate case/generate case  name issues.
  generate
    case (parm)
      1: begin: name_gcgc
        assign out[1] = 1;
      end
      default: begin: name_gcgc
        assign out[1] = 0;
      end
    endcase
  endgenerate
  generate
    case (parm)
      1: begin: name_gcgc
        assign out[1] = 1;
      end
      default: begin: name_gcgc
        assign out[1] = 0;
      end
    endcase
  endgenerate

  /***********
   * Check generate blocks (from 1364-2001)
   ***********/
  // Check generate block/parameter name issues.
  generate
    begin: name_gb
      assign out[0] = 0;
    end
  endgenerate

  // Check generate block/task name issues.
  task name_gbt;
    $display("FAILED in task name_gbt");
  endtask
  generate
    begin: name_gbt
      assign out[0] = 0;
    end
  endgenerate

  // Check generate block/function name issues.
  function name_gbf;
    input in;
    name_gbf = in;
  endfunction
  generate
    begin: name_gbf
      assign out[0] = 0;
    end
  endgenerate

  // Check generate block/genvar name issues.
  genvar name_gbv;
  generate
    for (name_gbv = 0; name_gbv < 2; name_gbv = name_gbv + 1) begin
      assign out[name_gbv] = name_gbv;
    end
  endgenerate
  generate
    begin: name_gbv
      assign out[0] = 0;
    end
  endgenerate

  // Check generate block/module instance name issues.
  test name_gbi(out[0]);
  generate
    begin: name_gbi
      assign out[0] = 0;
    end
  endgenerate

  // Check generate block/named block name issues.
  initial begin: name_gbb
    $display("FAILED in name_gbb");
  end
  generate
    begin: name_gbb
      assign out[0] = 0;
    end
  endgenerate

  // Check generate case/named event name issues.
  event name_gbe;
  generate
    begin: name_gbe
      assign out[0] = 0;
    end
  endgenerate

  // Check generate case/generate case  name issues.
  generate
    begin: name_gbgb
      assign out[0] = 0;
    end
  endgenerate
  generate
    begin: name_gbgb
      assign out[0] = 0;
    end
  endgenerate

  initial $display("FAILED");
endmodule

module test(out);
  output out;
  reg out = 1'b0;
endmodule

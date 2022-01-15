`timescale 100ns/100ps
module dummy;
  parameter [1:0] ipval = 2;
endmodule

`timescale 1us/1ns

module top;
  parameter [1:0] ipval = 2;
  parameter spval = "Help";
  parameter rpval = 1.0;
  event evt;
  reg [1:0] rgval;
  reg rgarr [2:0];
  wire [1:0] wval;
  wire warr [2:0];
  integer ival;
  real rval;
  real rarr [2:0];
  time tval;

  initial begin:blk
    $printtimescale(dummy);
    $printtimescale(dummy.ipval);
    // These should all print a timescale of 1us / 1ns.
    $printtimescale;
    $printtimescale(top.ipval);
    /* This does not currently work because Icarus does not know how
     * to keep the parameter reference in the part select. For now
     * it just returns a constant which the runtime will complain
     * does not have a vpiModule. */
//    $printtimescale(top.ipval[0]);
    $printtimescale(top.spval);
    /* The same goes here. */
//    $printtimescale(top.spval[0]);
    $printtimescale(top.rpval);
    $printtimescale(top.evt);
    $printtimescale(top.rgval);
    $printtimescale(top.rgval[0]);
    $printtimescale(top.rgarr);
    $printtimescale(top.rgarr[0]);
    $printtimescale(top.wval);
    $printtimescale(top.wval[0]);
    $printtimescale(top.warr);
    $printtimescale(top.warr[0]);
    $printtimescale(top.ival);
    $printtimescale(top.ival[1]);
    $printtimescale(top.rval);
    $printtimescale(top.rarr);
    $printtimescale(top.rarr[0]);
    $printtimescale(top.tval);
    $printtimescale(top.blk);
    $printtimescale(top.frk);
    $printtimescale(top.tsk);
    $printtimescale(top.fnc);
  end

  initial fork:frk
    $write("");
  join

  task tsk;
    begin
    end
  endtask

  function integer fnc;
    input integer tmp;
    fnc = 2 * tmp;
  endfunction

endmodule

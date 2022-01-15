`timescale 1 ps / 1 ps
module use_wid_gt_zero_assert (
);

parameter width = 1;
parameter option = "OFF";

reg [width-1:0] dst;

initial
begin
    dst = (option == "ON") ? {width{1'b1}} : {width{1'b0}};
    if (dst === 1'b0)
      $display("PASSED");
    else
      $display("FAILED");
end

endmodule

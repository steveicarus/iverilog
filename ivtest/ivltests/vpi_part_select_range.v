// Check that VPI part-select names preserve the declared range direction.

module test;
  reg [0:7] up;
  reg [7:0] down;
  wire [2:9] up_net;
  wire [9:2] down_net;
  reg [-9:-2] up_negative;
  reg [-2:-9] down_negative;
  wire [-3:4] up_mixed;
  wire [4:-3] down_mixed;

  initial begin
    $vpi_tree(up[0], up[7], up[0:2], up[2:4], up[5:7]);
    $vpi_tree(down[7], down[0], down[7:5], down[5:3], down[2:0]);
    $vpi_tree(up_net[2], up_net[9], up_net[2:4], up_net[4:6], up_net[7:9]);
    $vpi_tree(down_net[9], down_net[2], down_net[9:7], down_net[7:5],
              down_net[4:2]);
    $vpi_tree(up_negative[-9], up_negative[-2], up_negative[-9:-7],
              up_negative[-7:-5], up_negative[-4:-2]);
    $vpi_tree(down_negative[-2], down_negative[-9], down_negative[-2:-4],
              down_negative[-4:-6], down_negative[-7:-9]);
    $vpi_tree(up_mixed[-3], up_mixed[4], up_mixed[-3:-1], up_mixed[-1:1],
              up_mixed[2:4]);
    $vpi_tree(down_mixed[4], down_mixed[-3], down_mixed[4:2], down_mixed[2:0],
              down_mixed[-1:-3]);
  end
endmodule

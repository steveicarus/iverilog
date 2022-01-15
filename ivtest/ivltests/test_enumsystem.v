/***************************************************************
** Author: Oswaldo Cadenas (oswaldo.cadenas@gmail.com)
** Date: September 27 2011
**
** Test: Intended to test the vhd code in enumsystem.vhd
**
** A stimulus modules generates a count 0,1,.., 7, 1 and an enable signal
** A scoreboard forces a check according to the operation found in enumsystem.vhd
**
** The test runs for sometime making sure relevant input conditions are met throughout
**************************************************************************************/

module stim (input clk, reset, output reg [2:0] count, output reg en);

  always @(posedge clk) begin
    if (reset) count <= 3'b0;
    else       count <= count + 1;
end


initial begin
    en = 1;
    repeat (100) @(posedge clk);
    en = 0;
end

endmodule

module scoreboard (input [2:0] count, input reset, en, input [0:3] y);

initial begin
  @(posedge reset);
  @(negedge reset); // waiting for reset to become inactive
  mycheck();
end

task mycheck;
  forever begin
    #1;
    if (en == 0) begin
      if (y !== 4'b0000) begin
        $display ("ERROR");
        $finish;
      end
    end
    else begin
      #2;
       case (count)
          0: if (y !== 4'b1000) begin
               $display("ERROR");
               $finish;
             end
          1: if (y !== 4'b0100) begin
               $display("ERROR");
               $finish;
             end
          2: if (y !== 4'b0010) begin
               $display("ERROR");
               $finish;
             end
          3: if (y !== 4'b0001) begin
               $display("ERROR");
               $finish;
             end
          default: if (y !== 4'b1111 && en == 1) begin
                     $display("ERROR here, en = %d", en);
                     $finish;
                   end
      endcase
    end // else
end // always
endtask

endmodule


module test;
parameter T = 10;
parameter S = 2*10*150;

bit clk = 0, reset = 0;
wire en;
wire [2:0] count;
wire [0:3] y;


initial forever #(T) clk = !clk;

 initial begin
    @(negedge clk);
    reset = 1'b1;
    repeat(6) @(negedge clk);
    reset = 1'b0;
  end

stim stim (.clk(clk), .reset(reset), .en(en), .count(count) );
enumsystem duv (.clk(clk), .reset(reset), .en(en), .y(y) );
scoreboard check (.en(en), .reset(reset), .count(count), .y(y) );

initial begin
    #S;
    $display("PASSED");
    $finish;
end


endmodule

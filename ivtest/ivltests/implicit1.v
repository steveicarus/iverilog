/*
 * From PR#379
 */
`define  IDLE 2'b00
`define COUNT 2'b01
`define  DONE 2'b10

module Counter56 (POR, CLK, VoltageOK, ChargeDone );
input  POR;
input  CLK;
input  VoltageOK;
output ChargeDone;

reg  [1:0] CounterState, nextCounterState;

wire [8:0] nextMinuteCounter;

always @(posedge CLK or negedge POR)
  if (!POR)
     CounterState = 2'b00;
  else
     CounterState = nextCounterState;

always @(VoltageOK or CounterReset) // CounterReset should make an error
   casez (CounterState)
       `IDLE: begin
                  nextCounterState = (VoltageOK) ? `COUNT : `IDLE;
               end
       `COUNT: begin
                  nextCounterState = (VoltageOK) ? `COUNT : `IDLE;
               end
        `DONE: begin
                  nextCounterState = `DONE;
               end
      default: begin
                  nextCounterState = 2'bxx;
               end
   endcase

endmodule

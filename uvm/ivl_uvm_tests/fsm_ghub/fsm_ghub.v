//For starting from a particular state(s0), reset input has also been used which was not present in the state machine diagram. 
`timescale 1ns/1ns
module fsm(
  input A,
  input clock,
  input reset,
  output [1:0]Z
);

reg [1:0] present_state,next_state;
parameter s0 = 0, s1 = 1, s2 = 2, s3 = 3;

// Block for defining what happens when reset is asserted and if not
always @(posedge clock)  
begin 
if (reset) 
  present_state <= s0; 
else 
  present_state <= next_state; 
    end 

    //State Transition block
    always @(*)
      case(present_state)
	s0:begin
	  case(A)
	    1'b0:next_state=s3; 
	    1'b1:next_state=s1; 
	  endcase
	end
	s1:begin
	  case(A)
	    1'b0:next_state=s0; 
	    1'b1:next_state=s2; 
	  endcase
	end
	s2:begin
	  case(A)
	    1'b0:next_state=s1; 
	    1'b1:next_state=s3; 
	  endcase
	end
	s3:begin
	  case(A)
	    1'b0:next_state=s2; 
	    1'b1:next_state=s0; 
	  endcase
	end
      endcase

      assign Z = present_state; 
      endmodule

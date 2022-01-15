// This slightly convoluted module used to cause an argument-size
// mismatch in the call to the function extend_data

module test (c);

 parameter MAX_SIZE = 32;

 input			c;

 reg	[MAX_SIZE-1:0]	d,
			e,
			f;
 reg	[7:0]		g;

 wire			h;

 always @(posedge h or negedge c)
  if (~c)
   f <= #2 {MAX_SIZE{1'b0}};
  else
   case (g)
    8'h18 :
      f <= #2 hw(d, e);
    default :
     f <= #2 {MAX_SIZE{1'b0}};
   endcase

 parameter FALSE_RESULT = {MAX_SIZE{1'b0}},
	   TRUE_RESULT = FALSE_RESULT | 1'b1;

 function integer sign_of;

  input	[2*MAX_SIZE-1:0]	data;
  input				size;

  reg	[2*MAX_SIZE-1:0]	data;
  integer			size;

  if (data[size-1]===1'b1)
   sign_of = -1;
  else
   sign_of = 1;
 endfunction

 function [2*MAX_SIZE-1:0] extend_data;

  input	[2*MAX_SIZE-1:0]	data;
  input				size;
  input				new_size;
  input				extend;

  reg	[2*MAX_SIZE-1:0]	data;
  integer			size,
				new_size;
  reg				extend;

  for (extend_data = data ; new_size-size>0 ; new_size=new_size-1)
   extend_data[new_size-1] = extend & data[size-1];
 endfunction

 function [MAX_SIZE-1:0] hw;
  input	[MAX_SIZE-1:0]		a;
  input	[MAX_SIZE-1:0]		b;

  reg	[MAX_SIZE-1:0]		a,
				b;

  reg	[MAX_SIZE:0]		diff;

  begin
   diff = extend_data(b, MAX_SIZE, MAX_SIZE+1, 1'b1) -
          extend_data(a, MAX_SIZE, MAX_SIZE+1, 1'b1);
   if (sign_of(diff, MAX_SIZE+1)==-1)
    hw = TRUE_RESULT;
   else
    hw = FALSE_RESULT;
  end
 endfunction

endmodule

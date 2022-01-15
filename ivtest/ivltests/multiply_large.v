//
//    multiply_large.v
//
//    Copyright (c) 2001 ajb
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//

`define W (65)	// any value past 32 will suffice

module multiplier(a,b,sum);
parameter N=1;

input[N-1:0] a, b;
output[N-1:0] sum;

reg [(N-1)*2:0] tmp;
integer i;

always @(a or b)
	begin
	tmp = 0;
	for(i=0;i<N;i=i+1)
		begin
		if(b[i]) tmp=tmp + ({{N{1'b0}}, a} << i);
		end
	end

assign sum = tmp[N:0];

endmodule


module multiply_large;

reg [0:`W-1] a, b;
wire [0:`W-1] c, s;
reg [0:`W-1] d;
integer i, j;
reg rc;

assign c=a*b;
multiplier #(`W) mpy(a,b,s);

initial
begin

rc=0;
a={{`W-1{1'b0}}, 1'b1};		// walking 1s
for(i=0;i<`W;i=i+1)
	begin
	b={{`W-1{1'b0}}, 1'b1};
	for(j=0;j<`W;j=j+1)
		begin
                #1;
		d=a*b;
		if ((c!=d)||(c!=s))
			begin
			$display("a=%h b=%h c=%h d=%h s=%h", a,b,c,d,s);
			rc=1;
			end
		b={b[1:`W-1], 1'b0};
		end
	a={a[1:`W-1], 1'b0};
	end

a={{`W-1{1'b1}}, 1'b0};		// walking 0s
for(i=0;i<`W;i=i+1)
	begin
	b={{`W-1{1'b1}}, 1'b0};
	for(j=0;j<`W;j=j+1)
		begin
                #1;
		d=a*b;
		if ((c!=d)||(c!=s))
			begin
			$display("a=%h b=%h c=%h d=%h s=%h", a,b,c,d,s);
			rc=1;
			end
		b={b[1:`W-1], 1'b1};
		end
	a={a[1:`W-1], 1'b1};
	end

a={{`W-2{1'b0}}, 2'b11};	// walking 11s
for(i=0;i<`W;i=i+1)
	begin
	b={{`W-2{1'b0}}, 2'b11};
	for(j=0;j<`W;j=j+1)
		begin
                #1;
		d=a*b;
		if ((c!=d)||(c!=s))
			begin
			$display("a=%h b=%h c=%h d=%h s=%h", a,b,c,d,s);
			rc=1;
			end
		b={b[1:`W-1], 1'b0};
		end
	a={a[1:`W-1], 1'b0};
	end

a={{`W-2{1'b1}}, 2'b00};	// walking 00s
for(i=0;i<`W;i=i+1)
	begin
	b={{`W-2{1'b1}}, 2'b00};
	for(j=0;j<`W;j=j+1)
		begin
                #1;
		d=a*b;
		if ((c!=d)||(c!=s))
			begin
			$display("a=%h b=%h c=%h d=%h s=%h", a,b,c,d,s);
			rc=1;
			end
		b={b[1:`W-1], 1'b1};
		end
	a={a[1:`W-1], 1'b1};
	end

a={{`W-3{1'b0}}, 3'b101};	// walking 101s
for(i=0;i<`W;i=i+1)
	begin
	b={{`W-3{1'b0}}, 3'b101};
	for(j=0;j<`W;j=j+1)
		begin
                #1;
		d=a*b;
		if ((c!=d)||(c!=s))
			begin
			$display("a=%h b=%h c=%h d=%h s=%h", a,b,c,d,s);
			rc=1;
			end
		b={b[1:`W-1], 1'b0};
		end
	a={a[1:`W-1], 1'b0};
	end

a={{`W-3{1'b1}}, 3'b010};	// walking 010s
for(i=0;i<`W;i=i+1)
	begin
	b={{`W-3{1'b1}}, 3'b010};
	for(j=0;j<`W;j=j+1)
		begin
                #1;
		d=a*b;
		if ((c!=d)||(c!=s))
			begin
			$display("a=%h b=%h c=%h d=%h s=%h", a,b,c,d,s);
			rc=1;
			end
		b={b[1:`W-1], 1'b1};
		end
	a={a[1:`W-1], 1'b1};
	end

if (rc) $display("FAILED"); else $display("PASSED");
$finish;
end

endmodule

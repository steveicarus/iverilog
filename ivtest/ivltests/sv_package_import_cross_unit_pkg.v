package p;

  // Make the imported declaration positions greater than the reference
  // positions in the separate compilation unit.
  integer pad00, pad01, pad02, pad03, pad04, pad05, pad06, pad07;
  integer pad08, pad09, pad10, pad11, pad12, pad13, pad14, pad15;
  integer pad16, pad17, pad18, pad19, pad20, pad21, pad22, pad23;
  integer pad24, pad25, pad26, pad27, pad28, pad29, pad30, pad31;

  integer variable_value = 42;
  reg [31:0] array_value [0:1];
  event event_value;
  parameter integer parameter_value = 17;
  localparam integer localparam_value = 23;
  typedef enum integer { enum_value = 31 } enum_type;

endpackage

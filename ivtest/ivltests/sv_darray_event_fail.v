module sv_darray_event_fail;
  int values[];
  int queue[$];
  int index = 1;
  int packed_value;
  int static_array[2];

  initial @(values[1]);
  initial @(values[index]);
  initial @(posedge values[1]);
  initial @(queue[1]);
  initial @(-values[1]);
  initial @(packed_value[values[1]]);
  initial @(static_array[values[1]]);
endmodule

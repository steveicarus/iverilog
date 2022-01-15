module top;
  integer res, id, job, inform, status, code, value;
  reg [30:0] short_job, short_inform, short_stat;

  initial begin
    id = 1;
    $q_initialize();
    $q_initialize(1);
    $q_initialize(id);
    $q_initialize("A name");
    $q_initialize(id, 1);
    $q_initialize(id, 2);
    $q_initialize(id, "A type");
    $q_initialize(id, 1, 20);
    $q_initialize(id, 1, "A size");
    $q_initialize(id, 1, 20, "A status");
    $q_initialize(id, 1, 20, short_stat);
    $q_initialize(id, 1, 20, status, "extra");

    $q_add();
    $q_add(1);
    $q_add(id);
    $q_add("A name");
    $q_add(id, 1);
    $q_add(id, "A job");
    $q_add(id, 1, 45);
    $q_add(id, 1, "An inform");
    $q_add(id, 1, 45, short_stat);
    $q_add(id, 1, 45, status, "extra");

    $q_remove();
    $q_remove(1);
    $q_remove(id);
    $q_remove("A name");
    $q_remove(id, job);
    $q_remove(id, "A job");
    $q_remove(id, job, inform);
    $q_remove(id, job, "An inform");
    $q_remove(id, job, inform, short_stat);
    $q_remove(id, short_job, short_inform, status);
    $q_remove(id, job, inform, status, "extra");

    res = $q_full(1);
    res = $q_full(id);
    res = $q_full("A name");
    res = $q_full(id, short_stat);
    res = $q_full(id, status, "extra");

    $q_exam();
    $q_exam(1);
    $q_exam(id);
    $q_exam("A name");
    $q_exam(id, 1);
    $q_exam(id, code);
    $q_exam(id, "A code");
    $q_exam(id, code, value);
    $q_exam(id, code, short_job);
    $q_exam(id, code, "A value");
    $q_exam(id, code, value, short_stat);
    $q_exam(id, code, value, status, "extra");
  end
endmodule

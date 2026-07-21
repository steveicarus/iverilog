# Mailbox / semaphore (Tier A #5)

Status: **partial** — compiler builtins for Accellera-style IPC (int messages).

## Choice: compiler builtins (not library classes)

IEEE `mailbox` / `semaphore` are built-in classes. A pure SV library cannot implement the required API here because:

- Functions cannot take `output`/`ref` ports (`try_get(v)` needs a side-effecting write).
- Class `event` properties are not supported yet (needed for efficient wait/wakeup).

This fork therefore implements **native builtins**, similar in shape to `string` / queue method sfuncs: empty `netclass_t` types named `mailbox` / `semaphore`, methods lowered to `$ivl_mailbox$*` / `$ivl_semaphore$*`, and vvp opcodes (`%mbx/*`, `%sem/*`) with real thread suspend/resume.

The older IVL_UVM “poor-man’s” classes (`ivl_uvm_mbx`, `ivl_uvm_semaphore`) remain in `uvm/` for that library’s own tests; new Accellera-shaped code should use the builtins.

## Supported in this slice

```systemverilog
mailbox #(int) mb = new();   // #(T) accepted; element type is int/vec for v1
mb.put(7);
mb.get(v);                   // blocking
ok = mb.try_put(8);
ok = mb.try_get(v);          // nonblocking; writes v on success
n = mb.num();

semaphore sem = new(1);
sem.get(1);                  // blocking
sem.put(1);
ok = sem.try_get(1);
```

Also: unbounded `new()` / `new(0)`, bounded mailbox `new(N)`, blocking put when full, `peek` / `try_peek`.

## Encoding

| Layer | Role |
|-------|------|
| Parse / pform | Builtin typedefs `mailbox` / `semaphore`; `mailbox #(T) x` as var decl (T ignored for now) |
| Netlist | Empty `netclass_t` “mailbox” / “semaphore” |
| Elab | `$ivl_mailbox$new/put/get/...`, `$ivl_semaphore$new/get/put/try_get` |
| Codegen | `%mbx/*`, `%sem/*`, `%box/vec4` / `%unbox/vec4` for int payloads |
| Runtime | `vvp_mailbox` / `vvp_semaphore` (`vvp/vvp_mailbox.{h,cc}`) |

## Deferred (do not claim)

- True parameterized element types (`mailbox #(string)`, class handles) beyond boxing ints
- Explicit specialization of user param-classes interacting with mailbox
- `peek` stress / priority / typed message checking
- Replacing IVL_UVM’s `ivl_uvm_mbx` usages with the builtin

## Example

[`examples/mailbox_sem/mbx_sem_basic.sv`](../examples/mailbox_sem/mbx_sem_basic.sv) — prints `PASSED`.

```bash
./install/bin/iverilog -g2012 -o /tmp/mbx_sem.vvp examples/mailbox_sem/mbx_sem_basic.sv
./install/bin/vvp /tmp/mbx_sem.vvp
```

See also [STATUS.md](STATUS.md) and [ROADMAP.md](ROADMAP.md).

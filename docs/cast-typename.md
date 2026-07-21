# `$cast` / `$typename` (Tier A #7, first slice)

Status: **partial** — class-handle `$cast` and static `$typename` for common UVM factory-style patterns.

## Supported in this slice

```systemverilog
class base; endclass
class derived extends base;
  int x;
  function new(); x = 7; endfunction
endclass

base b;
derived d, d2;
bit ok;
string tn;

d = new;
b = d;
ok = $cast(d2, b);          // 1 if b's dynamic type is-a derived
tn = $typename(d);          // "class derived"
```

### `$cast(dest, src)`

| Form | Behavior |
|------|----------|
| Function `ok = $cast(dest, src)` | Returns `1` on success, `0` on failure; on success assigns `src` into `dest` |
| Task `$cast(dest, src);` | Same assign/check; return value discarded (no runtime fatal on failure yet) |

- **Class handles only** (destination must be a simple class variable)
- Success when `src` is non-null and its **runtime** class is `dest`'s declared type or a subclass (inheritance walk)
- Failure leaves `dest` unchanged and returns `0`
- Null `src` → failure

### `$typename(expr)` / `$typename(type)`

Returns a **compile-time** string for the **static** type of the argument (optional second length argument is accepted and ignored, for Accellera `uvm_typename` compatibility).

| Argument | String format |
|----------|----------------|
| Class variable / type | `class <name>` (e.g. `class derived`) |
| `string` | `string` |
| `real` | `real` |
| `bit` / `logic` (and other integral) | `bit` / `logic` (width not included yet) |
| Anything else | `unknown` |

**Limitation:** `$typename` does **not** report the dynamic (runtime) type of a class handle. `$typename(b)` is `"class base"` even when `b` holds a `derived` object. Use `$cast` / virtual `get_type_name()`-style methods for dynamic discrimination.

## Encoding

| Layer | Role |
|-------|------|
| Elab | `$cast` → `$ivl_cast(dest, src)`; `$typename` → `NetECString` |
| Codegen | `%cast/cobj C<dest_type>, v<dest>` after evaluating `src` |
| Runtime | Walk `class_type` super chain (`is_a`); store handle on success |
| Class emit | `.class "name" [n], C<sup>` records inheritance for the walk |

## Deferred (do not claim)

- Non-class `$cast` (integrals, enums, etc.)
- Destinations that are class properties / selects (`obj.m`, `arr[i]`)
- Task-form fatal on failed cast (IEEE optional behavior)
- Dynamic `$typename` / full IEEE type-string formatting (packed ranges, enums, packages)
- `$typename` second-argument truncation

## Example

[`examples/cast_typename/cast_typename_basic.sv`](../examples/cast_typename/cast_typename_basic.sv) — prints `PASSED`.

```bash
./install/bin/iverilog -g2012 -o /tmp/cast.vvp examples/cast_typename/cast_typename_basic.sv
./install/bin/vvp /tmp/cast.vvp
```

See also [STATUS.md](STATUS.md) and [ROADMAP.md](ROADMAP.md).

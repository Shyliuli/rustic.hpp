# rustic.hpp

Rust-inspired utilities for modern C++ projects. The single header brings Option/Result, match-style dispatch, and trait/impl ergonomics without adding dependencies or build steps.

## Features
- Rust-like error model: `Option` and `Result` with boolean and pointer semantics, plus `match` helpers for explicit branching.
- Lightweight syntax sugar: `fn`, `let`, `let_mut`, `Case`, `DefaultCase`, and access modifiers (`pub`, `prot`, etc.).
- Trait-style macros: `trait`/`impl` plus `from`/`data` to separate interfaces and storage.
- Header-only, zero third-party dependencies; relies only on the C++17/20 standard library.

## Requirements
- Prefer C++20 because `std::format` is used. If your project does not need format, the rest of the header works on C++17.
- No build steps beyond including the header.

## Installation
1. Copy `rustic.hpp` into your include path.
2. Include it in translation units that need the utilities:
   ```cpp
   #include "rustic.hpp"
   ```
3. To opt in selectively, define macros before the include:
   - `DO_NOT_ENABLE_ALL_RUSTIC` to disable auto-enabling everything.
   - `ENABLE_RS_KEYWORD` to enable syntax sugar (fn/let/pub, Case/DefaultCase).
   - `ENABLE_RS_ERROR` to enable `Option`, `Result`, and `panic`.
   - `ENABLE_RS_OBJECT` to enable trait/impl and inheritance helpers.

Example of enabling only the error model:
```cpp
#define DO_NOT_ENABLE_ALL_RUSTIC
#define ENABLE_RS_ERROR
#include "rustic.hpp"
```

## Quick start
```cpp
#include "rustic.hpp"
fn divide(f64 a, f64 b)->Result<f64, String> {
    if (b == 0) return Err(String("divide by zero"));
    return Ok(a / b);
}

fn main()->int {
    divide(6, 3).match(
        Case(val){ std::cout << val << '\n'; },
        Case(err){ std::cerr << err << '\n'; }
    );
    return 0;
}
```

## Syntax sugar
- `fn` expands to `auto` for return-type deduction.
- `let` creates an immutable binding (`const auto`), `let_mut` creates a mutable binding (`auto`).
- Access modifiers: `pub`, `priv`, `prot`.
- Pattern helpers: `Case(x)` to capture a value, `DefaultCase()` when no value is needed.

## Error handling
`Option<T>` models an optional value; `Result<T, E>` models success or failure. Both provide:
- Boolean semantics (`if (opt) { ... }`).
- Pointer semantics (`opt->field`, `*res`), which call `unwrap()` internally.
- `match` dispatch that enforces handling both branches.

Example (login flow):
```cpp
fn login(String name, String passwd)->Result<Unit, String> {
    return get_user(name).match(
        Case(user)->Result<Unit, String>{
            if (user->password == passwd) return Ok();
            return Err(String("invalid password"));
        },
        DefaultCase()->Result<Unit, String>{
            return Err(String("user not found"));
        }
    );
}
```

## Object model
Trait-style interfaces keep behavior separate from data. Recommended shape: `class Foo : from MyTrait, data FooState`.

```cpp
trait(Renderable,
    must(draw() -> void);
    def(area() -> f32) { return 0; }
);

struct CircleData { f32 r; };

class Circle : from Renderable, data CircleData {
pub:
    Circle(f32 r) : CircleData{r} {}
    impl(draw() -> void) {
        std::cout << "Circle r=" << r << '\n';
    }
};
```

## Notes and tips
- `panic` aborts the process; use it for truly unrecoverable states only.
- Ensure match branches return the same type when you return values from them.
- `Ok()` is available for `Result<Unit, E>` when you need to signal success without data.
- Because everything lives in a header, prefer including it in translation units rather than precompiled headers unless you know your build system handles macros consistently.

## Local testing
The library itself is header-only; add unit tests in your project using your preferred framework. The sample snippets in `src/` of your project can be compiled with standard C++ toolchains (`g++ -std=c++20` or newer).

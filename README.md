# rustic.hpp

English | [中文说明](README_ZH.md)

Rust-inspired utilities for modern C++ projects. This header delivers Option/Result, match-style dispatch, and trait/impl ergonomics without extra dependencies or build steps. The guide below is deliberately thorough so you can adopt the library in real services, tools, and classroom projects with confidence.

## Contents
- What this library provides
- Compatibility and build notes
- Installation and configuration
- Module deep dive
  - Syntax sugar and aliases (fn, let, i32/u32, Vec)
  - Error model (Option, Result, panic, match, unwrap family)
  - Object model (trait/impl, from/data/inner, pub)
- Patterns and best practices
- Integration examples
- Limitations and cautions
- Local testing checklist

## What this library provides
- Rust-like error model: `Option` and `Result` with boolean and pointer semantics, plus `match` helpers (`Case`, `DefaultCase`) for explicit branching.
- Rust-style aliases and binding sugar: `i32/u32`, `f64`, `Vec<T>`, `String`, plus `fn`, `let`, `let_mut`.
- Trait-style macros: `trait`/`impl` plus `from`/`data` to separate interfaces and storage, with `pub`/`inner` for public surface vs. implementation.
- Header-only, zero third-party dependencies; relies only on the C++17/20 standard library.

## Compatibility and build notes
- Prefer C++20 because `std::format` is used. If your project does not rely on format, the rest of the header builds under C++17.
- No global initializers; safe to include in multiple translation units.
- Macro configuration is per translation unit. Keep the same macro set across files to avoid inconsistent interfaces.

## Installation and configuration
1. Place `rustic.hpp` in your include path.
2. Include where needed:
   ```cpp
   #include "rustic.hpp"
   ```
3. Optional macros before the include:
   - `DO_NOT_ENABLE_ALL_RUSTIC` disables auto-enabling everything.
   - `ENABLE_RS_KEYWORD` enables type aliases and binding sugar (i32/u32, Vec, fn/let/let_mut).
   - `ENABLE_RS_ERROR` enables `Option`, `Result`, `panic`, and `Case/DefaultCase`.
   - `ENABLE_RS_OBJECT` enables trait/impl and inheritance helpers including `pub`/`inner`.

Example: enable only the error model
```cpp
#define DO_NOT_ENABLE_ALL_RUSTIC
#define ENABLE_RS_ERROR
#include "rustic.hpp"
```

## Module deep dive

### Syntax sugar and aliases (ENABLE_RS_KEYWORD)
- Type aliases: `i8/i16/i32/i64`, `u8/u16/u32/u64`, `f32`, `f64`, `usize`, `isize`, `String` (`std::string` alias), `Vec<T>` (`std::vector<T>` alias).
- Bindings: `fn` expands to `auto` for return-type deduction; `let` becomes `const auto`; `let_mut` becomes `auto`.
- Access modifiers are defined with the object model (see below).

Use these to reduce boilerplate while keeping intent readable. They are macros, so avoid local identifiers that collide with the macro names.

### Error model (ENABLE_RS_ERROR)
`Option<T>` and `Result<T, E>` enforce explicit handling and can be combined with pattern-style dispatch.

Key operations on `Option<T>`:
- Construction: `Some(value)`, `None()`.
- Queries: `is_some()`, `is_none()`, boolean cast (`if (opt) { ... }`).
- Access:
  - `unwrap()`: returns a reference if present, otherwise aborts via `panic`.
  - `expect(msg)`: same as `unwrap()` but with a custom message.
  - `unwrap_or(default)`: returns the contained value or the provided default copy.
- Pointer semantics: `opt->method()` and `*opt` call `unwrap()` internally.
- Matching: `opt.match(Case(v){...}, DefaultCase(){...});` using the `Case`/`DefaultCase` helpers provided by the error module. Branch lambdas may or may not take parameters.

Key operations on `Result<T, E>`:
- Construction: `Ok(value)`, `Err(error)`, and `Ok()` for `Result<Unit, E>`.
- Queries: `is_ok()`, `is_err()`, boolean cast.
- Access: `unwrap()`, `unwrap_err()`, `expect(msg)`.
- Pointer semantics identical to `Option`.
- Matching: `res.match(Case(val){...}, Case(err){...});` with consistent return types across branches.

Panic:
- `panic(msg)` and `rs_panic` abort the process. Use only for unrecoverable states.

Unwrap family: when to use which
- Prefer `match` for branching and logging, then return `Result` or `Option`.
- Use `unwrap()` only when the absence of a value is truly impossible (logic guaranteed by earlier checks).
- Use `expect(msg)` when you want a clearer crash message for debugging.
- Use `unwrap_or(default)` at boundaries where a safe fallback is acceptable and the fallback is cheap to copy.

### Object model (ENABLE_RS_OBJECT)
Macros that emulate Rust-style traits:
- `trait(Name, ...)` defines a pure-virtual base with a virtual destructor.
- `must(...)` declares pure virtual functions that subclasses must implement.
- `def(...)` declares virtual functions with defaults. It can supply a default implementation inside the trait and can also be used to add new virtual methods in the trait body; it is a thin wrapper over `virtual` with an optional definition.
- `impl(...)` expands to `override` in implementations.
- `from` and `data` are public and protected inheritance aliases. Recommended shape: `class Foo : from BarTrait, data BarState { inner: pub: };` to separate interface/data and make access levels explicit.
- `inner` is a protected shorthand for implementation details, and `pub` maps to `public` for the external surface.

Example:
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

## Patterns and best practices
- Prefer `match` over nested `if` chains so success and failure are both explicit.
- In library code, avoid `unwrap()` unless the failure is impossible. Return `Result` or `Option` and let callers decide.
- Use `unwrap_or` at the boundary when you have a safe fallback value.
- Combine boolean and pointer semantics for concise reads:
  ```cpp
  if (auto user = get_user(id)) {
      std::cout << user->name;
  }
  ```
- Keep trait state in a separate `data` base to prevent interface inheritance from carrying storage members.
- If you only need part of the library, gate features with macros to minimize namespace noise.
- When returning values from `match`, ensure all branches return the same type; otherwise, rely on side effects only.

## Integration examples

### Login flow with explicit errors
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

### Safe numeric parsing with Option
```cpp
let parse_i32 = [](std::string_view text)->Option<i32> {
    i32 value{};
    auto first = text.data();
    auto last = first + text.size();
    auto res = std::from_chars(first, last, value);
    if (res.ec == std::errc{} && res.ptr == last) return Some(value);
    return None();
};
```

### Trait-based rendering
```cpp
trait(Renderable,
    must(draw() -> void);
);

struct SquareData { f32 side; };

class Square : from Renderable, data SquareData {
pub:
    Square(f32 s) : SquareData{s} {}
    impl(draw() -> void) {
        std::cout << "Square side=" << side << '\n';
    }
};
```

### Match with value-returning branches
```cpp
fn price_tag(f64 price)->String {
    return Some(price).match(
        Case(p)->String{
            return std::format("${:.2f}", p);
        },
        DefaultCase()->String{
            return String("N/A");
        }
    );
}
```

## Limitations and cautions
- Macros are visible per translation unit. Keep configuration consistent to avoid surprising differences.
- `panic` always aborts; there is no recovery path.
- `std::format` requires C++20. Under C++17, avoid format in your own code or provide a different formatting function.
- As a header-only library, any change requires recompiling translation units that include it.

## Local testing checklist
- Compile a minimal sample with your usual flags:
  ```bash
  g++ -std=c++20 -Wall -Wextra -pedantic sample.cpp -o sample
  ```
- Add unit tests that exercise both success and error paths for functions returning `Result` or `Option`.
- If you rely on trait macros, test multiple derived types to confirm overrides are correctly marked with `impl(...)`.

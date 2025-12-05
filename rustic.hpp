// -----------------------------------------------------------------------------
// rustic.hpp - Rust-inspired utilities for modern C++
// -----------------------------------------------------------------------------
// Goals
// - Bring Rust-like expressiveness (Option/Result, match, trait/impl) to small
//   and medium C++ codebases.
// - Keep the macros lightweight and readable; avoid opaque metaprogramming.
// - Header-only with zero third-party dependencies, relying solely on the C++17
//   and C++20 standard library (variant, lambdas, format, etc.).
//
// Build targets
// - Prefer C++20 because std::format is used. If your project does not need
//   format, you can build with C++17.
// - Header-only, no global initializers, no extra build steps.
//
// Quick start
//   #include "rustic.hpp"
//   fn divide(f64 a, f64 b)->Result<f64, String> {
//       if (b == 0) return Err(String("divide by zero"));
//       return Ok(a / b);
//   }
//   fn main()->int {
//       divide(6, 3).match(
//           Case(val){ std::cout << val << '\n'; },
//           Case(err){ std::cerr << err << '\n'; }
//       );
//       return 0;
//   }
//
// Module map
// 0. Configuration: toggle syntax sugar, error handling, and the object model.
// 1. Syntax sugar: Rust-style aliases (i32, f64, Vec<T>...) and fn/let/let_mut.
// 2. Error handling: Option/Result with bool and pointer semantics plus match
//    helpers (Case/DefaultCase).
// 3. Object model: trait/impl macros, from/data/inner, pub for public surface.
//
// =============================================================================
// 0. Configuration
// =============================================================================
// All modules are enabled by default. To opt in selectively:
// 1. Define `DO_NOT_ENABLE_ALL_RUSTIC` before including this header.
// 2. Enable any combination of:
//    - `ENABLE_RS_KEYWORD`: fn, let, let_mut.
//    - `ENABLE_RS_ERROR`  : Option, Result, panic, and Case/DefaultCase helpers.
//    - `ENABLE_RS_OBJECT` : trait, impl, from, data, inner, pub macros.
// Tip: modules are independent; you can keep Result without syntax sugar, or use
// trait macros alone.
//
// Example configuration
//    #define DO_NOT_ENABLE_ALL_RUSTIC
//    #define ENABLE_RS_ERROR    // Use only Option/Result
//    #include "rustic.hpp"
//
// =============================================================================
// 1. Primitives
// =============================================================================
// Rust-style aliases to reduce typing while keeping intent clear:
//
// | Rust style | C++ native type    | Notes |
// |:-----------|:-------------------|:------|
// | i8/u8      | int8_t / uint8_t   |       |
// | i32/u32    | int32_t / uint32_t |       |
// | f32/f64    | float / double     |       |
// | usize      | size_t             |       |
// | String     | std::string        | Alias only, not Rust's memory model |
// | Vec<T>     | std::vector<T>     |       |
//
// =============================================================================
// 2. Keywords & Syntax Sugar
// =============================================================================
// Requires: ENABLE_RS_KEYWORD
//
// Functions and variables
// - `fn`      -> `auto`       : return type deduction (C++14+).
// - `let`     -> `const auto` : immutable by default.
// - `let_mut` -> `auto`       : mutable binding.
//
// Example
//   fn add(i32 a, i32 b) -> i32 { return a + b; }
//   let pi = 3.14;
//   let_mut count = 0;
//
// =============================================================================
// 2. Error Handling & Matching
// =============================================================================
// Requires: ENABLE_RS_ERROR
// Intent: force explicit handling of success or failure instead of silent
// ignoring. Option and Result provide both boolean and pointer semantics and can
// be combined with match-style dispatch.
//
// A. Option<T> - a value that may be absent
//    - Replacement for raw pointers (`nullptr`) or `std::optional`.
//    - Construction: `Some(val)` or `None()`.
//    - Access:
//      - `unwrap()`: returns a reference or panics if None.
//      - `expect(msg)`: same as unwrap with a custom panic message.
//      - Pointer semantics: `opt->method()` and `*opt` behave like unwrap; check
//        before dereferencing if you need safety.
//
// B. Result<T, E> - success or failure
//    - Replacement for exceptions or error codes.
//    - Construction: `Ok(val)` or `Err(err)`. Use `Ok()` when T is `Unit`.
//    - Access:
//      - `unwrap()`: panics if Err.
//      - `unwrap_err()`: panics if Ok.
//
// C. Match
//    - Syntax: `obj.match( Case(val){...}, Case(err){...} )`
//    - `Case(x)`: captures the lambda argument.
//    - `DefaultCase()`: captures no argument (for None or when the value
//      content is irrelevant).
//    - Note: all branches must agree on return type when a value is returned.
//
//    Example
//      fn divide(f32 a, f32 b) -> Result<f32, String> {
//          if (b == 0) return Err(String("Divide by zero"));
//          return Ok(a / b);
//      }
//
//      divide(10.0, 0.0).match(
//          Case(val) { std::cout << "Result: " << val; },
//          Case(err) { std::cerr << "Error: " << err; }
//      );
//
//      let res = divide(10.0, 2.0);
//      if (res) {            // operator bool checks is_ok
//          std::cout << *res; // operator* is equivalent to unwrap()
//      }
//
// =============================================================================
// 3. Object Model (Trait / Interface)
// =============================================================================
// Requires: ENABLE_RS_OBJECT
// Macros emulate Rust's trait definitions and impl blocks while keeping data and
// behavior separated.
//
// A. Define a trait (interface)
//    - `trait(Name, ...)`: defines a pure-virtual base class with a virtual
//      destructor.
//    - `must(...)`: declares a pure virtual function (`=0`) that subclasses
//      must implement.
//    - `def(...)`: declares a virtual function with a default implementation.
//
// B. Implement a trait
//    - Inheritance syntax: `class MyClass : from MyTrait`
//    - `impl(...)`: expands to `override` for explicit implementations.
//    - Recommended pattern: `class X : from Trait, data DataStruct { inner: pub: }`
//      to keep state and behavior in distinct bases and make access levels explicit.
//
//    Example
//      trait(Draw,
//          must(draw() -> void);
//          def(area() -> f32) { return 0; }
//      );
//
//      struct CircleData { f32 r; };
//
//      class Circle : from Draw, data CircleData {
//      pub:
//          Circle(f32 r) : CircleData{r} {}
//          impl(draw() -> void) {
//              std::cout << "Circle r=" << r << "\n";
//          }
//      };
//
// -----------------------------------------------------------------------------
#ifndef RUSTIC_H
#define RUSTIC_H

// ==========================================
// Configuration
// ==========================================
#ifndef DO_NOT_ENABLE_ALL_RUSTIC
#define ENABLE_RS_KEYWORD
#define ENABLE_RS_ERROR
#define ENABLE_RS_OBJECT
#endif

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <variant>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <string>
#include <format> // C++20

// ==========================================
// 1. Syntax Sugar (Type aliases + bindings)
// ==========================================
// Requires: ENABLE_RS_KEYWORD
//
// Rust-style aliases to reduce typing while keeping intent clear.
// | Rust style | C++ native type    | Notes |
// |:-----------|:-------------------|:------|
// | i8/u8      | int8_t / uint8_t   |       |
// | i32/u32    | int32_t / uint32_t |       |
// | f32/f64    | float / double     |       |
// | usize      | size_t             |       |
// | String     | std::string        | Alias only, not Rust's memory model |
// | Vec<T>     | std::vector<T>     |       |
//
// Functions and variables:
// - `fn`      -> `auto`       : return type deduction (C++14+).
// - `let`     -> `const auto` : immutable by default.
// - `let_mut` -> `auto`       : mutable binding.
//
// Example:
//   fn add(i32 a, i32 b) -> i32 { return a + b; }
//   let pi = 3.14;
//   let_mut count = 0;

#ifdef ENABLE_RS_KEYWORD
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;
using usize = size_t;
using isize = ssize_t;
using String = std::string;

template<typename T>
using Vec = std::vector<T>;

#define fn auto
#define let const auto
#define let_mut auto
#endif

// ==========================================
// 2. Error Handling
// ==========================================
#ifdef ENABLE_RS_ERROR

// Match helpers (kept with error model so Option/Result users always have them)
// Usage: res.match( Case(v){...}, Case(e){...} )
#define Case(Var) [&](auto&& Var)
#define DefaultCase() [&]()

struct Unit {
    bool operator==(const Unit&) const { return true; }
    bool operator!=(const Unit&) const { return false; }
};

template<typename T> class Option;
template<typename T, typename E> class Result;

inline void rs_panic(const std::string& msg) {
    std::cerr << "[Panic] " << msg << std::endl;
    std::abort(); // Hard abort; intentionally not catchable ("Let it crash")
}
inline void panic(const std::string& msg){
    rs_panic(msg);
}

// --- Option ---
template<typename T>
class Option {
    std::variant<std::monostate, T> value;
public:
    Option() : value(std::monostate{}) {}
    Option(std::monostate) : value(std::monostate{}) {}
    Option(const T& val) : value(val) {}
    Option(T&& val) : value(std::move(val)) {}

    static Option<T> Some(T&& val) { return Option<T>(std::move(val)); }
    static Option<T> Some(const T& val) { return Option<T>(val); }
    static Option<T> None() { return Option<T>(); }

    bool is_some() const { return std::holds_alternative<T>(value); }
    bool is_none() const { return std::holds_alternative<std::monostate>(value); }
    explicit operator bool() const { return is_some(); }

    T& unwrap() {
        if (is_none()) rs_panic("called `Option::unwrap()` on a `None` value");
        return std::get<T>(value);
    }
    const T& unwrap() const {
        if (is_none()) rs_panic("called `Option::unwrap()` on a `None` value");
        return std::get<T>(value);
    }

    T& expect(const char* msg) {
        if (is_none()) rs_panic(msg);
        return std::get<T>(value);
    }

    T unwrap_or(const T& def) const { return is_some() ? std::get<T>(value) : def; }

    // Pointer semantics
    T* operator->() { return &unwrap(); }
    const T* operator->() const { return &unwrap(); }
    T& operator*() { return unwrap(); }
    const T& operator*() const { return unwrap(); }

    // Match Pattern
    template<typename F1, typename F2>
    auto match(F1&& f_some, F2&& f_none) const {
        if (is_some()) {
            if constexpr (std::is_invocable_v<F1, const T&>) {
                return f_some(std::get<T>(value));
            } else {
                return f_some(); // Support Case() without args
            }
        } else {
            return f_none();
        }
    }
};

// --- Result ---
template<typename T> struct OkValue { T value; };
template<typename E> struct ErrValue { E error; };

template<typename T, typename E>
class Result {
    std::variant<T, E> value;
public:
    Result(const T& val) : value(std::in_place_index<0>, val) {}
    Result(T&& val) : value(std::in_place_index<0>, std::move(val)) {}
    Result(const E& err) : value(std::in_place_index<1>, err) {}
    Result(E&& err) : value(std::in_place_index<1>, std::move(err)) {}

    // Implicit Conversion
    template<typename U>
    Result(OkValue<U>&& ok) : value(std::in_place_index<0>, std::move(ok.value)) {}
    template<typename U>
    Result(ErrValue<U>&& err) : value(std::in_place_index<1>, std::move(err.error)) {}

    bool is_ok() const { return value.index() == 0; }
    bool is_err() const { return value.index() == 1; }
    explicit operator bool() const { return is_ok(); }

    T& unwrap() {
        if (is_err()) rs_panic("called `Result::unwrap()` on an `Err` value");
        return std::get<0>(value);
    }
    const T& unwrap() const {
        if (is_err()) rs_panic("called `Result::unwrap()` on an `Err` value");
        return std::get<0>(value);
    }

    T& expect(const char* msg) {
        if (is_err()) rs_panic(msg);
        return std::get<0>(value);
    }

    E& unwrap_err() {
        if (is_ok()) rs_panic("called `Result::unwrap_err()` on an `Ok` value");
        return std::get<1>(value);
    }

    // Pointer semantics
    T* operator->() { return &unwrap(); }
    const T* operator->() const { return &unwrap(); }
    T& operator*() { return unwrap(); }
    const T& operator*() const { return unwrap(); }

    // Match Pattern
    template<typename F1, typename F2>
    auto match(F1&& f_ok, F2&& f_err) const {
        if (is_ok()) {
            return f_ok(std::get<0>(value));
        } else {
            return f_err(std::get<1>(value));
        }
    }
};

// Factories
template<typename T> auto Some(T&& v) { return Option<std::decay_t<T>>(std::forward<T>(v)); }
inline auto None() { return std::monostate{}; }

template<typename T> auto Ok(T&& v) { return OkValue<std::decay_t<T>>{std::forward<T>(v)}; }
inline auto Ok() { return OkValue<Unit>{Unit{}}; }
template<typename E> auto Err(E&& e) { return ErrValue<std::decay_t<E>>{std::forward<E>(e)}; }

#endif

// ==========================================
// 4. Object Model (Trait / Interface)
// ==========================================
#ifdef ENABLE_RS_OBJECT

class Interface {
public:
    virtual ~Interface() = default;
};

// Define Trait
#define trait(Name, ...) \
struct Name : public Interface { \
    pub: \
    __VA_ARGS__ \
};
// Keep interface and data separate; inherit both explicitly.
// Recommended shape: class Foo : from BarTrait, data BarState { inner: pub: };
#define from public
#define data protected

// Access modifiers focused on interface vs implementation.
#define pub public
// inner: shorthand for protected members that stay visible to derived classes.
#define inner protected

// Default Function (Virtual)
#define must(...) virtual auto __VA_ARGS__ =0
#define def(...) virtual auto __VA_ARGS__
// Implementation (Override)
#define impl(...) auto __VA_ARGS__ override


#endif // ENABLE_RS_OBJECT


#endif // RUSTIC_H

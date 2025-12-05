# rustic.hpp

一个为现代 C++ 提供 Rust 风格语义的单头文件库。它带来 Option/Result、match 风格分支以及 trait/impl 宏，减少样板又不引入外部依赖。

## 特性
- 错误模型：`Option` 与 `Result`，具备布尔和指针语义，并提供 `match` 便捷分支。
- 语法糖：`fn`、`let`、`let_mut`、`Case`、`DefaultCase` 以及访问控制别名 (`pub`、`prot` 等)。
- 对象模型：`trait`/`impl` 与 `from`/`data`，将接口与数据分离。
- 纯头文件、零第三方依赖，仅使用 C++17/20 标准库。

## 编译要求
- 推荐 C++20（依赖 `std::format`）。如果项目不需要 format，本库的其余部分可在 C++17 上编译。
- 除了包含头文件，无需额外构建步骤。

## 使用步骤
1. 将 `rustic.hpp` 放入头文件搜索路径。
2. 在需要的源文件中包含：
   ```cpp
   #include "rustic.hpp"
   ```
3. 如需按需启用，先定义：
   - `DO_NOT_ENABLE_ALL_RUSTIC` 关闭默认全量开启。
   - `ENABLE_RS_KEYWORD` 开启语法糖（fn/let/pub、Case/DefaultCase）。
   - `ENABLE_RS_ERROR` 开启 `Option`、`Result`、`panic`。
   - `ENABLE_RS_OBJECT` 开启 trait/impl 以及继承相关宏。

仅启用错误模型的示例：
```cpp
#define DO_NOT_ENABLE_ALL_RUSTIC
#define ENABLE_RS_ERROR
#include "rustic.hpp"
```

## 快速示例
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

## 语法糖
- `fn` 展开为 `auto`（返回类型推导），`let` 展开为 `const auto`，`let_mut` 展开为 `auto`。
- 访问控制别名：`pub`、`priv`、`prot`。
- 模式助手：`Case(x)` 捕获参数，`DefaultCase()` 用于无参分支。

## 错误处理
`Option<T>` 表示可空值，`Result<T, E>` 表示成功或失败。二者提供：
- 布尔语义（`if (opt) { ... }`）。
- 指针语义（`opt->field`、`*res`）会内部调用 `unwrap()`。
- `match` 强制同时处理两种分支。

登录流程示例：
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

## 对象模型
推荐形式 `class Foo : from MyTrait, data FooState`，接口与数据分离。

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

## 提示
- `panic` 会直接中止进程，仅在真正不可恢复的场景使用。
- `match` 返回值需在所有分支保持一致。
- 无返回数据的成功分支可使用 `Ok()`（对应 `Result<Unit, E>`）。
- 建议在调用处包含头文件；若放入预编译头，请确保宏配置一致。

# rustic.hpp

[English](README.md) | 中文说明

一个为现代 C++ 提供 Rust 风格语义的单头文件库。它带来 Option/Result、match 风格分支以及 trait/impl 宏，减少样板又不引入外部依赖。本文档刻意写得很详细，方便你在真实服务、命令行工具或课程作业里落地。

## 目录
- 本库提供什么
- 兼容性与编译说明
- 安装与配置
- 模块详解
  - 语法糖与类型别名（fn, let, i32/u32, Vec）
  - 错误模型（Option, Result, panic, match、unwrap 系列）
  - 对象模型（trait/impl, from/data/inner, pub）
- 使用模式与最佳实践
- 集成示例
- 限制与注意事项
- 本地测试清单

## 本库提供什么
- 错误模型：`Option` 与 `Result`，具备布尔和指针语义，并提供 `match` 辅助（`Case`、`DefaultCase`）。
- 语法糖与别名：`i32/u32`、`f64`、`Vec<T>`、`String` 等类型别名，以及 `fn`、`let`、`let_mut` 等绑定语法。
- 对象模型：`trait`/`impl` 与 `from`/`data`，配合 `pub`/`inner` 划分对外接口与实现细节。
- 纯头文件、零第三方依赖，只依赖 C++17/20 标准库。

## 兼容性与编译说明
- 推荐使用 C++20，因为示例和项目常用 `std::format`。如果不依赖 format，本头文件主体可在 C++17 下编译。
- 无全局初始化；可安全地在多个翻译单元中包含。
- 宏配置按翻译单元生效，保持一致可避免接口差异。

## 安装与配置
1. 将 `rustic.hpp` 放入头文件搜索路径。
2. 在需要的源文件中包含：
   ```cpp
   #include "rustic.hpp"
   ```
3. 可选宏（需在包含前定义）：
   - `DO_NOT_ENABLE_ALL_RUSTIC` 关闭默认全量开启。
   - `ENABLE_RS_KEYWORD` 开启类型别名与绑定语法糖（i32/u32、Vec、fn/let/let_mut）。
   - `ENABLE_RS_ERROR` 开启 `Option`、`Result`、`panic` 与 `Case/DefaultCase`。
   - `ENABLE_RS_OBJECT` 开启 trait/impl、继承与访问控制宏（含 `pub`/`inner`）。

仅启用错误模型的示例：
```cpp
#define DO_NOT_ENABLE_ALL_RUSTIC
#define ENABLE_RS_ERROR
#include "rustic.hpp"
```

## 模块详解

### 语法糖与类型别名（ENABLE_RS_KEYWORD）
- 类型别名：`i8/i16/i32/i64`，`u8/u16/u32/u64`，`f32`，`f64`，`usize`，`isize`，`String`（`std::string` 的别名），`Vec<T>`（`std::vector<T>` 的别名）。
- 绑定语法糖：`fn` 展开为 `auto`，`let` 展开为 `const auto`，`let_mut` 展开为 `auto`。
- 访问控制宏放在对象模型部分（见下文）。

用于减少样板并保持可读性。它们是宏，避免在相同作用域内使用同名标识符。

### 错误模型（ENABLE_RS_ERROR）
`Option<T>` 与 `Result<T, E>` 强制显式处理，可与模式分支组合。

`Option<T>` 关键操作：
- 构造：`Some(value)`，`None()`。
- 查询：`is_some()`，`is_none()`，布尔转换（`if (opt) { ... }`）。
- 访问：
  - `unwrap()`：有值返回引用，无值触发 `panic`。
  - `expect(msg)`：同 `unwrap()`，但带自定义报错信息。
  - `unwrap_or(default)`：无值时返回提供的默认副本。
- 指针语义：`opt->method()` 与 `*opt` 内部调用 `unwrap()`。
- 匹配：`opt.match(Case(v){...}, DefaultCase(){...});` 使用错误模型提供的 `Case`/`DefaultCase` 辅助，分支可有无参数。

`Result<T, E>` 关键操作：
- 构造：`Ok(value)`，`Err(error)`，无返回数据时可用 `Ok()`（`Result<Unit, E>`）。
- 查询：`is_ok()`，`is_err()`，布尔转换。
- 访问：`unwrap()`，`unwrap_err()`，`expect(msg)`。
- 指针语义与 `Option` 相同。
- 匹配：`res.match(Case(val){...}, Case(err){...});` 返回值类型需一致。

panic：
- `panic(msg)` 和内部的 `rs_panic` 会直接终止进程，仅在不可恢复状态使用。

unwrap 系列：选择何时使用
- 分支处理和记录日志时优先 `match`，再返回 `Result` 或 `Option` 给上层。
- 只有在逻辑保证“不可能为空”的情况下才用 `unwrap()`。
- 调试期需要更清晰报错时使用 `expect(msg)`。
- 在边界层、且有廉价可复制的兜底值时用 `unwrap_or(default)`。

### 对象模型（ENABLE_RS_OBJECT）
模拟 Rust trait 的宏：
- `trait(Name, ...)` 定义带虚析构的纯虚基类。
- `must(...)` 声明必须实现的纯虚函数。
- `def(...)` 声明带默认实现的虚函数，也可以在 trait 内新增方法并给出默认实现；本质是对 `virtual` 的简明包装。
- `impl(...)` 在实现处展开为 `override`。
- `from`、`data` 是 public/protected 继承别名。推荐形式：`class Foo : from BarTrait, data BarState { inner: pub: };` 既分离接口与数据，也让访问级别明确。
- `inner` 是用于实现细节的 protected 简写，`pub` 映射到 `public` 作为对外接口。

示例：
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

## 使用模式与最佳实践
- 优先使用 `match` 而非层层 `if`，让成功与失败分支都显式存在。
- 在库代码中，除非失败不可能，否则避免 `unwrap()`，返回 `Result` 或 `Option` 交由调用方决定。
- 有明确退路时在边界使用 `unwrap_or`。
- 结合布尔与指针语义写简洁代码：
  ```cpp
  if (auto user = get_user(id)) {
      std::cout << user->name;
  }
  ```
- 使用 `data` 基类存放状态，避免接口继承链携带成员数据。
- 只需部分功能时，用宏开关裁剪，减少命名空间污染。
- `match` 若需要返回值，所有分支必须返回同一类型；若仅做副作用则不返回。

## 集成示例

### 登录流程的显式错误
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

### 使用 Option 进行安全数字解析
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

### 基于 trait 的渲染
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

### 带返回值的 match 分支
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

## 限制与注意事项
- 宏在每个翻译单元独立生效，保持配置一致以免接口差异。
- `panic` 会直接终止进程，没有恢复路径。
- `std::format` 需要 C++20；在 C++17 下请避免依赖 format 或自行提供替代。
- 作为头文件库，修改后需要重新编译包含它的翻译单元。

## 本地测试清单
- 用日常编译选项构建一个最小示例，确认宏可见性符合预期：
  ```bash
  g++ -std=c++20 -Wall -Wextra -pedantic sample.cpp -o sample
  ```
- 为返回 `Result` 或 `Option` 的接口添加单元测试，覆盖成功与失败分支。
- 若依赖 trait 宏，测试多个派生类，确保 `impl(...)` 正确覆盖。

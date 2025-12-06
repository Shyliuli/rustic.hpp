#include "rustic.hpp"
#include <iostream>

// Demonstrates keywords/aliases, Option/Result with match, and trait/impl usage.

trait(Renderable,
    must(draw() -> void);
    def(area() -> f32) { return 0; }
);

struct RectData { f32 w; f32 h; };

class Rect : from Renderable, datafrom RectData {
inner:
    // inner is protected; derived classes can access data.
public:
    Rect(f32 w, f32 h) : RectData{w, h} {}
    impl(draw() -> void) {
        std::cout << "Rect " << w << " x " << h << "\n";
    }
    impl(area() -> f32) {
        return w * h;
    }
};

fn divide(i32 a, i32 b) -> Result<f32, String> {
    if (b == 0) return Err(String("divide by zero"));
    return Ok(static_cast<f32>(a) / static_cast<f32>(b));
}

fn find_user(Vec<String> names, String target) -> Option<usize> {
    for (usize i = 0; i < names.size(); ++i) {
        if (names[i] == target) return Some(i);
    }
    return None();
}

fn main()->int {
    // Aliases and bindings
    let width = static_cast<f32>(3);
    let_mut height = static_cast<f32>(4);
    Rect rect(width, height);
    rect.draw();
    std::cout << "Area: " << rect.area() << "\n";

    // Option with match
    Vec<String> users = {"alice", "bob", "carol"};
    find_user(users, "bob").match(
        Case(idx){ std::cout << "Found at index " << idx << "\n"; },
        DefaultCase(){ std::cout << "User not found\n"; }
    );

    // Result with match
    divide(10, 2).match(
        Case(val){ std::cout << "10 / 2 = " << val << "\n"; },
        Case(err){ std::cout << "Error: " << err << "\n"; }
    );
    divide(1, 0).match(
        Case(val){ std::cout << val << "\n"; },
        Case(err){ std::cout << "Expected error: " << err << "\n"; }
    );

    return 0;
}

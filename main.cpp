#include <optional>

#include "magic_builder.hpp"


struct myChecker {
    enum class Action { set_a, set_b, set_c };

    const bool a_is_set = false;
    const bool b_is_set = false;
    const bool c_is_set = false;

    consteval myChecker state_after(Action action) const {
        switch (action) {
            case Action::set_a:
                return change_fields(true, std::nullopt, std::nullopt);
            case Action::set_b:
                return change_fields(std::nullopt, true, std::nullopt);
            case Action::set_c: 
                return change_fields(std::nullopt, std::nullopt, true);
        }
    }

    consteval myChecker change_fields(std::optional<decltype(a_is_set)> a,
                                     std::optional<decltype(b_is_set)> b,
                                     std::optional<decltype(c_is_set)> c) const {
        return {
            a.value_or(a_is_set), b.value_or(b_is_set), c.value_or(c_is_set)
        };
    }

    consteval bool is_allowed(Action action) {
        switch (action) {
            case Action::set_a:
                return !a_is_set;
            
            case Action::set_b:
                return !b_is_set;
            
            case Action::set_c:
                return !c_is_set;
        }
    }
};
static_assert(magic_bldr::Checker<myChecker>);

int main() {}
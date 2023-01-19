#include <optional>

#include "magic_builder.hpp"

using namespace magic_bldr;

struct myChecker {
    enum class Action { set_a, set_b, set_c, set_a_and_b };
    template <Action A>
    struct ActionArgs {};
    template <>
    struct ActionArgs<Action::set_a> {
        using ArgType = int;
        consteval static auto state_after(const myChecker& checker) {
            return checker.change_fields(true, std::nullopt, std::nullopt);
        }
        consteval static auto is_allowed(const myChecker& checker) {
            return !checker.a_is_set;
        }
    };
    template <>
    struct ActionArgs<Action::set_b> {
        using ArgType = int;
        consteval static myChecker state_after(const myChecker& checker) {
            return checker.change_fields(std::nullopt, true, std::nullopt);
        }
        consteval static bool is_allowed(const myChecker& checker) {
            return !checker.b_is_set;
        }
    };
    template <>
    struct ActionArgs<Action::set_c> {
        using ArgType = int;
        consteval static myChecker state_after(const myChecker& checker) {
            return checker.change_fields(std::nullopt, std::nullopt, true);
        }
        consteval static bool is_allowed(const myChecker& checker) {
            return !checker.c_is_set;
        }
    };
    template <>
    struct ActionArgs<Action::set_a_and_b> {
        using ArgType = std::tuple<int, int>;
        consteval static myChecker state_after(const myChecker& checker) {
            return checker.change_fields(true, true, std::nullopt);
        }
        consteval static bool is_allowed(const myChecker& checker) {
            return !checker.a_is_set && !checker.b_is_set;
        }
    };

    const bool a_is_set = false;
    const bool b_is_set = false;
    const bool c_is_set = false;

    template <Action A>
    consteval myChecker state_after() const {
        return ActionArgs<A>::state_after(*this);
    }

    consteval myChecker change_fields(
        std::optional<decltype(a_is_set)> a,
        std::optional<decltype(b_is_set)> b,
        std::optional<decltype(c_is_set)> c) const {
        return {a.value_or(a_is_set), b.value_or(b_is_set),
                c.value_or(c_is_set)};
    }

    template <Action A>
    consteval bool is_allowed() {
        return ActionArgs<A>::is_allowed(*this);
    }
};
// static_assert(Checker<myChecker>);

struct myBuildable {
    int a;
    int b;
    int c;
};

class myBuilder : Builder<myBuilder, myBuildable, myChecker, myBuildable> {
   public:
    template <Action A, typename Arg>
    consteval void action_to_run(Arg arg);

    template <>
    consteval void action_to_run<Action::set_a>(int arg) {
        builder_args.a = arg;
    }
};

int main() { myBuilder().action_to_run<myChecker::Action::set_a>(5); }
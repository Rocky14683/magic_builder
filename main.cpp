#include <optional>
#include <type_traits>

#include "magic_builder.hpp"
#include <type_traits>

using namespace magic_bldr;

struct myBuildable {
    int a;
    int b;
    int c;
};

using myBuildData = myBuildable;

enum class myActions { set_a, set_b, set_c, set_a_and_b };

struct myChecker : Checker<myChecker, myBuildable, myActions, myBuildData> {
    const bool a_is_set = false;
    const bool b_is_set = false;
    const bool c_is_set = false;

    consteval myChecker change_fields(
        std::optional<decltype(a_is_set)> a,
        std::optional<decltype(b_is_set)> b,
        std::optional<decltype(c_is_set)> c) const {
        return {a.value_or(a_is_set), b.value_or(b_is_set),
                c.value_or(c_is_set)};
    }

    template <Action A>
    myChecker state_after() const {
        return ActionDetail<A>::state_after(*this);
    }

    template <Action A>
    consteval bool is_allowed() const {
        return ActionDetail<A>::is_allowed(*this);
    }

    consteval bool ready() const {
        return a_is_set && b_is_set && c_is_set;
    }
};

template <>
struct myChecker::ActionDetail<myActions::set_a> {
    using ArgType = int;

    consteval static auto state_after(const myChecker& checker) {
        return checker.change_fields(true, std::nullopt, std::nullopt);
    }

    consteval static auto is_allowed(const myChecker& checker) {
        return !checker.a_is_set;
    }

    static void run(BuildData& b, ArgType arg) {
        b.a = arg;
    }
};

template <>
struct myChecker::ActionDetail<myActions::set_b> {
    using ArgType = int;
    
    consteval static myChecker state_after(const myChecker& checker) {
        return checker.change_fields(std::nullopt, true, std::nullopt);
    }

    consteval static bool is_allowed(const myChecker& checker) {
        return !checker.b_is_set;
    }

    static void run(BuildData& b, ArgType arg) {
        b.b = arg;
    }
};

template <>
struct myChecker::ActionDetail<myActions::set_c> {
    using ArgType = int;

    consteval static myChecker state_after(const myChecker& checker) {
        return checker.change_fields(std::nullopt, std::nullopt, true);
    }

    consteval static bool is_allowed(const myChecker& checker) {
        return !checker.c_is_set;
    }

    static void run(BuildData& b, ArgType arg) {
        b.c = arg;
    }
};

template <>
struct myChecker::ActionDetail<myActions::set_a_and_b> {
    using ArgType = std::tuple<int, int>;

    consteval static myChecker state_after(const myChecker& checker) {
        return checker.change_fields(true, true, std::nullopt);
    }

    consteval static bool is_allowed(const myChecker& checker) {
        return !checker.a_is_set && !checker.b_is_set;
    }

    static void run(BuildData& b, const ArgType& arg) {
        std::tie(b.a, b.b) = arg;
    }
};

// static_assert(Checker<myChecker>);
// static_assert(ActionArgLike<myChecker::ActionDetail<myChecker::Action::set_a>, myChecker>);


class myBuilder : Builder<myBuilder, myChecker> {
   public:
    template <myActions A, typename Arg>
    consteval void action_to_run(Arg arg);

    template <>
    consteval void action_to_run<myActions::set_a>(int arg) {
        builder_args.a = arg;
    }
};

int main() { myBuilder().action_to_run<myChecker::Action::set_a>(5); }
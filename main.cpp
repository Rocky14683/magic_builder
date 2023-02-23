#include <optional>
#include <type_traits>

#include "magic_builder.hpp"

using namespace magic_bldr;

struct myBuildable {
    int a;
    int b;
    int c;
};

using myBuildData = myBuildable;

struct myChecker : Checker<myChecker> {
    bool a_is_set = false;
    bool b_is_set = false;
    bool c_is_set = false;

    // ctors necessary because first entry of aggregate initialization is the 
    // fucking base class. ugh
    myChecker(bool a, bool b, bool c) : a_is_set(a), b_is_set(b), c_is_set(c) {}
    myChecker() = default;

    consteval bool ready() const { return a_is_set && b_is_set && c_is_set; }
};

template<typename Checker>
struct checker_traits<myChecker> {
    enum class Actions { set_a, set_b, set_c, set_a_and_b };
    using Buildable = myBuildable;
    using BuildData = myBuildData;
};

struct myChecker::ActionDetail {
    using ArgType = int;

    consteval static myChecker state_after(myChecker c) {
        c.a_is_set = true;
        return c;
    }

    consteval static auto is_allowed(const myChecker& checker) {
        return !checker.a_is_set;
    }

    static void run(BuildData& b, ArgType arg) { b.a = arg; }
};

template <>
struct myChecker::ActionDetail<myActions::set_b> {
    using ArgType = int;

    consteval static myChecker state_after(myChecker c) {
        c.b_is_set = true;
        return c;
        // return {c.a_is_set, true, c.c_is_set};
        // return checker.change_fields(std::nullopt, true, std::nullopt);
    }

    consteval static bool is_allowed(const myChecker& checker) {
        return !checker.b_is_set;
    }

    static void run(BuildData& b, ArgType arg) { b.b = arg; }
};

template <>
struct myChecker::ActionDetail<myActions::set_c> {
    using ArgType = int;

    consteval static myChecker state_after(myChecker c) {
        // return {c.a_is_set, c.b_is_set, true};
        c.c_is_set = true;
        return c;
    }

    consteval static bool is_allowed(const myChecker& checker) {
        return !checker.c_is_set;
    }

    static void run(BuildData& b, ArgType arg) { b.c = arg; }
};

template <>
struct myChecker::ActionDetail<myActions::set_a_and_b> {
    using ArgType = std::tuple<int, int>;

    consteval static myChecker state_after(myChecker c) {
        // return {true, true, c.c_is_set};
        c.a_is_set = true;
        c.b_is_set = true;
        return c;
    }

    consteval static bool is_allowed(const myChecker& checker) {
        return !checker.a_is_set && !checker.b_is_set;
    }

    static void run(BuildData& b, const ArgType& arg) {
        std::tie(b.a, b.b) = arg;
    }
};

// static_assert(Checker<myChecker>);
// static_assert(ActionArgLike<myChecker::ActionDetail<myChecker::Action::set_a>,
// myChecker>);

class myBuilder : Builder<myBuilder, myChecker> {
   public:
    template <myActions A, typename Arg>
    consteval void action_to_run(Arg arg);

    template <>
    consteval void action_to_run<myActions::set_a>(int arg) {
        builder_args.a = arg;
    }
};

template <myChecker C>
struct A {};

int main() {
    // myBuilder().action_to_run<myChecker::Action::set_a>(5);
}
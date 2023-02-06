#include <array>
#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>

namespace magic_bldr {
// clang-format off
// template <typename T, typename Action = typename T::Action>
// concept Checker = requires(T checker, Action action) {
//     // T::Action is an enum
//     requires std::is_enum_v<Action>;

//     // Check that T::T() can be constant-evaluated
//     // Pretty ugly idiom but it seems to be the best way to check
//     // Explanation: https://stackoverflow.com/a/69515693
//     {
//         std::bool_constant<(T(), true)>()
//     } -> std::same_as<std::true_type>;  

//     // T::state_after returns T and is constant-evaluable
//     // { checker.template state_after<Action()>() } -> std::same_as<T>;
//     checker.state_after();
//     {
//         std::bool_constant<(T().state_after(), true)>()
//     } -> std::same_as<std::true_type>;

//     // T::is_allowed returns bool and is constant-evaluable
//     { checker.is_allowed() } -> std::same_as<bool>;
//     {
//         std::bool_constant<(T().is_allowed(), true)>()
//     } -> std::same_as<std::true_type>;
// };

// template <typename AA, typename C>
// concept ActionArgLike = requires(C c) {
//     // AA has an ArgType defined
//     typename AA::ArgType;

//     // AA::state_after is constant evaluable and returns C
//     { AA::state_after(c) } -> std::same_as<C>;
//     { // Explanation: https://stackoverflow.com/a/69515693
//         std::bool_constant<(AA::state_after(C()), true)>()
//     } -> std::same_as<std::true_type>;

//     // AA::is_allowed is constant evaluable and returns bool
//     { AA::is_allowed(c) } -> std::same_as<bool>;
//     { // Explanation: https://stackoverflow.com/a/69515693
//         std::bool_constant<(AA::is_allowed(C()), true)>()
//     } -> std::same_as<std::true_type>;
// };
// clang-format on


// template <Checker C, typename A = typename C::Actions>
// consteval auto operator+(const C& checker, const A& action) -> C {
//     return checker.state_after_action(action);
// }

template <typename Derived, typename Buildable_, typename Action_, typename BuildData_>
class Checker {
  public:
    using Action = Action_;
    using BuildData = BuildData_;
    using Buildable = Buildable_;

    template <Action A>
    struct ActionDetail {};
};


template <typename Derived, typename Checker_>
class Builder {
   protected:
    using Checker = Checker_;
    using BuildData = typename Checker::BuildData;
    using Buildable = typename Checker::Buildable;
    using Action = typename Checker::Action;

    typename Checker_::BuildData builder_args;

    template <Action A, typename Arg>
    consteval void action_to_run(Arg arg) {
        static_cast<Derived*>(this)->action_to_run<A>(arg);
    }

   private:
    template <Checker_ C = Checker_()>
    class Worker {};

   public:
    constexpr Builder() {}

    template <Action A, typename... Args>
    consteval auto set(Args... args) {
        return action_to_run<A>(std::make_tuple(args...));
    }
};

}  // namespace magic_bldr
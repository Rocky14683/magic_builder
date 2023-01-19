#include <array>
#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>

namespace magic_bldr {

// clang-format off
template <typename T, typename Action = typename T::Action>
concept Checker = requires(T checker, Action action) {
    // T::Action is an enum
    requires std::is_enum_v<Action>;

    // Check that T::T() can be constant-evaluated
    // Pretty ugly idiom but it seems to be the best way to check
    // Explanation: https://stackoverflow.com/a/69515693
    {
        std::bool_constant<(T(), true)>()
    } -> std::same_as<std::true_type>;  

    // T::state_after returns T and is constant-evaluable
    { checker.state_after(action) } -> std::same_as<T>;
    {
        std::bool_constant<(T().state_after(Action()), true)>()
    } -> std::same_as<std::true_type>;

    // T::is_allowed returns bool and is constant-evaluable
    { checker.is_allowed(action) } -> std::same_as<bool>;
    {
        std::bool_constant<(T().is_allowed(Action()), true)>()
    } -> std::same_as<std::true_type>; 
};
// clang-format on

template <Checker C, typename A = typename C::Action>
consteval auto operator+(const C& checker, const A& action) -> C {
    return checker.state_after_action(action);
} 

template <typename Buildable, Checker Checker, typename Args_t>
class Builder {
    using Action = typename Checker::Actions;

    template <Checker C = Checker()>
    class Worker {};

    template <Action A>
    consteval void action_to_run();

   public:
    Builder() {}
};

}  // namespace magic_bldr
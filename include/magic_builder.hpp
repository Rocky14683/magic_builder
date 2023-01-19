#include <array>
#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>

namespace magic_bldr {

// clang-format off
template <typename T, typename Action = typename T::Actions>
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

template <Checker C, typename A = typename C::Actions>
consteval auto operator+(const C& checker, const A& action) -> C {
    return checker.state_after_action(action);
} 

template <typename Derived, typename Buildable, Checker Checker, typename BuilderArgs>
class Builder {
  protected:
    using base_builder = Builder<Derived, Buildable, Checker, BuilderArgs>;
    using Action = typename Checker::Actions;

    BuilderArgs builder_args;
    
    template <Action A, typename Arg>
    consteval void action_to_run(Arg arg) {
        static_cast<Derived*>(this)->action_to_run<A>(arg);
    }

  private:  
    template <Checker C = Checker()>
    class Worker {};

   public:
    constexpr Builder() {}

    template<Action A, typename... Args>
    consteval auto set(Args... args) {
        action_to_run<A>(std::make_tuple(args...));
    } 
};

}  // namespace magic_bldr
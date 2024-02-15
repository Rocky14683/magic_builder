#ifndef MAGIC_BUILDER_HPP
#define MAGIC_BUILDER_HPP

#include <functional>
#include <type_traits>
#include <utility>

namespace magic_bldr {
namespace detail {
    template <typename T, typename V>
    concept ActionOf = std::same_as<T, typename V::Action>;
}

template <std::integral auto C>
std::integral_constant<decltype(C), C> c_;

template <typename Verifier, typename Verifier::Action A, Verifier V>
bool action_is_valid();

template <auto A, typename BuilderData, typename... Args>
auto run_action(BuilderData& data, Args&&... args);

template <typename Derived>
struct Verifier {
    template <typename... Ts>
    struct MultiSetField;

    template <typename T>
    struct SetField {
        T Derived::* field;
        T state;

        operator Derived() {
            Derived ret;
            ret.*field = state;
            return ret;
        }

        template <typename U>
        auto operator|(SetField<U> sf) -> MultiSetField<T, U> {
            return {.fields = {field, sf.field},
                    .states = {state, sf.state}};
        }
    };

    template <typename... Ts>
    struct MultiSetField {
        std::tuple<Ts Derived::* ...> fields;
        std::tuple<Ts...> states;

        operator Derived() {
            Derived ret;
            auto field_lvalues = std::apply(
                [&](Ts Derived::*... ts) -> std::tuple<Ts&...> {
                    return {(ret.*ts)...};
            }, fields);
            field_lvalues = states;
            return ret;
        }

        template <typename U>
        auto operator|(SetField<U> sf) -> MultiSetField<Ts..., U> {
            return {.fields = std::tuple_cat(fields, {sf.field}),
                    .states = std::tuple_cat(states, {sf.state})};
        }

        template <typename... Us>
        auto operator|(MultiSetField<Us...> msf) -> MultiSetField<Ts..., Us...> {
            return {.fields = std::tuple_cat(fields, msf.fields),
                    .states = std::tuple_cat(states, msf.states)};
        }
    };
    
    consteval auto operator+(detail::ActionOf<Derived> auto rhs) {
        return static_cast<Derived*>(this)->state_after(rhs);
    }

    template <typename T>
    consteval SetField<T> set(T Derived::* field, std::remove_const_t<T> state) {
        return {field, state};
    }
};

template <typename Buildable, typename Verifier_, typename BuilderData, Verifier_ V>
class Builder {
    template <Verifier_ V_>
    using ThisBuilder = Builder<Buildable, Verifier_, BuilderData, V_>;

    BuilderData data;

  protected:
    using Action = typename Verifier_::Action;

    template <Action A, typename... Args>
    requires requires (Args... args) {
        run_action<A>(data, args...);
        requires action_is_valid<Verifier_, A, V>();
    }
    constexpr auto run(Args&&... args) {
        run_action<A>(data, std::forward<Args>(args)...);
        return ThisBuilder<V + A>{std::move(data)};
    }
};
}
#endif
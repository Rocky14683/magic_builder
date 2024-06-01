#ifndef MAGIC_BUILDER_HPP
#define MAGIC_BUILDER_HPP

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <set>

namespace magic_bldr {

namespace detail {
template <typename A, typename V>
concept IsActionOfValidator = std::same_as<A, typename V::Action>;

template <typename V, typename A>
concept IsValidatorOfAction = std::same_as<A, typename V::Action>;

}

template <size_t N>
struct TemplateStringLiteral {
    char value[N];

    constexpr TemplateStringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
};

template <typename Derived>
class Validator {
    template <typename... Ts>
    struct MultiSetField;

    template <typename T>
    struct SetField {
        Derived prev_state;
        T Derived::* field;
        T state;

        consteval operator Derived() {
            Derived ret = prev_state;
            ret.*field = state;
            return ret;
        }

        template <typename U>
        consteval auto operator+(SetField<U> sf) -> MultiSetField<T, U> {
            return {.prev_state = prev_state,
                    .fields = {field, sf.field},
                    .states = {state, sf.state}};
        }
    };

    template <typename... Ts>
    struct MultiSetField {
        Derived prev_state;
        std::tuple<Ts Derived::* ...> fields;
        std::tuple<Ts...> states;

        consteval operator Derived() {
            Derived ret = prev_state;
            auto field_lvalues = std::apply(
                [&](Ts Derived::*... ts) -> std::tuple<Ts&...> {
                    return {(ret.*ts)...};
            }, fields);
            field_lvalues = states;
            return ret;
        }

        template <typename U>
        consteval auto operator+(SetField<U> sf) -> MultiSetField<Ts..., U> {
            return {.prev_state = prev_state,
                    .fields = std::tuple_cat(fields, {sf.field}),
                    .states = std::tuple_cat(states, {sf.state})};
        }

        template <typename... Us>
        consteval auto operator+(MultiSetField<Us...> msf) -> MultiSetField<Ts..., Us...> {
            return {.prev_state = prev_state,
                    .fields = std::tuple_cat(fields, msf.fields),
                    .states = std::tuple_cat(states, msf.states)};
        }
    };

  public:
    consteval auto operator+(detail::IsActionOfValidator<Derived> auto rhs) const {
        return static_cast<const Derived*>(this)->state_after(rhs);
    }

  protected:
    template <typename T>
    consteval SetField<T> set(T Derived::* field, T state) const {
        return {.prev_state = *static_cast<const Derived*>(this),
                .field = field,
                .state = state};
    }
};

template <auto A>
struct ActionImpl {
    static constexpr void run(auto builder_data, auto... args) = delete;

    static consteval auto is_allowed(detail::IsValidatorOfAction<decltype(A)> auto) = delete;
};


template <typename Buildable, typename Validator_, typename BuilderData, template <Validator_> typename Derived, Validator_ V>
class Builder {
    template <typename, typename Vdtr, typename, template <Vdtr> typename, Vdtr>
    friend class Builder;

    BuilderData data;

    Builder(BuilderData&& data) : data{std::move(data)} {}

  protected:
    using Action = typename Validator_::Action;

    template <Action A, typename... Args>
    requires (ActionImpl<A>::is_allowed(V))
    // TODO: exapnd requires-clause here
    constexpr auto run(Args&&... args) {
        ActionImpl<A>::run(&data, std::forward<Args>(args)...);
        return Derived<V + A>{std::move(data)};
    }

  public:
    Builder() = default;

    [[nodiscard]] Buildable build() &&
    requires (V.ready()) {
        return Buildable{std::move(data)};
    }
};

inline namespace utils {

constexpr bool is_in(auto a, std::initializer_list<decltype(a)> list) {
    return std::find(list.begin(), list.end(), a) != list.end();
}

}
}
#endif
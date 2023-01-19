#include <algorithm>
#include <functional>
#include <stdexcept>
#include <vector>

namespace magic_bldr {
template <typename Key, typename T>
class constexpr_map {
  public:  
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<key_type, mapped_type>;
  private:  
    using data_type = std::vector<value_type>;
  public:  
    using size_type = typename data_type::size_type;
    using iterator = typename data_type::iterator;
  
  private:
    const data_type data;

  public:
    constexpr bool contains(const Key& key) const {
        const auto iter =
            std::find_if(data.begin(), data.end(),
                         [&](value_type i) { return i.first == key; });

        return iter != data.end();
    }

    constexpr const mapped_type& at(const Key& key) const {
        const auto iter = std::find_if(data.begin(), data.end(),
                         [&](value_type i) { return i.first == key; });
        if (iter != data.end()) {
            return *iter;
        } else {
            throw std::out_of_range("Constexpr Map key not found");
        }
    }

    constexpr constexpr_map(std::initializer_list<value_type> data) : data{data} {}
    constexpr constexpr_map(const std::vector<value_type>& data) : data{data} {}
    constexpr constexpr_map(std::vector<value_type>&& data) : data{std::move(data)} {}
};
}  // namespace magic_bldr
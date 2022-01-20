#pragma once
#include <functional>

using FeatureRegistry = std::unordered_map<std::string, > entries_;

class FeatureRegistry {
 private:
 public:
  FeatureRegistry &add(std::string const &name);
  void set(std::string const &name, std::string const &value);
  bool has(std::string const &name) const;
  std::string const &get(std::string const &name) const;
};

template <typename T, typename Rest...>
class FeatureFlags : FeatureFlags<Rest...> {
  FeatureFlags() { T::RegisterFeatureFlags(registry_); }
};

template <>
class FeatureFlags {
 public:
  void set(std::string const &name,
           std::string const &value){registry_.set()} std::unordered_set<std::string> available() const {
    std::unordered_set<std::string> keys;
    for (auto &&entry : registry_) keys.emplace(entry.first);
    return keys;
  }

 protected:
  std::unordered_map<std::string, std::optional<std::string>> entries_;
};
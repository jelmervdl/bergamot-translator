/*
 * Bindings for ResponseOptions class
 *
 */

#include <emscripten/bind.h>

#include "response_options.h"

using ResponseOptions = marian::bergamot::ResponseOptions;
using HTMLOptions = marian::bergamot::HTML::Options;

using namespace emscripten;

void split(std::string const &str, char delimiter, std::unordered_set<std::string> &out) {
  std::string::size_type pos{0}, offset{0};
  while ((pos = str.find(delimiter, offset)) != std::string::npos) {
    if (pos > offset) out.emplace(str.substr(offset, pos - offset));
    offset = pos + 1;
  }
  if (offset != str.size()) out.emplace(str.substr(offset));
}

std::string join(std::unordered_set<std::string> const &items, char delimiter) {
  std::stringstream out;
  bool first = true;
  for (auto &&item : items) {
    if (first)
      first = false;
    else
      out << delimiter;
    out << item;
  }
  return out.str();
}

void setAdd(std::unordered_set<std::string> &set, std::string const &val) { set.emplace(val); }
void setClear(std::unordered_set<std::string> &set) { set.clear(); }
void setDelete(std::unordered_set<std::string> &set, std::string const &val) { set.erase(val); }
bool setHas(std::unordered_set<std::string> &set, std::string const &val) { return set.find(val) != set.end(); }
std::string setToString(std::unordered_set<std::string> &set) { return join(set, ','); };
std::unordered_set<std::string> setFromString(std::string const str) {
  std::unordered_set<std::string> set;
  split(str, ',', set);
  return set;
}

template <typename Retval, typename T>
auto make_ref(Retval T::*prop) {
  return [=](T &obj) -> Retval & { return obj.*prop; };
}

template <typename Retval, typename T, typename... Args>
auto make_ref(Retval T::*prop, Args... args) {
  auto step = make_ref(args...);
  return [=](T & obj) -> auto & { return step(obj.*prop); };
}

template <typename Retval, typename T, typename... Args>
auto make_getter(Retval T::*prop, Args... args) {
  auto ref = make_ref(prop, args...);
  return [=](T const &obj) -> auto const & { return ref(const_cast<T &>(obj)); };
}

// Binding code
EMSCRIPTEN_BINDINGS(response_options) {
  class_<std::unordered_set<std::string>>("StringSet")
      .constructor<>()
      .constructor(&setFromString)
      .function("add", &setAdd)
      .function("clear", &setClear)
      .function("delete", &setDelete)
      .function("has", &setHas)
      .function("toString", &setToString);

  class_<ResponseOptions>("ResponseOptions")
      .constructor<>()
      .property("qualityScores", &ResponseOptions::qualityScores)
      .property("alignment", &ResponseOptions::alignment)
      .property("html", &ResponseOptions::HTML)
      .property("voidTags", make_getter(&ResponseOptions::HTML, &HTMLOptions::voidTags))
      .property("inlineTags", make_getter(&ResponseOptions::HTML, &HTMLOptions::inlineTags))
      .property("continuationDelimiters", make_getter(&ResponseOptions::HTML, &HTMLOptions::continuationDelimiters))
      .property("substituteInlineTagsWithSpaces",
                make_getter(&ResponseOptions::HTML, &HTMLOptions::substituteInlineTagsWithSpaces));
}

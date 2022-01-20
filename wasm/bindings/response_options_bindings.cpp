/*
 * Bindings for ResponseOptions class
 *
 */

#include <emscripten/bind.h>
#include <emscripten/val.h>

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

void setFeatureFlag(ResponseOptions &options, std::string const &name, val const &value) {
  if (name == "inlineTags") {
    options.HTMLOptions.inlineTags.clear();
    split(value.as<std::string>(), ',', options.HTMLOptions.inlineTags);
  } else if (name == "continuationDelimiters") {
    options.HTMLOptions.continuationDelimiters = value.as<std::string>();
  } else if (name == "substituteInlineTagsWithSpaces") {
    options.HTMLOptions.substituteInlineTagsWithSpaces = value.as<bool>();
  }
}

val getFeatureFlags(ResponseOptions const &options) {
  val obj = val::object();
  for (auto &&entry : DefaultFeatureMap::defaults) obj.set(entry.first, val(entry.second));
  return obj;
}

// Binding code
EMSCRIPTEN_BINDINGS(response_options) {
  class_<ResponseOptions>("ResponseOptions")
      .constructor<>()
      .property("qualityScores", &ResponseOptions::qualityScores)
      .property("alignment", &ResponseOptions::alignment)
      .property("html", &ResponseOptions::HTML)
      .function("setFeatureFlag", &setFeatureFlag)
      .function("getFeatureFlags", &getFeatureFlags);
}

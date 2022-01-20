#ifndef SRC_BERGAMOT_RESPONSE_OPTIONS_H_
#define SRC_BERGAMOT_RESPONSE_OPTIONS_H_
#include <string>

#include "definitions.h"
#include "html.h"

namespace marian {
namespace bergamot {

enum ConcatStrategy {
  /// Target text is constructed faithful to the source-text  structure.
  FAITHFUL,

  /// Target text is concatenated by a space.
  SPACE
};

/// ResponseOptions dictate how to construct a Response for an input string of
/// text to be translated.
struct ResponseOptions {
  bool qualityScores{false};  ///< Include quality-scores or not.
  bool alignment{false};      ///< Include alignments or not.

  bool HTML{false};  /// Remove HTML tags from text and insert in output.

  /// Whether to include sentenceMappings or not. Alignments require
  /// sentenceMappings and are available irrespective of this option if
  /// `alignment=true`.
  bool sentenceMappings{false};

  ConcatStrategy concatStrategy{ConcatStrategy::FAITHFUL};

  FeatureMap flags;

  template <typename T>
  int setFeatureFlag(std::string const &name, T const &value) {
    auto entry = DefaultFeatureMap::defaults.find(name);
    if (entry == DefaultFeatureMap::defaults.end()) return 1;  // wrong name
    if (!std::holds_alternative<T>(entry->second)) return 2;   // wrong type
    flags[name] = value;
    return 0;
  }
};

}  // namespace bergamot
}  // namespace marian

#endif  //  SRC_BERGAMOT_RESPONSE_OPTIONS_H_

// Register to register flags, their types & their default values
// Then setFeatureFlag to set it
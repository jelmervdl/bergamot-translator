/*
 * Bindings for Service class
 */

#include <emscripten/bind.h>

#include <future>
#include <memory>

#include "service.h"

using namespace emscripten;

using BlockingService = marian::bergamot::BlockingService;
using TranslationModel = marian::bergamot::TranslationModel;
using AlignedMemory = marian::bergamot::AlignedMemory;
using MemoryBundle = marian::bergamot::MemoryBundle;
using TranslationCache = marian::bergamot::TranslationCache;

val getByteArrayView(AlignedMemory& alignedMemory) {
  return val(typed_memory_view(alignedMemory.size(), alignedMemory.as<char>()));
}

EMSCRIPTEN_BINDINGS(aligned_memory) {
  class_<AlignedMemory>("AlignedMemory")
      .constructor<std::size_t, std::size_t>()
      .function("size", &AlignedMemory::size)
      .function("getByteArrayView", &getByteArrayView);

  register_vector<AlignedMemory*>("AlignedMemoryList");
}

// When source and target vocab files are same, only one memory object is passed from JS to
// avoid allocating memory twice for the same file. However, the constructor of the Service
// class still expects 2 entries in this case, where each entry has the shared ownership of the
// same AlignedMemory object. This function prepares these smart pointer based AlignedMemory objects
// for unique AlignedMemory objects passed from JS.
std::vector<std::shared_ptr<AlignedMemory>> prepareVocabsSmartMemories(std::vector<AlignedMemory*>& vocabsMemories) {
  auto sourceVocabMemory = std::make_shared<AlignedMemory>(std::move(*(vocabsMemories[0])));
  std::vector<std::shared_ptr<AlignedMemory>> vocabsSmartMemories;
  vocabsSmartMemories.push_back(sourceVocabMemory);
  if (vocabsMemories.size() == 2) {
    auto targetVocabMemory = std::make_shared<AlignedMemory>(std::move(*(vocabsMemories[1])));
    vocabsSmartMemories.push_back(std::move(targetVocabMemory));
  } else {
    vocabsSmartMemories.push_back(sourceVocabMemory);
  }
  return vocabsSmartMemories;
}

MemoryBundle prepareMemoryBundle(AlignedMemory* modelMemory, AlignedMemory* shortlistMemory,
                                 std::vector<AlignedMemory*> uniqueVocabsMemories,
                                 AlignedMemory* qualityEstimatorMemory) {
  MemoryBundle memoryBundle;
  memoryBundle.model = std::move(*modelMemory);
  memoryBundle.shortlist = std::move(*shortlistMemory);
  memoryBundle.vocabs = std::move(prepareVocabsSmartMemories(uniqueVocabsMemories));
  if (qualityEstimatorMemory != nullptr) {
    memoryBundle.qualityEstimatorMemory = std::move(*qualityEstimatorMemory);
  }

  return memoryBundle;
}

// This allows only shared_ptrs to be operational in JavaScript, according to emscripten.
// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#smart-pointers
std::shared_ptr<TranslationModel> TranslationModelFactory(const std::string& config, AlignedMemory* model,
                                                          AlignedMemory* shortlist, std::vector<AlignedMemory*> vocabs,
                                                          AlignedMemory* qualityEstimator) {
  MemoryBundle memoryBundle = prepareMemoryBundle(model, shortlist, vocabs, qualityEstimator);
  return std::make_shared<TranslationModel>(config, std::move(memoryBundle));
}

EMSCRIPTEN_BINDINGS(translation_model) {
  class_<TranslationModel>("TranslationModel")
      .smart_ptr_constructor("TranslationModel", &TranslationModelFactory, allow_raw_pointers());
}

std::shared_ptr<TranslationCache> TranslationCacheFactory(size_t size, size_t buckets) {
  return std::make_shared<TranslationCache>(size, buckets);
}

EMSCRIPTEN_BINDINGS(translation_cache) {
  class_<TranslationCache>("TranslationCache").smart_ptr_constructor("TranslationCache", &TranslationCacheFactory);
}

std::shared_ptr<BlockingService> BlockingServiceFactory() {
  BlockingService::Config config;
  config.logger.level = "critical";
  return std::make_shared<BlockingService>(config);
}

EMSCRIPTEN_BINDINGS(blocking_service) {
  class_<BlockingService>("BlockingService")
      .smart_ptr_constructor("BlockingService", &BlockingServiceFactory)
      .function("translate", &BlockingService::translateMultiple)
      .function("translateViaPivoting", &BlockingService::pivotMultiple);

  register_vector<std::string>("VectorString");
}

#ifndef INCLUDE_KVSTORE_STORE_HPP_
#define INCLUDE_KVSTORE_STORE_HPP_

#include <string>
#include <unordered_map>

#include "spdlog/sinks/stdout_color_sinks.h"

namespace kvstore {

// A generic key-value store.
template <class K, class V>
class Store {
 public:
  Store();

  V Get(const K& key);
  void Put(const K& key, const V& value);
  V Append(const K& key, const V& value);

 private:
  std::unordered_map<K, V> store_;

  std::shared_ptr<spdlog::logger> logger_;
};

template <class K, class V>
Store<K, V>::Store() {
  logger_ = spdlog::stdout_color_mt("store");
}

template <class K, class V>
V Store<K, V>::Get(const K& key) {
  logger_->trace("get {}", key);

  return store_[key];
}

template <class K, class V>
void Store<K, V>::Put(const K& key, const V& value) {
  logger_->trace("put {}", key);

  store_[key] = value;
}

template <class K, class V>
V Store<K, V>::Append(const K& key, const V& value) {
  logger_->trace("append {}", key);

  V existing_value;
  if (store_.find(key) != store_.end()) {
    existing_value = store_[key];
  }

  store_[key] = existing_value + value;
  return store_[key];
}

}  // namespace kvstore

#endif  // INCLUDE_KVSTORE_STORE_HPP_

/* Copyright (c) 2021 Scott McMillan <scott.andrew.mcmillan@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "spdlog/spdlog.h"
#include <memory>
#include <vector>
#include <wassail/wassail.hpp>

namespace fand {
  std::shared_ptr<spdlog::logger> logger();
  json read_config(const std::string);

  template <typename T>
  bool contains(std::vector<T> v, T i) {
    if (std::find(v.begin(), v.end(), i) != v.end()) {
      return true;
    }
    else {
      return false;
    }
  };
} // namespace fand

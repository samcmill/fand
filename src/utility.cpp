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

#include "config.h"

#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <cassert>
#include <fstream>
#include <wassail/wassail.hpp>

namespace fand {
  std::shared_ptr<spdlog::logger> logger() {
    auto logger = spdlog::get(LOGGER);

    if (not logger) {
      /* The standard logger was not setup, so create a null logger. */
      auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
      logger = std::make_shared<spdlog::logger>("null_logger", null_sink);
    }
    assert(logger);

    return (logger);
  }

  json read_config(const std::string file) {
    ::fand::logger()->debug("reading configuration file '{}'", file);

    try {
      std::ifstream config_stream(file);
      /* allow comments in the JSON file */
      json config = json::parse(config_stream, nullptr, true, true);
      ::fand::logger()->trace("{}", config.dump());
      return config;
    }
    catch (std::exception &e) {
      ::fand::logger()->error("error reading configuration file: '{}'",
                              e.what());
      exit(1);
    }
  }
} // namespace fand

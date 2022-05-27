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

#include "fand.hpp"
#include "config.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "utility.hpp"
#include <algorithm>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <unistd.h>
#include <vector>
#include <wassail/wassail.hpp>

#if defined(HAVE_EXECUTION_H) && __cpp_lib_execution >= 201603L
#include <execution>
#endif

namespace fand {
  fand::fand(system_t s, wassail::log_level log_level) {
    _system = s;

    /* initialize the system dispatch table.  each system type needs a
     * specialized function to create check pairs. */
    make_system_dispatch_table();

    /* setup the logging facilities */
    wassail::initialize(log_level);

    auto logger = spdlog::stderr_color_mt(LOGGER);

    /* assumes that wassail::log_level is identical to spdlog::level::level_enum
     */
    logger->set_level(static_cast<spdlog::level::level_enum>(log_level));

    logger->debug("wassail version {}", wassail::version());
  }

  void fand::add_check_pair(check_pair cp, std::shared_ptr<wassail::result> r) {
    ::fand::logger()->debug("adding check pair {0} / {1}", cp.check->name(),
                            cp.data->name());
    cp.result = r;
    checks.push_back(cp);
  }

  void fand::check() {
    ::fand::logger()->debug("invoking check subcommand for {} pairs",
                            checks.size());

    std::for_each(
#if __cpp_lib_execution >= 201603L
        std::execution::par,
#endif
        checks.cbegin(), checks.cend(), [&](const auto &cp) {
          ::fand::logger()->info("performing check {}", cp.check->name());

          /* get the data from the data source */
          json d = get_data(cp.data);

          try {
            /* perform the check on the data and store the result */
            auto r = cp.check->check(d);
            cp.result->add_child(r);
            ::fand::logger()->trace(static_cast<json>(r).dump());
          }
          catch (std::exception &e) {
            ::fand::logger()->error("error performing check {0}: '{1}'",
                                    cp.check->name(), e.what());
          }
        });
  }

  std::list<json> fand::collect() {
    ::fand::logger()->debug("invoking collect subcommand for {} pairs",
                            checks.size());

    /* collect the data */
    std::for_each(
#if __cpp_lib_execution >= 201603L
        std::execution::par,
#endif
        checks.cbegin(), checks.cend(),
        [&](const auto &cp) { get_data(cp.data); });

    /* create a list of data in json format */
    std::list<json> jsonl;
    std::map<size_t, bool> seen;

    for (auto const &cp : checks) {
      json j = cp.data->to_json();

      /* the same data source may be used in multiple check pairs.  the data
       * should only be included once.  compute a hash and skip duplicates. */
      auto h = std::hash<json>{}(j);

      /* Only include the data if it hasn't been encountered before. */
      if (seen.find(h) == seen.end()) {
        jsonl.push_back(j);
        seen[h] = true;
      }
    }

    return jsonl;
  }

  json fand::get_data(std::shared_ptr<wassail::data::common> d) {
    /* verify that the data source is enabled, i.e., valid, for this system */
    if (d->enabled()) {
      /* wassail caches data source evaluations.  if the data has already been
       * collected, just return it. */
      if (d->collected()) {
        ::fand::logger()->info("data source {} has already been collected",
                               d->name());
        return d->to_json();
      }
      else {
        try {
          ::fand::logger()->info("evaluating data source {0}", d->name());

          d->evaluate();

          json j = d->to_json();
          ::fand::logger()->trace(j.dump());

          return j;
        }
        catch (std::exception &e) {
          ::fand::logger()->error("error evaluating data source {0}: '{1}'",
                                  d->name(), e.what());
          return static_cast<json>(nullptr);
        }
      }
    }
    else {
      ::fand::logger()->error("data source {} is not enabled", d->name());
      return static_cast<json>(nullptr);
    }
  }

  void fand::list() {
    ::fand::logger()->debug("invoking list subcommand");

    std::cout << std::left << std::setw(30) << "Check"
              << "  " << std::left << std::setw(30) << "Data" << std::endl;
    std::cout << std::setw(30) << std::setfill('-') << "-"
              << "  " << std::setw(30) << std::setfill('-') << "-" << std::endl;
    std::cout << std::setfill(' ');

    std::for_each(checks.cbegin(), checks.cend(), [](const auto &cp) {
      std::cout << std::left << std::setw(30) << cp.check->name() << "  "
                << std::left << std::setw(30) << cp.data->name() << std::endl;
    });
  }

  void fand::load_data(std::unique_ptr<std::istream> in) {
    ::fand::logger()->debug("loading data");

    for (std::string s; std::getline((*in), s);) {
      json j = json::parse(s);
      ::fand::logger()->trace("read {}", j.dump());

      for (auto const &cp : checks) {
        if (cp.data->collected()) {
          continue;
        }

        if (j.value("name", "unknown") == cp.data->name()) {
          cp.data->from_json(j);
          ::fand::logger()->info("loaded data for {}", cp.data->name());
          continue;
        }
      }
    }
  }

  void fand::make_check_pairs(std::shared_ptr<options_t> options,
                              std::shared_ptr<wassail::result> r) {
    auto cps = system_dispatch_table[_system](options);
    for (const auto &cp : cps) {
      add_check_pair(cp, r);
    }
  }

  void fand::make_system_dispatch_table() {
    system_dispatch_table.emplace(system_t::linux_custom,
                                  make_system_checks<system_t::linux_custom>);
    system_dispatch_table.emplace(system_t::MacBookPro10_2,
                                  make_system_checks<system_t::MacBookPro10_2>);
  }
} // namespace fand

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

#include "systems.hpp"
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <wassail/wassail.hpp>

namespace fand {
  /*! \brief Categories of checks.  A check pair may belong to more than
   *  category
   */
  enum class category { CPU, FILESYSTEM, MEMORY, NETWORK, PERFORMANCE };

  /*! \brief A pair of a wassail check and the wassail data source that
   *  it uses.
   */
  struct check_pair {
    std::shared_ptr<wassail::check::common> check; /*!< wassail check */
    std::shared_ptr<wassail::data::common> data;   /*!< wassail data source */
    std::shared_ptr<wassail::result>
        result; /*!< Result of the check performed on the data */

    /*! \brief construct a check pair */
    check_pair(std::shared_ptr<wassail::check::common> c,
               std::shared_ptr<wassail::data::common> d)
        : check(c), data(d){};
  };

  struct options_t {
    std::vector<fand::category> categories = {
        category::CPU, category::FILESYSTEM, category::MEMORY,
        category::NETWORK};   /*!< list of default categories */
    std::string config_file;  /*!< path of the configuration file */
    std::string input_file;   /*!< path of the file containing the previously
                                 collected data */
    bool json_result = false; /*!< output the results as JSON */
    wassail::log_level log_level = wassail::log_level::warn; /*!< log level */
    std::string
        output_file;       /*!< path of the file to store the collected data */
    fand::system_t system; /*!< the system type, defines the check pairs */
  };

  /*! \brief Create the check pairs.  Specialized for each system type.
   */
  template <fand::system_t T>
  std::vector<fand::check_pair>
      make_system_checks(std::shared_ptr<fand::options_t>);

  class fand {
  public:
    fand(system_t, wassail::log_level = wassail::log_level::warn);

    /*! \brief Add a check pair to the object */
    void add_check_pair(check_pair, std::shared_ptr<wassail::result>);

    /*! \brief Perform the check */
    void check();

    /*! \brief Collect the data
     *  \return list of collected data in JSON format
     */
    std::list<json> collect();

    /*! \brief List the configured check pairs */
    void list();

    /*! \brief Load precollected data from the specified stream */
    void load_data(std::unique_ptr<std::istream>);

    /*! \brief Create the check pairs */
    void make_check_pairs(std::shared_ptr<options_t>,
                          std::shared_ptr<wassail::result>);

  private:
    /*! \brief Helper to collect the data for the specified data source */
    json get_data(std::shared_ptr<wassail::data::common>);

    /*! \brief Populate the dispatch table for each system type */
    void make_system_dispatch_table();

    /*! \brief List of check pairs */
    std::list<check_pair> checks;

    /*! \brief System type to check */
    system_t _system;

    /*! \brief Map the system type to a specialized make_system_checks()
     *  for the corresponding system type
     */
    inline static std::map<system_t, std::function<std::vector<check_pair>(
                                         std::shared_ptr<options_t>)>>
        system_dispatch_table;
  };
} // namespace fand

#include "fand.hpp"
#include "systems.hpp"
#include "utility.hpp"
#include <vector>
#include <wassail/wassail.hpp>

namespace fand {
  template <>
  std::vector<check_pair> make_system_checks<system_t::linux_custom>(
      std::shared_ptr<options_t> options) {
    ::fand::logger()->debug("making check pairs for system linux custom");

    if (options->config_file.empty()) {
      ::fand::logger()->error("A configuration file must be specified");
      exit(1);
    }

    json config = ::fand::read_config(options->config_file);

    std::vector<check_pair> checks;

    /* declare the data source once so they can be reused without
     * needlessly reevaluating them */
    auto getmntent = std::make_shared<wassail::data::getmntent>();
    auto stream = std::make_shared<wassail::data::stream>();
    auto sysconf = std::make_shared<wassail::data::sysconf>();

    if (contains(options->categories, category::CPU)) {
      auto num_cores = json::json_pointer("/cpu/core_count/num_cores");

      if (config.contains(num_cores)) {
        int parameter = config.value(num_cores, 0);
        checks.emplace_back(check_pair(
            std::make_shared<wassail::check::cpu::core_count>(parameter),
            sysconf));
      }
    }

    if (contains(options->categories, category::FILESYSTEM)) {
      auto fs = json::json_pointer("/disk/percent_free");

      if (config.contains(fs)) {
        for (auto f : config.value(fs, json::array())) {
          if (f.contains("filesystem") and f.contains("percent")) {
            std::string parameter1 = f.value("filesystem", "");
            float parameter2 = f.value("percent", 0.0);

            checks.emplace_back(
                check_pair(std::make_shared<wassail::check::disk::percent_free>(
                               parameter1, parameter2),
                           getmntent));
          }
        }
      }
    }

    if (contains(options->categories, category::MEMORY)) {
      auto mem_size = json::json_pointer("/memory/physical_size/mem_size");
      auto tolerance = json::json_pointer("/memory/physical_size/tolerance");

      if (config.contains(mem_size) and config.contains(tolerance)) {
        uint64_t parameter1 = config.value(mem_size, 0UL);
        uint64_t parameter2 = config.value(tolerance, 0UL);
        checks.emplace_back(
            check_pair(std::make_shared<wassail::check::memory::physical_size>(
                           parameter1, parameter2),
                       sysconf));
      }
    }

    return checks;
  } // namespace fand
} // namespace fand

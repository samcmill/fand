#include "fand.hpp"
#include "systems.hpp"
#include "utility.hpp"
#include <vector>
#include <wassail/wassail.hpp>

namespace fand {
  template <>
  std::vector<check_pair> make_system_checks<system_t::MacBookPro10_2>(
      std::shared_ptr<options_t> options) {
    ::fand::logger()->debug("making check pairs for system MacBookPro10,2");

    std::vector<check_pair> checks;

    /* declare the data source once so they can be reused without
     * needlessly reevaluating them */
    auto getfsstat = std::make_shared<wassail::data::getfsstat>();
    auto stream = std::make_shared<wassail::data::stream>();
    auto sysconf = std::make_shared<wassail::data::sysconf>();

    if (contains(options->categories, category::CPU)) {
      checks.emplace_back(check_pair(
          std::make_shared<wassail::check::cpu::core_count>(4), sysconf));
    }

    if (contains(options->categories, category::FILESYSTEM)) {
      checks.emplace_back(check_pair(
          std::make_shared<wassail::check::disk::percent_free>("/", 5),
          getfsstat));
    }

    if (contains(options->categories, category::MEMORY)) {
      checks.emplace_back(
          check_pair(std::make_shared<wassail::check::memory::physical_size>(
                         8UL * 1024 * 1024 * 1024, 1UL * 1024 * 1024),
                     sysconf));
    }

    if (contains(options->categories, category::PERFORMANCE)) {
      auto c = std::make_shared<wassail::check::rules_engine>(
          "Checking STREAM performance",
          "Observed performance is less than 12000 MB/s",
          "Unable to perform check",
          "Observed performance is more than 12000 MB/s");
      c->add_rule([](json j) {
        return j.value(json::json_pointer("/data/triad"), 0.0) >= 12000.0;
      });
      checks.emplace_back(check_pair(c, stream));
    }

    return checks;
  }
} // namespace fand

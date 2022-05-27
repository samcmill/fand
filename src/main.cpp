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

#include "CLI11/CLI11.hpp"
#include "fand.hpp"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <wassail/wassail.hpp>

/*! Process the command line */
void cli_init(CLI::App &cli, std::shared_ptr<fand::options_t> options) {
  /* map string value to wassail log levels */
  std::map<std::string, wassail::log_level> log_map{
      {"critical", wassail::log_level::critical},
      {"error", wassail::log_level::err},
      {"warning", wassail::log_level::warn},
      {"info", wassail::log_level::info},
      {"debug", wassail::log_level::debug},
      {"trace", wassail::log_level::trace}};

  /* map string value to fand categories */
  std::map<std::string, fand::category> category_map{
      {"cpu", fand::category::CPU},
      {"filesystem", fand::category::FILESYSTEM},
      {"memory", fand::category::MEMORY},
      {"network", fand::category::NETWORK},
      {"performance", fand::category::PERFORMANCE}};

  /* map string value to fand system types */
  std::map<std::string, fand::system_t> system_map{
      {"linux_custom", fand::system_t::linux_custom},
      {"MacBookPro10,2", fand::system_t::MacBookPro10_2}};

  /* command line options appear in the help message in the order they
   * are added, so make sure they are in alphabetical order */
  cli.add_option("-x,--category", options->categories, "Categories")
      ->delimiter(',')
      ->transform(CLI::CheckedTransformer(category_map, CLI::ignore_case));

  cli.add_option("-c,--config", options->config_file, "Configuration file")
      ->check(CLI::ExistingPath);

  cli.add_option("-l,--log-level", options->log_level, "Log level")
      ->transform(CLI::CheckedTransformer(log_map, CLI::ignore_case));
  //    ->transform(CLI::IsMember(log_map));
  //->default_val("warning");

#ifdef FAND_SYSTEM
  options->system = system_map[FAND_SYSTEM];
#else
  cli.add_option("-s,--system", options->system, "System")
      ->transform(CLI::CheckedTransformer(system_map, CLI::ignore_case))
      ->envname("FAND_SYSTEM")
      ->required();
#endif

  cli.add_flag_function("-v,--version",
                        [](size_t num) {
                          std::cout << std::string(PACKAGE_STRING) << std::endl;
                          exit(0);
                        },
                        "Print version string");

  /* subcommands also appear in the help message in the order they are defined,
   * so make sure this is alphabetical */
  cli.require_subcommand(1);

  /* collect data and check it */
  auto check_subcmd = cli.add_subcommand("check", "check subcommand");
  check_subcmd->add_option("-f,--file", options->input_file, "Input file")
      ->check(CLI::ExistingPath);
  auto format = check_subcmd->add_option_group("format", "Output format type");
  format->add_flag("-j,--json", options->json_result, "JSON output format");

  /* collect data and stop */
  auto collect_subcmd = cli.add_subcommand("collect", "collect subcommand");
  collect_subcmd->add_option("-f,--file", options->output_file, "Output file")
      ->check(CLI::NonexistentPath);

  /* list the checks and stop */
  auto list_subcmd = cli.add_subcommand("list", "list subcommand");
}

int main(int argc, char **argv) {
  auto options = std::make_shared<fand::options_t>();

  /* Setup CLI */
  CLI::App cli{"fand system health analyzer"};
  cli_init(cli, options);

  try {
    cli.parse(argc, argv);
  }
  catch (const CLI::ParseError &e) {
    return cli.exit(e);
  }

  /* construct fand object */
  auto f = fand::fand(options->system, options->log_level);

  /* create a top level wassail result.  all the results will be children
   * of this result. */
  auto overall = wassail::make_result();
  overall->brief = "Overall system health status";

  /* create check / data pairs */
  f.make_check_pairs(options, overall);

  if (cli.got_subcommand("check")) {
    /* perform system health check */

    if (not options->input_file.empty()) {
      /* read data from file */
      f.load_data(
          std::make_unique<std::ifstream>(std::ifstream{options->input_file}));
    }

    /* perform system health check */
    f.check();

    /* set overall health based on check results */
    overall->priority = overall->max_priority();
    overall->issue = overall->max_issue();

    if (options->json_result) {
      /* output as json */
      std::cout << static_cast<json>(overall) << std::endl;
    }
    else {
      /* human readable output */
      std::cout << overall;
    }

    /* set exit code */
    if (overall->issue == wassail::result::issue_t::NO) {
      return 0;
    }
    else {
      return 1;
    }
  }
  else if (cli.got_subcommand("collect")) {
    /* just collect data */
    std::list<json> jsonl = f.collect();

    /* dump to standard output by default */
    auto out = std::make_unique<std::ostream>(std::cout.rdbuf());

    if (not options->output_file.empty()) {
      /* write to file instead */
      out =
          std::make_unique<std::ofstream>(std::ofstream{options->output_file});
    }

    /* print out data */
    std::for_each(jsonl.cbegin(), jsonl.cend(), [&](const json &j) {
      (*out) << j.dump(-1, ' ', false, json::error_handler_t::replace)
             << std::endl;
    });
  }
  else if (cli.got_subcommand("list")) {
    /* list checks */
    f.list();
  }

  return 0;
}

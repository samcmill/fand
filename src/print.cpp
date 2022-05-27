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

#include "utility.hpp"
#include <istream>
#include <list>
#include <sstream>
#include <string>
#include <wassail/wassail.hpp>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

const std::string green("\033[1;32m");
const std::string red("\033[0;31m");
const std::string yellow("\033[1;33m");
const std::string reset("\033[0m");

namespace fand {
  namespace {
    std::string line_wrap(const std::string &in, const uint8_t width,
                          const uint8_t leading_indent,
                          const uint8_t hanging_indent, const char fill = '.') {
      std::list<std::string> l;

      if (in.length() + leading_indent <= width) {
        l.push_back(std::string(leading_indent, ' ') + in +
                    std::string(width - in.length() - leading_indent, fill));
      }
      else {
        size_t cur = 0;
        size_t indent = leading_indent;
        size_t next = width - indent;

        while (next < in.size()) {
          size_t space = in.rfind(' ', next);

          if (space == std::string::npos or space <= cur) {
            space = in.find(' ', next);

            if (space == std::string::npos) {
              break;
            }
          }

          l.push_back(std::string(indent, ' ') + in.substr(cur, space - cur));

          cur = space + 1;
          indent = hanging_indent;
          next = cur + width - indent;
        }

        l.push_back(std::string(indent, ' ') + in.substr(cur) +
                    std::string(next - in.length(), fill));
      }

      std::stringstream out;
      if (not l.empty()) {
        std::copy(l.begin(), std::prev(l.end()),
                  std::ostream_iterator<std::string>(out, "\n"));
        out << l.back();
      }

      return out.str();
    }
  } // namespace

  void print_result(std::ostream &os, std::shared_ptr<wassail::result> const &r,
                    const uint8_t indent) {
#ifdef HAVE_IOCTL
    winsize ws;
    ioctl(0, TIOCGWINSZ, &ws);
    size_t width = ws.ws_col - 7 - 1; // UNKNOWN is 7 characters long
#else
    size_t width = 72; // 80 - 7 - 1
#endif

    auto brief = line_wrap(r->brief, width, indent, indent + 2);
    std::cout << brief << r->issue << std::endl;

    if (not r->detail.empty()) {
      auto detail = line_wrap(r->detail, width, indent + 2, indent + 4, ' ');
      std::cout << detail << std::endl;
    }

    std::cout << "Level: " << r->priority << std::endl;

    for (auto rr : r->children) {
      print_result(os, rr, indent + 2);
    }
  }
} // namespace fand

namespace wassail {
  std::ostream &operator<<(std::ostream &os,
                           enum wassail::result::issue_t const &i) {
    switch (i) {
    case wassail::result::issue_t::YES:
      os << red << "NOT OK" << reset;
      break;
    case wassail::result::issue_t::NO:
      os << green << "OK" << reset;
      break;
    case wassail::result::issue_t::MAYBE:
      os << yellow << "UNKNOWN" << reset;
      break;
    default:
      os << "UNKNOWN";
    }

    return os;
  }

  std::ostream &operator<<(std::ostream &os,
                           enum wassail::result::priority_t const &p) {
    switch (p) {
    case wassail::result::priority_t::EMERGENCY:
      os << red << "EMERGENCY" << reset;
      break;
    case wassail::result::priority_t::ALERT:
      os << red << "ALERT" << reset;
      break;
    case wassail::result::priority_t::ERROR:
      os << red << "ERROR" << reset;
      break;
    case wassail::result::priority_t::WARNING:
      os << yellow << "WARNING" << reset;
      break;
    case wassail::result::priority_t::NOTICE:
      os << green << "NOTICE" << reset;
      break;
    case wassail::result::priority_t::INFO:
      os << green << "INFO" << reset;
      break;
    case wassail::result::priority_t::DEBUG:
      os << green << "DEBUG" << reset;
      break;
    default:
      os << yellow << "UNKNOWN" << reset;
    }

    return os;
  }

  std::ostream &operator<<(std::ostream &os,
                           std::shared_ptr<wassail::result> const &r) {
    fand::print_result(os, r, 0);
    return os;
  }
} // namespace wassail

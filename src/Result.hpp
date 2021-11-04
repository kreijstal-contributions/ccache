// Copyright (C) 2019-2021 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#pragma once

#include "third_party/nonstd/expected.hpp"
#include "third_party/nonstd/optional.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace core {

class CacheEntryReader;
class CacheEntryWriter;

} // namespace core

class Context;

namespace Result {

extern const std::string k_file_suffix;
extern const uint8_t k_magic[4];
extern const uint8_t k_version;

extern const char* const k_unknown_file_type;

using UnderlyingFileTypeInt = uint8_t;
enum class FileType : UnderlyingFileTypeInt {
  // These values are written into the cache result file. This means they must
  // never be changed or removed unless the result file version is incremented.
  // Adding new values is OK.

  // The main output specified with -o or implicitly from the input filename.
  object = 0,

  // Dependency file specified with -MF or implicitly from the output filename.
  dependency = 1,

  // Text sent to standard output.
  stderr_output = 2,

  // Coverage notes file generated by -ftest-coverage with filename in unmangled
  // form, i.e. output file but with a .gcno extension.
  coverage_unmangled = 3,

  // Stack usage file generated by -fstack-usage, i.e. output file but with a
  // .su extension.
  stackusage = 4,

  // Diagnostics output file specified by --serialize-diagnostics.
  diagnostic = 5,

  // DWARF object file generated by -gsplit-dwarf, i.e. output file but with a
  // .dwo extension.
  dwarf_object = 6,

  // Coverage notes file generated by -ftest-coverage with filename in mangled
  // form, i.e. full output file path but with a .gcno extension and with
  // slashes replaced with hashes.
  coverage_mangled = 7,
};

const char* file_type_to_string(FileType type);

std::string gcno_file_in_mangled_form(const Context& ctx);
std::string gcno_file_in_unmangled_form(const Context& ctx);

struct FileSizeAndCountDiff
{
  int64_t size_kibibyte;
  int64_t count;

  FileSizeAndCountDiff& operator+=(const FileSizeAndCountDiff& other);
};

// This class knows how to read a result cache entry.
class Reader
{
public:
  Reader(const std::string& result_path);

  class Consumer
  {
  public:
    virtual ~Consumer() = default;

    virtual void on_header(core::CacheEntryReader& cache_entry_reader,
                           uint8_t result_format_version) = 0;
    virtual void on_entry_start(uint32_t entry_number,
                                FileType file_type,
                                uint64_t file_len,
                                nonstd::optional<std::string> raw_file) = 0;
    virtual void on_entry_data(const uint8_t* data, size_t size) = 0;
    virtual void on_entry_end() = 0;
  };

  // Returns error message on error, otherwise nonstd::nullopt.
  nonstd::optional<std::string> read(Consumer& consumer);

private:
  const std::string m_result_path;

  bool read_result(Consumer& consumer);
  void read_entry(core::CacheEntryReader& cache_entry_reader,
                  uint32_t entry_number,
                  Reader::Consumer& consumer);
};

// This class knows how to write a result cache entry.
class Writer
{
public:
  Writer(Context& ctx, const std::string& result_path);

  // Register a file to include in the result. Does not throw.
  void write(FileType file_type, const std::string& file_path);

  // Write registered files to the result. Returns an error message on error.
  nonstd::expected<FileSizeAndCountDiff, std::string> finalize();

private:
  Context& m_ctx;
  const std::string m_result_path;
  std::vector<std::pair<FileType, std::string>> m_entries_to_write;

  FileSizeAndCountDiff do_finalize();
  static void write_embedded_file_entry(core::CacheEntryWriter& writer,
                                        const std::string& path,
                                        uint64_t file_size);
  FileSizeAndCountDiff write_raw_file_entry(const std::string& path,
                                            uint32_t entry_number);
};

} // namespace Result

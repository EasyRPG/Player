/** **********************************************************************
 *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗
 *  ██║     ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝
 *  ██║     █████╗  ███████║███████╗ ╚████╔╝
 *  ██║     ██╔══╝  ██╔══██║╚════██║  ╚██╔╝
 *  ███████╗███████╗██║  ██║███████║   ██║
 *  ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝
 *
 *          The EasyRPG engine, with runtime extensions, easily.
 *
 *  Developed by @wys
 *  https://github.com/wys-prog
 * 
 *  This file is free and open source. You may credit its usage in sources
 *  by using this Github profile: https://github.com/wys-prog.
 * 
 *  You may see the evolution of this file at https://github.com/wys-prog/leasy.
 * 
 *  0xEF9087A@wys-prog.https://github.com/wys-prog/leasy
 * 
 * **********************************************************************/


#pragma once

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "kits/cppsupport/bases.hpp"

namespace leasy::ios {

namespace ansi {

inline constexpr std::string_view reset          = "\033[0m";
inline constexpr std::string_view black          = "\033[30m";
inline constexpr std::string_view red            = "\033[31m";
inline constexpr std::string_view green          = "\033[32m";
inline constexpr std::string_view yellow         = "\033[33m";
inline constexpr std::string_view blue           = "\033[34m";
inline constexpr std::string_view magenta        = "\033[35m";
inline constexpr std::string_view cyan           = "\033[36m";
inline constexpr std::string_view white          = "\033[37m";
inline constexpr std::string_view bright_black   = "\033[90m";
inline constexpr std::string_view bright_red     = "\033[91m";
inline constexpr std::string_view bright_green   = "\033[92m";
inline constexpr std::string_view bright_yellow  = "\033[93m";
inline constexpr std::string_view bright_blue    = "\033[94m";
inline constexpr std::string_view bright_magenta = "\033[95m";
inline constexpr std::string_view bright_cyan    = "\033[96m";
inline constexpr std::string_view bright_white   = "\033[97m";
inline constexpr std::string_view bold           = "\033[1m";
inline constexpr std::string_view dim            = "\033[2m";

inline std::string strip(std::string_view text) {
  std::string out;
  out.reserve(text.size());
  for (std::size_t i = 0; i < text.size(); ) {
    if (text[i] == '\033' && i + 1 < text.size() && text[i + 1] == '[') {
      i += 2;
      while (i < text.size() && !(text[i] >= 0x40 && text[i] <= 0x7E))
        ++i;
      ++i;
    } else {
      out += text[i++];
    }
  }
  return out;
}

} // namespace ansi

class buffer {
public:
  template<typename... Args>
  inline void write(Args&&... args) {
    (stream_ << ... << std::forward<Args>(args));
  }

  inline std::string str() const {
    return stream_.str();
  }

  inline void clear() {
    stream_.str("");
    stream_.clear();
  }

private:
  std::ostringstream stream_;
};

class sink {
public:
  inline virtual ~sink() = default;
  inline virtual void write(std::string_view text) = 0;
  inline virtual void flush() {}
};

class console_sink final : public sink {
public:
  inline explicit console_sink(std::ostream& os = std::cout)
    : os_(os) {}

  inline void write(std::string_view text) override {
    std::lock_guard lock(mutex_);
    os_ << text;
  }

  inline void flush() override {
    std::lock_guard lock(mutex_);
    os_.flush();
  }

private:
  std::ostream& os_;
  std::mutex    mutex_;
};

class file_sink final : public sink {
public:
  inline explicit file_sink(const std::filesystem::path& path)
    : file_(path, std::ios::app) {}

  inline void write(std::string_view text) override {
    std::lock_guard lock(mutex_);
    file_ << ansi::strip(text);
  }

  inline void flush() override {
    std::lock_guard lock(mutex_);
    file_.flush();
  }

private:
  std::ofstream file_;
  std::mutex    mutex_;
};

class memory_sink final : public sink {
public:
  inline void write(std::string_view text) override {
    std::lock_guard lock(mutex_);
    data_ += ansi::strip(text);
  }

  inline std::string str() const {
    std::lock_guard lock(mutex_);
    return data_;
  }

  inline void clear() {
    std::lock_guard lock(mutex_);
    data_.clear();
  }

private:
  mutable std::mutex mutex_;
  std::string        data_;
};

class channel;

class attachment {
  friend class channel;
public:
  inline attachment() = default;

  inline attachment(const attachment&)            = delete;
  inline attachment& operator=(const attachment&) = delete;

  inline attachment(attachment&& other) noexcept
    : owner_(other.owner_),
      sink_(std::move(other.sink_)) {
    other.owner_ = nullptr;
  }

  inline attachment& operator=(attachment&& other) noexcept {
    if (this == &other)
      return *this;
    disconnect();
    owner_ = other.owner_;
    sink_  = std::move(other.sink_);
    other.owner_ = nullptr;
    return *this;
  }

  inline ~attachment() {
    disconnect();
  }

  inline void disconnect();

  inline bool connected() const {
    return owner_ != nullptr;
  }

private:
  inline attachment(channel* owner, std::shared_ptr<sink> sink)
    : owner_(owner), sink_(std::move(sink)) {}

  channel*               owner_ = nullptr;
  std::shared_ptr<sink>  sink_;
};

struct format_options {
  bool        timestamp = false;
  bool        thread_id = false;
  std::string prefix;
  std::string color;
};

class channel {
public:
  channel() = default;

  template<typename... Args>
  inline void write(Args&&... args) {
    if (!enabled_)
      return;
    std::ostringstream ss;
    (ss << ... << std::forward<Args>(args));
    dispatch(format(ss.str()));
  }

  template<typename... Args>
  inline void writeln(Args&&... args) {
    if (!enabled_)
      return;
    std::ostringstream ss;
    (ss << ... << std::forward<Args>(args));
    ss << '\n';
    dispatch(format(ss.str()), true);
  }

  inline attachment attach(std::shared_ptr<sink> output) {
    {
      std::lock_guard lock(mutex_);
      outputs_.push_back(output);
    }
    return attachment(this, std::move(output));
  }

  inline void clear_outputs() {
    std::lock_guard lock(mutex_);
    outputs_.clear();
  }

  inline channel& enabled(bool value) {
    enabled_ = value;
    return *this;
  }

  inline channel& timestamp(bool value) {
    format_.timestamp = value;
    return *this;
  }

  inline channel& thread_id(bool value) {
    format_.thread_id = value;
    return *this;
  }

  inline channel& prefix(std::string value) {
    format_.prefix = std::move(value);
    return *this;
  }

  inline channel& color(std::string value) {
    format_.color = std::move(value);
    return *this;
  }

private:
  friend class attachment;

  inline void detach(const std::shared_ptr<sink>& target) {
    std::lock_guard lock(mutex_);
    stl2::compat::cpp17::erase_if(
      outputs_,
      [&](const auto& weak) {
        auto locked = weak.lock();
        return !locked || locked == target;
      }
    );
  }

  inline void dispatch(const std::string& text, bool flush = false) {
    std::vector<std::shared_ptr<sink>> sinks;
    {
      std::lock_guard lock(mutex_);
      stl2::compat::cpp17::erase_if(outputs_, [](const auto& weak) {
        return weak.expired();
      });
      sinks.reserve(outputs_.size());
      for (auto& weak : outputs_) {
        if (auto s = weak.lock())
          sinks.push_back(std::move(s));
      }
    }
    for (auto& s : sinks) {
      if (!text.empty())
        s->write(text);
      if (flush)
        s->flush();
    }
  }

  std::string format(const std::string& text) const {
    std::ostringstream ss;

    if (format_.timestamp) {
      auto now = std::chrono::system_clock::now();
      auto tt  = std::chrono::system_clock::to_time_t(now);
      std::tm tm{};
#ifdef _WIN32
      localtime_s(&tm, &tt);
#else
      localtime_r(&tt, &tm);
#endif
      ss << '[' << std::put_time(&tm, "%H:%M:%S") << "] ";
    }

    if (format_.thread_id) {
      ss << "[T:" << std::this_thread::get_id() << "] ";
    }

    if (!format_.prefix.empty()) {
      ss << format_.prefix << ' ';
    }

    ss << text;

    if (!format_.color.empty()) {
      return format_.color + ss.str() + std::string(ansi::reset);
    }

    return ss.str();
  }

  mutable std::mutex                  mutex_;
  std::vector<std::weak_ptr<sink>>    outputs_;
  bool                                enabled_ = true;
  format_options                      format_;
};

inline void attachment::disconnect() {
  if (!owner_)
    return;
  owner_->detach(sink_);
  sink_.reset();
  owner_ = nullptr;
}

class temp_file {
public:
  inline temp_file() {
    auto dir = std::filesystem::temp_directory_path();
    path_ = dir / (
      "leasy_" +
      std::to_string(
        std::chrono::steady_clock::now()
          .time_since_epoch()
          .count()
      ) +
      ".tmp"
    );
    std::ofstream file(path_);
  }

  inline ~temp_file() {
    std::error_code ec;
    std::filesystem::remove(path_, ec);
  }

  inline const std::filesystem::path& path() const {
    return path_;
  }

private:
  std::filesystem::path path_;
};

inline std::shared_ptr<sink> console(std::ostream& stream = std::cout) {
  return std::make_shared<console_sink>(stream);
}

inline std::shared_ptr<sink> file(const std::filesystem::path& path) {
  return std::make_shared<file_sink>(path);
}

inline std::shared_ptr<memory_sink> memory() {
  return std::make_shared<memory_sink>();
}

class io_system {
public:
  channel System;
  channel Debug;
  channel Warning;
  channel Error;
  channel Info;
  channel Gui;

  inline io_system() {
    System.prefix("leasy.io.System");
    Debug.prefix("leasy.io.Debug");
    Warning.prefix("leasy.io.Warning");
    Error.prefix("leasy.io.Error");
    Info.prefix("leasy.io.Info");
    basicsetup();
  }

private:
  inline void basicsetup() {
    system_console_ = System.attach(console());
    debug_console_ = Debug.attach(console());
    info_console_ = Info.attach(console());
    gui_ = Gui.attach(console());
    warning_console_ = Warning.attach(console(std::cerr));
    error_console_   = Error.attach(console(std::cerr));

    System.timestamp(true);
    Debug.timestamp(true);
    Warning.timestamp(true);
    Error.timestamp(true);
    Info.timestamp(true);
    Gui.timestamp(false);

    System.color(std::string(ansi::white));
    Debug.color(std::string(ansi::bright_blue));
    Warning.color(std::string(ansi::yellow));
    Error.color(std::string(ansi::bright_red));
    Info.color(std::string(ansi::bright_cyan));
  }

  attachment system_console_;
  attachment debug_console_;
  attachment warning_console_;
  attachment error_console_;
  attachment info_console_;
  attachment gui_;
};

} // namespace leasy::ios

namespace leasy {

  extern ios::io_system &io();

} // namespace leasy


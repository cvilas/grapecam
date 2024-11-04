/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Raspberry Pi Ltd
 *
 * Helper to parse options for streams
 */

#pragma once

#include <optional>

#include <libcamera/stream.h>

#include "options.h"

namespace libcamera {
class CameraConfiguration;
}

class StreamKeyValueParser : public KeyValueParser {
public:
  StreamKeyValueParser();

  KeyValueParser::Options parse(const char* arguments) override;

  static auto roles(const OptionValue& values) -> std::vector<libcamera::StreamRole>;
  static auto updateConfiguration(libcamera::CameraConfiguration* config, const OptionValue& values)
      -> int;

private:
  static auto parseRole(const KeyValueParser::Options& options)
      -> std::optional<libcamera::StreamRole>;
};

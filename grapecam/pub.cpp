//=================================================================================================
// Copyright (C) 2024 grapecam contributors
//=================================================================================================

#include <cstdlib>
#include <print>

#include <grape/app/app.h>
#include <grape/conio/program_options.h>
#include <libcamera/libcamera.h>

//=================================================================================================
// - Reads configuration file to open a specified camera and configure it
// - Publishes camera frames on a specified topic
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto maybe_args =
        grape::conio::ProgramDescription("Camera stream publisher")                     //
            .declareOption<std::string>("config", "Top level configuration file path")  //
            .parse(argc, argv);
    if (not maybe_args.has_value()) {
      throw std::runtime_error(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();
    const auto maybe_config = args.getOption<std::string>("config");
    if (not maybe_config.has_value()) {
      throw std::runtime_error(toString(maybe_config.error()));
    }
    grape::app::init(maybe_config.value());
    grape::app::cleanup();
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}

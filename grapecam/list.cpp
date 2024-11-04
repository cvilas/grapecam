//=================================================================================================
// Copyright (C) 2024 grapecam contributors
//=================================================================================================

#include <cstdlib>
#include <print>

#include <libcamera/libcamera.h>

//=================================================================================================
// Lists cameras available on the host
auto main() -> int {
  try {
    auto cm = std::make_unique<libcamera::CameraManager>();
    cm->start();

    auto cameras = cm->cameras();
    if (cameras.empty()) {
      std::println("No cameras found.");
      cm->stop();
      return EXIT_FAILURE;
    }

    for (auto const& device : cameras) {
      const auto& props = device->properties();
      const auto& model = props.get(libcamera::properties::Model);
      const auto id = device->id();
      auto camera = cm->get(id);
      std::println("{}\t({})", (model ? *model : "NoName"), id);
      // camera->acquire();
      const auto config = camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
      for (const auto& stream_config : *config) {
        std::println("\t{}", stream_config.toString());
      }
      // camera->release();
    }

    cm->stop();

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::puts(ex.what());
    return EXIT_FAILURE;
  }
}

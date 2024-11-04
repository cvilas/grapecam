/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * qcam - The libcamera GUI test application
 */

#include <QApplication>
#include <QtDebug>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include <libcamera/camera_manager.h>

#include "common/options.h"
#include "common/stream_options.h"
#include "main_window.h"
#include "message_handler.h"

using namespace libcamera;

namespace {

void signalHandler([[maybe_unused]] int signal) {
  qInfo() << "Exiting";
  qApp->quit();
}

auto parseOptions(int argc, char* argv[]) -> OptionsParser::Options {
  StreamKeyValueParser streamKeyValue;

  OptionsParser parser;
  parser.addOption(OptHelp, OptionNone, "Display this help message", "help");
  parser.addOption(OptRenderer, OptionString, "Choose the renderer type {qt,gles} (default: qt)",
                   "renderer", ArgumentRequired, "renderer");
  parser.addOption(OptStream, &streamKeyValue, "Set configuration of a camera stream", "stream",
                   true);
  parser.addOption(OptVerbose, OptionNone, "Print verbose log messages", "verbose");

  auto options = parser.parse(argc, argv);
  if (options.isSet(OptHelp)) {
    parser.usage();
  }

  return options;
}

} /* namespace */

auto main(int argc, char** argv) -> int {
  QApplication app(argc, argv);

  const auto options = parseOptions(argc, argv);
  if (!options.valid()) {
    return EXIT_FAILURE;
  }
  if (options.isSet(OptHelp)) {
    return EXIT_SUCCESS;
  }

  MessageHandler msgHandler(options.isSet(OptVerbose));

  struct sigaction sa = {};
  sa.sa_handler = &signalHandler;
  sigaction(SIGINT, &sa, nullptr);

  auto cm = std::make_unique<libcamera::CameraManager>();
  auto ret = cm->start();
  if (ret) {
    qInfo() << "Failed to start camera manager:" << strerror(-ret);
    return EXIT_FAILURE;
  }

  auto main_window = std::make_unique<MainWindow>(cm.get(), options);
  main_window->show();
  ret = app.exec();
  main_window.reset();
  cm->stop();
  cm.reset();
  return ret;
}

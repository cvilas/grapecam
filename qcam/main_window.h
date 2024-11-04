/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * qcam - Main application window
 */

#pragma once

#include <QElapsedTimer>
#include <QIcon>
#include <QMainWindow>
#include <QMutex>
#include <QObject>
#include <QPushButton>
#include <QQueue>
#include <QTimer>
#include <memory>
#include <vector>

#include "common/stream_options.h"
#include "viewfinder.h"

namespace libcamera {
class Camera;
class CameraManager;
class FrameBuffer;
class FrameBufferAllocator;
class ControlList;
class Request;
}  // namespace libcamera

class QAction;

class CameraSelectorDialog;
class Image;
class HotplugEvent;

enum {
  OptHelp = 'h',
  OptRenderer = 'r',
  OptStream = 's',
  OptVerbose = 'v',
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(libcamera::CameraManager* cm, const OptionsParser::Options& options);
  ~MainWindow() override;

  auto event(QEvent* e) -> bool override;

private Q_SLOTS:
  void quit();
  void updateTitle();

  void switchCamera();
  void toggleCapture(bool start);

  void saveImageAs();
  void captureRaw();
  void processRaw(libcamera::FrameBuffer* buffer, const libcamera::ControlList& metadata);

  void renderComplete(libcamera::FrameBuffer* buffer);

private:
  auto createToolbars() -> int;

  auto chooseCamera() -> std::string;
  auto openCamera() -> int;

  auto startCapture() -> int;
  void stopCapture();

  void addCamera(std::shared_ptr<libcamera::Camera> camera);
  void removeCamera(std::shared_ptr<libcamera::Camera> camera);

  auto queueRequest(libcamera::Request* request) -> int;
  void requestComplete(libcamera::Request* request);
  void processCapture();
  void processHotplug(HotplugEvent* e);
  void processViewfinder(libcamera::FrameBuffer* buffer);

  /* UI elements */
  QToolBar* toolbar_{ nullptr };
  QAction* startStopAction_{ nullptr };
  QPushButton* cameraSelectButton_{ nullptr };
  QAction* saveRaw_{ nullptr };
  ViewFinder* viewfinder_{ nullptr };

  QIcon iconPlay_;
  QIcon iconStop_;

  QString title_;
  QTimer titleTimer_;

  CameraSelectorDialog* cameraSelectorDialog_{ nullptr };

  /* Options */
  const OptionsParser::Options& options_;

  /* Camera manager, camera, configuration and buffers */
  libcamera::CameraManager* cm_{ nullptr };
  std::shared_ptr<libcamera::Camera> camera_;
  libcamera::FrameBufferAllocator* allocator_{ nullptr };

  std::unique_ptr<libcamera::CameraConfiguration> config_{ nullptr };
  std::map<libcamera::FrameBuffer*, std::unique_ptr<Image>> mappedBuffers_;

  /* Capture state, buffers queue and statistics */
  bool isCapturing_{ false };
  bool captureRaw_{ false };
  libcamera::Stream* vfStream_{ nullptr };
  libcamera::Stream* rawStream_{ nullptr };
  std::map<const libcamera::Stream*, QQueue<libcamera::FrameBuffer*>> freeBuffers_;
  QQueue<libcamera::Request*> doneQueue_;
  QQueue<libcamera::Request*> freeQueue_;
  QMutex mutex_; /* Protects freeBuffers_, doneQueue_, and freeQueue_ */

  uint64_t lastBufferTime_{};
  QElapsedTimer frameRateInterval_;
  uint32_t previousFrames_{};
  uint32_t framesCaptured_{};

  std::vector<std::unique_ptr<libcamera::Request>> requests_;
};

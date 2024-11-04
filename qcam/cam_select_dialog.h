/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022, Utkarsh Tiwari <utkarsh02t@gmail.com>
 *
 * qcam - Camera Selection dialog
 */

#pragma once

#include <QDialog>
#include <QString>
#include <string>

namespace libcamera {
class CameraManager;
}

class QComboBox;
class QLabel;

class CameraSelectorDialog : public QDialog {
  Q_OBJECT
public:
  CameraSelectorDialog(libcamera::CameraManager* cameraManager, QWidget* parent);
  ~CameraSelectorDialog() override;

  auto getCameraId() -> std::string;

  /* Hotplug / Unplug Support. */
  void addCamera(QString cameraId);
  void removeCamera(QString cameraId);

  /* Camera Information */
  void updateCameraInfo(QString cameraId);

private:
  libcamera::CameraManager* cm_{ nullptr };

  /* UI elements. */
  QComboBox* cameraIdComboBox_{ nullptr };
  QLabel* cameraLocation_{ nullptr };
  QLabel* cameraModel_{ nullptr };
};

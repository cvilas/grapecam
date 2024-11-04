/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022, Utkarsh Tiwari <utkarsh02t@gmail.com>
 *
 * qcam - Camera Selection dialog
 */

#include "cam_select_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QScreen>
#include <QString>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/controls.h>
#include <libcamera/property_ids.h>

CameraSelectorDialog::CameraSelectorDialog(libcamera::CameraManager* cameraManager, QWidget* parent)
  : QDialog(parent)
  , cm_(cameraManager)
  , cameraIdComboBox_(new QComboBox)
  , cameraLocation_(new QLabel)
  , cameraModel_(new QLabel) {
  // Use a QFormLayout for the dialog.
  auto* layout = new QFormLayout(this);

  // Setup the camera id combo-box.
  for (const auto& cam : cm_->cameras()) {
    cameraIdComboBox_->addItem(QString::fromStdString(cam->id()));
  }

  // Set camera information labels.
  updateCameraInfo(cameraIdComboBox_->currentText());
  connect(cameraIdComboBox_, &QComboBox::currentTextChanged, this,
          &CameraSelectorDialog::updateCameraInfo);

  // Setup the QDialogButton Box
  auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  // Set the layout.
  layout->addRow("Camera:", cameraIdComboBox_);
  layout->addRow("Location:", cameraLocation_);
  layout->addRow("Model:", cameraModel_);
  layout->addWidget(buttonBox);

  // Decrease the minimum width of dialog to fit on narrow screens, with a 20 pixels margin.
  auto screenGeometry = qGuiApp->primaryScreen()->availableGeometry();
  if (screenGeometry.width() < minimumWidth()) {
    setMinimumWidth(screenGeometry.width() - 20);
  }
}

CameraSelectorDialog::~CameraSelectorDialog() = default;

auto CameraSelectorDialog::getCameraId() -> std::string {
  return cameraIdComboBox_->currentText().toStdString();
}

// Hotplug / Unplug Support.
void CameraSelectorDialog::addCamera(QString cameraId) {
  cameraIdComboBox_->addItem(cameraId);
}

void CameraSelectorDialog::removeCamera(QString cameraId) {
  const auto cameraIndex = cameraIdComboBox_->findText(cameraId);
  cameraIdComboBox_->removeItem(cameraIndex);
}

// Camera Information
void CameraSelectorDialog::updateCameraInfo(QString cameraId) {
  const auto& camera = cm_->get(cameraId.toStdString());

  if (!camera) {
    return;
  }

  const auto& properties = camera->properties();

  const auto& location = properties.get(libcamera::properties::Location);
  if (location) {
    switch (*location) {
      case libcamera::properties::CameraLocationFront:
        cameraLocation_->setText("Internal front camera");
        break;
      case libcamera::properties::CameraLocationBack:
        cameraLocation_->setText("Internal back camera");
        break;
      case libcamera::properties::CameraLocationExternal:
        cameraLocation_->setText("External camera");
        break;
      default:
        cameraLocation_->setText("Unknown");
    }
  } else {
    cameraLocation_->setText("Unknown");
  }

  const auto& model = properties.get(libcamera::properties::Model).value_or("Unknown");

  cameraModel_->setText(QString::fromStdString(model));
}
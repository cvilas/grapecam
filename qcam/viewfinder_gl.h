/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Linaro
 *
 * OpenGL Viewfinder for rendering by OpenGL shader
 */

#pragma once

#include <QImage>
#include <QMutex>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QSize>
#include <array>
#include <memory>

#include <libcamera/formats.h>
#include <libcamera/framebuffer.h>

#include "viewfinder.h"

class ViewFinderGL : public QOpenGLWidget, public ViewFinder, protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit ViewFinderGL(QWidget* parent = nullptr);
  ~ViewFinderGL() override;

  auto nativeFormats() const -> const QList<libcamera::PixelFormat>& override;

  auto setFormat(const libcamera::PixelFormat& format, const QSize& size,
                 const libcamera::ColorSpace& colorSpace, unsigned int stride) -> int override;
  void render(libcamera::FrameBuffer* buffer, Image* image) override;
  void stop() override;

  auto getCurrentImage() -> QImage override;

Q_SIGNALS:
  void renderComplete(libcamera::FrameBuffer* buffer);

protected:
  void initializeGL() override;
  void paintGL() override;
  auto sizeHint() const -> QSize override;

private:
  auto selectFormat(const libcamera::PixelFormat& format) -> bool;
  void selectColorSpace(const libcamera::ColorSpace& colorSpace);

  void configureTexture(QOpenGLTexture& texture);
  auto createFragmentShader() -> bool;
  auto createVertexShader() -> bool;
  void removeShader();
  void doRender();

  /* Captured image size, format and buffer */
  libcamera::FrameBuffer* buffer_{ nullptr };
  libcamera::PixelFormat format_{};
  libcamera::ColorSpace colorSpace_;
  QSize size_{};
  unsigned int stride_{};
  Image* image_{ nullptr };

  /* Shaders */
  QOpenGLShaderProgram shaderProgram_;
  std::unique_ptr<QOpenGLShader> vertexShader_{ nullptr };
  std::unique_ptr<QOpenGLShader> fragmentShader_{ nullptr };
  QString vertexShaderFile_;
  QString fragmentShaderFile_;
  QStringList fragmentShaderDefines_;

  /* Vertex buffer */
  QOpenGLBuffer vertexBuffer_;

  /* Textures */
  std::array<std::unique_ptr<QOpenGLTexture>, 3> textures_;

  /* Common texture parameters */
  GLuint textureMinMagFilters_{};
  GLuint projMatrixUniform_{};

  /* YUV texture parameters */
  GLuint textureUniformU_{};
  GLuint textureUniformV_{};
  GLuint textureUniformY_{};
  GLuint textureUniformStep_{};
  unsigned int horzSubSample_{};
  unsigned int vertSubSample_{};

  /* Raw Bayer texture parameters */
  GLuint textureUniformSize_{};
  GLuint textureUniformStrideFactor_{};
  GLuint textureUniformBayerFirstRed_{};
  QPointF firstRed_{};

  QMutex mutex_; /* Prevent concurrent access to image_ */
};

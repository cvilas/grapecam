/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Ideas on Board Oy
 *
 * Multi-planar image with access to pixel data
 */

#include "image.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <map>
#include <print>
#include <type_traits>

#include <sys/mman.h>
#include <unistd.h>

namespace {
auto toUInt(Image::MapMode mode) -> unsigned int {
  return static_cast<unsigned int>(mode);
}
}  // namespace
auto Image::fromFrameBuffer(const libcamera::FrameBuffer* buffer, MapMode mode)
    -> std::unique_ptr<Image> {
  std::unique_ptr<Image> image{ new Image() };

  assert(not buffer->planes().empty());

  auto mmap_flags = 0u;

  if (toUInt(mode) & toUInt(MapMode::ReadOnly)) {
    mmap_flags |= PROT_READ;
  }

  if (toUInt(mode) & toUInt(MapMode::WriteOnly)) {
    mmap_flags |= PROT_WRITE;
  }

  struct MappedBufferInfo {
    uint8_t* address = nullptr;
    size_t map_length = 0;
    size_t dmabuf_length = 0;
  };
  std::map<int, MappedBufferInfo> mapped_buffers;

  for (const auto& plane : buffer->planes()) {
    const int fd = plane.fd.get();
    if (mapped_buffers.find(fd) == mapped_buffers.end()) {
      const size_t length = lseek(fd, 0, SEEK_END);
      mapped_buffers[fd] = { .address = nullptr, .map_length = 0, .dmabuf_length = length };
    }

    const size_t length = mapped_buffers[fd].dmabuf_length;

    if (plane.offset > length || plane.offset + plane.length > length) {
      std::println("plane is out of buffer: buffer length={}, plane offset={}, plane length={}",
                   length, plane.offset, plane.length);
      return nullptr;
    }
    auto& map_length = mapped_buffers[fd].map_length;
    map_length = std::max(map_length, static_cast<size_t>(plane.offset + plane.length));
  }

  for (const auto& plane : buffer->planes()) {
    const int fd = plane.fd.get();
    auto& info = mapped_buffers[fd];
    if (!info.address) {
      void* address =
          mmap(nullptr, info.map_length, static_cast<int>(mmap_flags), MAP_SHARED, fd, 0);
      if (address == MAP_FAILED) {
        const auto err = -errno;
        std::println("Failed to mmap plane: {}", strerror(-err));
        return nullptr;
      }

      info.address = static_cast<uint8_t*>(address);
      image->maps_.emplace_back(info.address, info.map_length);
    }

    image->planes_.emplace_back(info.address + plane.offset, plane.length);
  }

  return image;
}

Image::Image() = default;

Image::~Image() {
  for (const auto& map : maps_) {
    munmap(map.data(), map.size());
  }
}

auto Image::numPlanes() const -> unsigned int {
  return planes_.size();
}

auto Image::data(unsigned int plane) -> libcamera::Span<uint8_t> {
  assert(plane <= planes_.size());
  return planes_[plane];
}

auto Image::data(unsigned int plane) const -> libcamera::Span<const uint8_t> {
  assert(plane <= planes_.size());
  return planes_[plane];
}

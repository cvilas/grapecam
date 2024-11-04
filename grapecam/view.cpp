//=================================================================================================
// Copyright (C) 2024 grapecam contributors
//=================================================================================================

#include <condition_variable>
#include <mutex>
#include <print>
#include <queue>
#include <stdexcept>

#include <SDL3/SDL.h>
#include <libcamera/libcamera.h>
#include <libcamera/property_ids.h>
#include <libcamera/request.h>
#include <sys/mman.h>

//=================================================================================================
// Encapsulating capture-view class
class CameraViewer {
public:
  CameraViewer();
  ~CameraViewer();
  void run();

  CameraViewer(const CameraViewer&) = delete;
  auto operator=(const CameraViewer&) -> CameraViewer& = delete;
  CameraViewer(CameraViewer&&) = delete;
  auto operator=(CameraViewer&&) -> CameraViewer& = delete;

private:
  auto mapBuffer(libcamera::FrameBuffer* buffer) -> void*;
  void unmapBuffer(void* memory, size_t length);
  void requestComplete(libcamera::Request* request);
  void handleEvents();
  void display();

  static constexpr auto WINDOW_WIDTH = 640U;
  static constexpr auto WINDOW_HEIGHT = 480U;
  std::vector<void*> mapped_buffers_;
  std::queue<libcamera::Request*> completed_requests_;
  std::mutex mutex_;
  std::condition_variable condition_;

  std::shared_ptr<libcamera::CameraManager> camera_manager_;
  std::shared_ptr<libcamera::Camera> camera_;
  std::shared_ptr<libcamera::CameraConfiguration> camera_config_;

  SDL_Window* window_{ nullptr };
  SDL_Renderer* renderer_{ nullptr };
  SDL_Texture* texture_{ nullptr };
  bool running_{ true };
};

//-------------------------------------------------------------------------------------------------
CameraViewer::CameraViewer() {
  // start camera manager
  camera_manager_ = std::make_shared<libcamera::CameraManager>();
  if (camera_manager_->start() != 0) {
    throw std::runtime_error("Failed to start camera manager");
  }

  // enumerate cameras
  auto cameras = camera_manager_->cameras();
  if (cameras.empty()) {
    throw std::runtime_error("No cameras available");
  }

  // pick the first camera
  camera_ = cameras.at(0);
  const auto& props = camera_->properties();
  const auto& model = props.get(libcamera::properties::Model);
  std::println("{} ({})", (model ? *model : "NoName"), camera_->id());
  if (camera_->acquire() != 0) {
    throw std::runtime_error("Failed to acquire camera");
  }

  // configure the camera viewfinder stream
  camera_config_ = camera_->generateConfiguration({ libcamera::StreamRole::Viewfinder });
  auto& stream_config = camera_config_->at(0);
  stream_config.size = { WINDOW_WIDTH, WINDOW_HEIGHT };
  stream_config.pixelFormat = libcamera::formats::RGB888;
  if (camera_config_->validate() == libcamera::CameraConfiguration::Status::Invalid) {
    throw std::runtime_error("Camera configuration invalid");
  }
  if (camera_->configure(camera_config_.get())) {
    throw std::runtime_error("Failed to configure camera");
  }

  // initialize SDL video backend
  if (not SDL_Init(SDL_INIT_VIDEO)) {
    throw std::runtime_error("SDL initialization failed");
  }

  const auto window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
  window_ = SDL_CreateWindow("Camera Viewer", WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
  if (window_ == nullptr) {
    throw std::runtime_error("Failed to create window");
  }

  renderer_ = SDL_CreateRenderer(window_, nullptr);
  if (renderer_ == nullptr) {
    throw std::runtime_error("Failed to create renderer");
  }
  SDL_SetRenderVSync(renderer_, 1);
  SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window_);

  texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
                               WINDOW_WIDTH, WINDOW_HEIGHT);
  if (texture_ == nullptr) {
    throw std::runtime_error("Failed to create texture");
  }
}

//-------------------------------------------------------------------------------------------------
CameraViewer::~CameraViewer() {
  // for (size_t i = 0; i < mapped_buffers_.size(); ++i) {
  //   unmapBuffer(mapped_buffers_[i], buffers[i]->planes().front().length);
  // }
  if (camera_) {
    camera_->release();
  }
  if (camera_manager_) {
    camera_manager_->stop();
  }

  if (texture_) {
    SDL_DestroyTexture(texture_);
  }
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
  }
  if (window_) {
    SDL_DestroyWindow(window_);
  }
  SDL_Quit();
}

//-------------------------------------------------------------------------------------------------
auto CameraViewer::mapBuffer(libcamera::FrameBuffer* buffer) -> void* {
  if (buffer->planes().empty())
    return nullptr;

  const libcamera::FrameBuffer::Plane& plane = buffer->planes().front();
  void* memory = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);

  if (memory == MAP_FAILED)
    return nullptr;

  return memory;
}

//-------------------------------------------------------------------------------------------------
void CameraViewer::unmapBuffer(void* memory, size_t length) {
  if (memory)
    munmap(memory, length);
}

//-------------------------------------------------------------------------------------------------
void CameraViewer::run() {
  // Allocate framebuffers
  auto allocator = std::make_unique<libcamera::FrameBufferAllocator>(camera_);
  for (auto& cfg : *camera_config_) {
    if (allocator->allocate(cfg.stream()) < 0) {
      throw std::runtime_error("Can't allocate buffers");
    }
    auto num_allocated = allocator->buffers(cfg.stream()).size();
    std::println("Allocated {} buffers for stream", num_allocated);
  }

  auto* stream = camera_config_->at(0).stream();
  const auto& buffers = allocator->buffers(stream);

  for (const auto& buffer : buffers) {
    void* memory = mapBuffer(buffer.get());
    if (!memory) {
      throw std::runtime_error("Failed to map buffer memory");
    }
    mapped_buffers_.push_back(memory);
  }

  // create frame capture requests
  std::vector<std::unique_ptr<libcamera::Request>> requests;
  for (auto& buffer : buffers) {
    auto request = camera_->createRequest();
    if (!request) {
      throw std::runtime_error("Can't create request");
    }
    if (request->addBuffer(stream, buffer.get()) < 0) {
      throw std::runtime_error("Can't set buffer for request");
    }
    // Controls can be added to a request on a per frame basis.
    // auto& controls = request->controls();
    // constexpr auto BRIGHTNESS = 0.5;
    // controls.set(libcamera::controls::Brightness, BRIGHTNESS);
    requests.push_back(std::move(request));
  }

  // set request completion handler
  camera_->requestCompleted.connect(this, &CameraViewer::requestComplete);

  // Start capture by queueing requests
  if (camera_->start()) {
    throw std::runtime_error("Failed to start camera");
  }
  for (auto& request : requests) {
    camera_->queueRequest(request.get());
  }
  while (running_) {
    handleEvents();
    display();
  }
  camera_->stop();
}

//-------------------------------------------------------------------------------------------------
void CameraViewer::handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      running_ = false;
    }
  }
}

//-------------------------------------------------------------------------------------------------
void CameraViewer::requestComplete(libcamera::Request* request) {
  // This happens in the camera thread. Do as little as possible
  std::unique_lock lock(mutex_);
  completed_requests_.push(request);
  condition_.notify_one();
}

//-------------------------------------------------------------------------------------------------
void CameraViewer::display() {
  std::unique_lock<std::mutex> lock(mutex_);
  condition_.wait(lock, [this] { return !completed_requests_.empty(); });
  auto* completed_request = completed_requests_.front();
  completed_requests_.pop();
  lock.unlock();

  if (completed_request->status() == libcamera::Request::Status::RequestComplete) {
    const libcamera::FrameBuffer* fb = completed_request->buffers().begin()->second;
    size_t buffer_index = 0;  // You'll need to track which buffer this is

    // The memory is already mapped in mappedBuffers
    const void* buffer_data = mapped_buffers_[buffer_index];
    size_t length = fb->planes().front().length;

    // Update texture with new frame data
    void* pixels = nullptr;
    int pitch = 0;
    SDL_LockTexture(texture_, nullptr, &pixels, &pitch);
    memcpy(pixels, buffer_data, length);
    SDL_UnlockTexture(texture_);

    // Render the texture
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);

    // Re-queue the request
    completed_request->reuse();
    camera_->queueRequest(completed_request);
  }
}

//=================================================================================================
auto main() -> int {
  try {
    CameraViewer viewer;
    viewer.run();
    return EXIT_SUCCESS;
  } catch (const std::exception& e) {
    std::ignore = std::fputs(e.what(), stderr);
    return EXIT_FAILURE;
  }
}
# grapecam

[GRAPE](https://github.com/cvilas/grape) camera application

## Requirements

- Supports standalone camera system built around RaspberryPi HQ camera
- Statically configurable device selection
- Remotely configurable exposure settings
- Reports sequence number and timestamp
- GLFW + imgui + OpenGL based visualiser (rationale: simplicity, compared to Qt)
- (Optional) Implements audio streaming support

## Hardware

- [Raspberry Pi 5](https://thepihut.com/products/raspberry-pi-5)
- [High Quality camera module](https://thepihut.com/products/raspberry-pi-high-quality-camera-module)
- [Camera mounting plate](https://thepihut.com/products/mounting-plate-for-high-quality-camera)
- (optional) [Mic](https://thepihut.com/products/mini-usb-microphone)
- (optional) [PoE+ HAT](https://thepihut.com/products/uctronics-poe-hat-for-raspberry-pi-5-with-active-cooler-802-3af-at)

## Software build instructions

- Install dependencies

  ```bash
  sudo apt install pkg-config libevent-dev libudev-dev qt6-base-dev qt6-tools-dev-tools
  ```

- libcamera

  - On the RaspberryPi, build libcamera from sources as described in [raspberrypi/libcamera](https://github.com/raspberrypi/libcamera)
  - On all other hosts, install libcamera with: `sudo apt install libcamera-dev libcamera-tools`

- Build grapecam

  ```bash
  git clone git@github.com:cvilas/grapecam.git
  mkdir -p grapecam/build
  cd grapecam/build

  cmake .. 
  make -j8
  ```

## TODO

- :done: Test hello on rp4 (bookworm) and rp5 (ubuntu + webcam)
- :done: Study qcam Qt viewer to understand how it works, both Qt and GLES
- :done: Implement example program to list cameras
- Implement GLFW-based viewer PoC using qcam-gl as the basis
- Implement publisher
  - Open default camera with default configuration
  - Publish image topic
- Implement subscriber
  - Listen to topic specified on command line
  - Convert native format to display format
  - View using GLFW + OpenGL
  - Benchmark against `qcam -r gles` for CPU vs GPU usage
- Make publisher configurable
  - Specify camera settings in config file
  - Add attachments to image topic: sequence number, timestamp, encoding format
- Integrate libcamera build [using cmake](https://github.com/christianrauch/libcamera_cmake)
  - Resolve: Upstream libcamera does not work on rpi, and rpi fork does not work on other platforms

## References

- [zcam](https://github.com/eclipse-zenoh/zenoh-demos/tree/master/computer-vision/zcam)

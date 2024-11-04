# grapecam

[GRAPE](https://github.com/cvilas/grape) camera application

## Requirements

- Supports standalone camera system built around RaspberryPi HQ camera
- Statically configurable device selection
- Remotely configurable exposure settings
- Reports sequence number and timestamp
- SDL3 based visualiser
- (Optional) Implements audio streaming support

## Hardware

- [Raspberry Pi 5](https://thepihut.com/products/raspberry-pi-5)
- [High Quality camera module](https://thepihut.com/products/raspberry-pi-high-quality-camera-module)
- [Camera mounting plate](https://thepihut.com/products/mounting-plate-for-high-quality-camera)
- (optional) [Mic](https://thepihut.com/products/mini-usb-microphone)
- (optional) [PoE+ HAT](https://thepihut.com/products/uctronics-poe-hat-for-raspberry-pi-5-with-active-cooler-802-3af-at)

## Software build instructions

- Build and install libcamera

  - On the RaspberryPi, build libcamera following [raspberrypi/libcamera](https://github.com/raspberrypi/libcamera)
  - On all other hosts, build libcamera following [getting started guide](https://libcamera.org/getting-started.html)

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
- Implement SDL3-based viewer PoC. References:
  - qcam for libcamera configuration, framebuffer mapping, pixel format conversions
  - SDL camera [example](https://examples.libsdl.org/SDL3/camera/01-read-and-draw/) for SDL texture settings and display update
- Test SDL3-based viewer on raspberry pi
- Benchmark SDL3-based viewer against `qcam -r gles` for GPU usage
- Remove qcam and qt6 dependencies
- Implement publisher
  - Open default camera
  - Publish image topic
- Implement subscriber
  - Listen to topic specified on command line
  - Convert native format to display format
  - View using SDL3
- Make publisher configurable
  - Specify camera settings in config file
  - Add attachments to image topic: sequence number, timestamp, encoding details, camera id
- Integrate libcamera build [using cmake](https://github.com/christianrauch/libcamera_cmake)
  - Resolve: Upstream libcamera does not work on rpi, and rpi fork does not work on other platforms

## References

- [zcam](https://github.com/eclipse-zenoh/zenoh-demos/tree/master/computer-vision/zcam)

# ASL_Converter
Finals Project for CPS 112 - Machine Perception and Robotics

## Goal
We want to help the mighty people that are verbally-challenged by developing a prototype that will transform sign languages, ASL in this case, into a format more easily understood by people who aren't familiar with sign language/s.

This prototype uses an ESP32S3 on station mode along with a SSD1306 128x64 OLED Display.
A Python script utilizing OpenCV will connect to the ESP32S3 for sending the detected ASL gestures.

The received letter detected will then be converted into messages/phrases commonly used by cashiers to simulate a transactional conversation between them and customers.

## Setup
Make sure to use Python 3.11.14 on a virtual environment

## Packages Used
### Python
* mediapipe
* numpy
* pyserial
* opencv-python
* scikit-learn
### Platformio Libdeps (on ESP32S3 Zero)
* adafruit/Adafruit SSD1306@^2.5.15

## Relevant Links
* https://github.com/rickkas7/SSD1306-tutorial?tab=readme-ov-file
* https://rickkas7.github.io/DisplayGenerator/

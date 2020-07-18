# TrailCam

A remote cellular enabled camera meant to take photos of wildlife

## Hardware Requirements

 - Hologram Dash
 - an [ArduCAM](https://www.arducam.com/product-category/spi-camera-for-arduino/) camera
 - Battery
 - Enclosure

## Software requirements

 - [My fork of the dash firmware](https://github.com/DomAmato/dash-system-firmware)
 - [My fork of the dash aurdino integration](https://github.com/DomAmato/hologram-dash-arduino-integration)
 - [My fork of the arducam library](https://github.com/DomAmato/ArduCam)

## Setup the Arduino Environment

You still need the dash uploader tool so install everything like you normally would for the dash. In the hardware folder it should install a hologram folder with the dash information inside. That installation is the user program space on the dash that handles all the calls you make from the arduino program you write. I change the name of that folder to `dash-user` and then created a folder called `dash-system` and copied over the system firmware files. I also replaced the user system files with the one from my fork as it adds some extra features that are required to make this work.

The changes I made to the arducam library aren't functional ones but more due to its overall lack of organization and lack of polymorphism. 

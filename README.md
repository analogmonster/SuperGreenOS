![SuperGreenLab](assets/sgl.png?raw=true "SuperGreenLab")

# Table of Contents

   * [SuperGreenOS](#supergreenos)
      * [Who is this document for](#who-is-this-document-for)
      * [Features](#features)
   * [Quickstart](#quickstart)
      * [Workspace setup](#workspace-setup)
         * [Esp-idf setup](#esp-idf-setup)
         * [Clone repo, build and run](#clone-repo-build-and-run)
         * [How to use](#how-to-use)


![WeedAppPic](assets/weedapppic.png?raw=true "WeedAppPic")

# SuperGreenOS

SuperGreenOS provides most features used when growing cannabis, all in one package, and controllable from your smartphone, pc, mac, linux, toaster, plumbus, whatnot...

It is the official firmware for the [SuperGreenController](https://github.com/supergreenlab/SuperGreenController).

## Who is this document for

This document is for developpers that want to start playing with there controller's internal stuffs, or just setup their own hardware.
This repository is based on [SuperGreenOSBoilerplate](https://github.com/supergreenlab/SuperGreenOSBoilerplate), please read the doc here first.

## Features

Here's what it can (or will) do:

- Lights on and off schedules
- Up to 6 separate led channels (you can put multiple leds behind one channel)
- Up to 3 separate timers, for full-cycle setups (veg + flo)
- Monitoring a wide range of sensors
- Data sent to a MQTT server
- Produce alerts based on sensor values
- Allows remote control
- Manual ventilation control
- Automatic ventilation control based on temperature and humidity
- `Stretch` mode, allows to choose how much you want your plant to stretch or thicken
- `Sunglass` mode, so you don't burn your eyes when you work on your plants
- More to come..

This is the firmware that runs the [SuperGreenController](https://github.com/supergreenlab/SuperGreenController).

# Workspace setup

Included in this repo is a docker setup with all the required pre-requisites to build the firmware.  You just need to create a build.env file - see the example in the repo.

## Clone repo, build and run

Now you should be able to clone and build the firmware:

```

# Optional: fix permissions if your workspace was mounted with root ownership
sudo chown -R devuser:devuser /workspace
# CAUTION! If your devcontainer is mounting your local copy of this repo to /workspace (the default behavior), 
# running git clone inside the container will overwrite the host copy. 
# Only do this if /workspace is currently empty or not a copy of the branch you want to build from.
git clone https://github.com/supergreenlab/SuperGreenOS.git
cd SuperGreenOS

```

The below will build the controller firmware.  To build for another variant, replace config.controller.json with config.<variant_name>.json

```

./update_templates.sh config.controller.json
./update_htmlapp.sh config.controller.json
make -j4

```

Next, plug in your [SuperGreenController](https://github.com/supergreenlab/SuperGreenController) or any supported esp32 based board and run the commands:

```

make -j4 flash monitor
./write_spiffs.sh

```

The first command flashes the firmware, the second writes the embedded admin interface on the tiny file system (~20KB available).

# How to use

Once the firmware is flashed you can access the controller's wifi network, once connected go to http://192.168.4.1/fs/app.html,
this will display the html embedded admin interface, which allows you to easily modify any of the controller's parameters.

![Admin](assets/admin.png?raw=true "Admin")
# Doom-MCX port of popular Doom game to run on NXP MCX microcontrollers

A port of prBoom to NXP MCXN MCU running Zephyr RTOS

![MCX-Doom](https://raw.githubusercontent.com/NXPHoverGames/Doom-MCX/refs/heads/main/assets/doom_mcx.jpg)

<!----- Boards ----->
[![License badge](https://img.shields.io/badge/License-GPL%202.0-red)](https://github.com/search?q=org%3Anxp-appcodehub+vision+in%3Areadme&type=Repositories)
![Language badge](https://img.shields.io/badge/Language-C-yellow)
[![Board badge](https://img.shields.io/badge/Board-FRDM&ndash;MCXN947-blue)]()
[![Category badge](https://img.shields.io/badge/Category-RTOS-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+rtos+in%3Areadme&type=Repositories)
[![Category badge](https://img.shields.io/badge/Category-HMI-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+hmi+in%3Areadme&type=Repositories)
[![Category badge](https://img.shields.io/badge/Category-GRAPHICS-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+graphics+in%3Areadme&type=Repositories)

![Demo](https://raw.githubusercontent.com/NXPHoverGames/Doom-MCX/refs/heads/main/assets/demo.gif)

## Table of Contents
1. [Software](#step1)
2. [Hardware](#step2)
3. [Setup](#step3)
4. [Results](#step4)
5. [FAQs](#step5) 
6. [Support](#step6)
7. [Release Notes](#step7)

## 1. Software<a name="step1"></a>
- Zephyr 4.1.0
- Zephyr SDK version 0.17.0

## 2. Hardware<a name="step2"></a>

![FRDM-MCXN947](https://raw.githubusercontent.com/NXPHoverGames/Doom-MCX/refs/heads/main/assets/FRDM-MCXN947-TOP.avif) | ![LCD-PAR-S035](https://raw.githubusercontent.com/NXPHoverGames/Doom-MCX/refs/heads/main/assets/LCD-PAR-S035-TOP.avif)
:-------------------------:|:-------------------------:
[FRDM Development Board for MCX N947](https://www.nxp.com/design/design-center/development-boards-and-designs/general-purpose-mcus/frdm-development-board-for-mcx-n94-n54-mcus:FRDM-MCXN947)  |  [LCD-PAR-S035 480x320 IPS LCD](https://www.nxp.com/design/design-center/development-boards-and-designs/general-purpose-mcus/3-5-480x320-ips-tft-lcd-module:LCD-PAR-S035)

Optional [Joystick shield](https://duckduckgo.com/?t=h_&q=funduino+board+joystick+shield&ia=web) on FRDM-MCXN947 Arduino Header



## 3. Setup<a name="step3"></a>


Before getting started, make sure you have a proper Zephyr development
environment. Follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

> [!NOTE]  
> DOOM-MCX has been tested on Zephyr SDK version 0.17.0. When installing the sdk it's recommended to specify the version using `west sdk install --version 0.17.0`

### Initialization

The first step is to initialize the DOOM-MCX Zephyr workspace folder (``doom-mcx-workspace``) where ``doom-mcx`` and all Zephyr modules will be cloned. Run the following
command:

```shell
# initialize  for the example-application (main branch)
west init -m https://github.com/nxphovergames/doom-mcx --mr main doom-mcx-workspace
# update Zephyr modules
cd doom-mcx-workspace
west update
```

### Building

To build the application, run the following command:

```shell
cd doom-mcx
west build -p always -b frdm_mcxn947/mcxn947/cpu0
```

### Flashing

> [!NOTE]  
> [NXP Linkserver](https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/linkserver-for-microcontrollers:LINKERSERVER) v1.5.30 or newer has to be installed. See [Zephyr LinkServer guide](https://docs.zephyrproject.org/latest/develop/flash_debug/host-tools.html#linkserver-debug-host-tools) for more information.

To flash the application to the FRDM-MCXN947 board. Connect a USB-C cable to the "MCU-Link" USB-C port and run the following command:

```shell
west flash
```


## 4. Results<a name="step4"></a>
On a successful flash the display (LCD-PAR-S035) should turn and you can start playing the game.

## 5. FAQs<a name="step5"></a>

### Doom-MCX Controls

Doom-MCX supports 2 types of control either through touchscreen or the [Joystick shield](https://duckduckgo.com/?t=h_&q=funduino+board+joystick+shield&ia=web).

#### Touchscreen controls
**Open doors/start:** Tap "Use Area"  
**Fire:** Tap "Fire Area"  
**Wake & Strafe** Tap and drag in the "Virtual Joystick Area"

![Doom-MXC Touch screen layout](https://raw.githubusercontent.com/NXPHoverGames/Doom-MCX/refs/heads/main/assets/doom_mcx_touch_screen_layout.jpg)

> [!CAUTION]  
> When using the Funduino shield make sure the switch is in 3V3 mode, otherwise you will damage the board.

#### Joytick Shield controls
**Fire:** D  
**Use:** C  
**Walk:** Joystick Y-axis  
**Strafe:** Joystick X-axis  
**Menu:** A

## 6. Support<a name="step6"></a>

Questions regarding the content/correctness of this example can be entered as Issues within this GitHub repository.

>**Warning**: For more general technical questions regarding NXP Microcontrollers and the difference in expected functionality, enter your questions on the [NXP Community Forum](https://community.nxp.com/)

[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/NXP_Semiconductors)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/nxp-semiconductors)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/nxpsemi/)
[![Follow us on Twitter](https://img.shields.io/badge/X-Follow%20us%20on%20X-black.svg)](https://x.com/NXP)


# Simulation on PC

Zephyr RTOS also provides a native_posix target

To compile as native type:
```shell
west build -p always -b native_posix_64
```

To run type
```shell
cd doom-mcx
./build/zephyr/zephyr.elf
```

### native_posix controls
**Fire:** CTRL  
**Use:** Spacebar  
**Walk:** Up-arrow & Down-arrow  
**Strafe:** Left-arrow & Right-arrow  
**Menu:** ESC

![MCX-Doom Native](https://raw.githubusercontent.com/NXPHoverGames/Doom-MCX/refs/heads/main/assets/mcx_doom_zephyr_native_posix.png)

## 7. Release Notes<a name="step7"></a>
| Version | Description / Update                           | Date                        |
|:-------:|------------------------------------------------|----------------------------:|
| 1.0     | Initial release                                | September 9<sup>th</sup> 2024 |
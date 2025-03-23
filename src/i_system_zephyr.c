/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright 2024 NXP
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 *-----------------------------------------------------------------------------
 */

#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "d_event.h"
#include "d_main.h"
#include "g_game.h"
#include "i_system_zephyr.h"

#include "doomdef.h"
#include "lprintf.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sample, LOG_LEVEL_INF);
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>

#ifdef CONFIG_DOOM_SHOW_NXP_LOGO
#include "nxp_80x32.h"
#endif

#define RIGHT_NODE DT_ALIAS(right)
#define LEFT_NODE DT_ALIAS(left)
#define DOWN_NODE DT_ALIAS(down)
#define UP_NODE DT_ALIAS(up)
#define FIRE_NODE DT_ALIAS(fire)
#define ENTER_NODE DT_ALIAS(enter)
#define MENU_NODE DT_ALIAS(menu)
#define WEAPONTOGGLE_NODE DT_ALIAS(weapontoggle)
#define STRAFE_NODE DT_ALIAS(strafe)
#define RUN_NODE DT_ALIAS(run)
#define MAP_NODE DT_ALIAS(map)
#define TOGGLE_NODE DT_ALIAS(toggle)

#define DISPLAY_NODE DT_CHOSEN(zephyr_display)

#define STACKSIZE 2000
struct k_thread display_thread;
K_THREAD_STACK_DEFINE(display_stack, STACKSIZE);
struct display_capabilities d_capabilities;
struct k_sem sem_display;
struct k_sem sem_renderer;

//**************************************************************************************

struct key_mapping {
  struct gpio_dt_spec node;
  const int *key;
  evtype_t eventstate;
};

struct key_mapping keymap[] = {
#ifndef CONFIG_DOOM_ZEPHYR_ADC_JOYSTICK
    {GPIO_DT_SPEC_GET_OR(UP_NODE, gpios, {0}), &key_up, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(DOWN_NODE, gpios, {0}), &key_down, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(LEFT_NODE, gpios, {0}), &key_left, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(RIGHT_NODE, gpios, {0}), &key_right, ev_keyup},
#endif
    {GPIO_DT_SPEC_GET_OR(FIRE_NODE, gpios, {0}), &key_fire, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(ENTER_NODE, gpios, {0}), &key_enter, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(ENTER_NODE, gpios, {0}), &key_use, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(MENU_NODE, gpios, {0}), &key_escape, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(RUN_NODE, gpios, {0}), &key_speed, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(MAP_NODE, gpios, {0}), &key_map, ev_keyup},
    {GPIO_DT_SPEC_GET_OR(TOGGLE_NODE, gpios, {0}), &key_toggle, ev_keyup},
};

#define KEYMAP_SIZE (sizeof(keymap) / sizeof(keymap[0]))

#ifdef CONFIG_DOOM_ZEPHYR_ADC_JOYSTICK
static const struct device *const adc_joy_dev =
    DEVICE_DT_GET(DT_NODELABEL(adc_joystick));
#endif
#ifdef CONFIG_DOOM_ZEPHYR_TOUCH_SCREEN
static const struct device *const touch_screen_dev =
    DEVICE_DT_GET(DT_ALIAS(touch_screen));
#endif

bool x_l_pressed = false;
bool x_r_pressed = false;
bool x_center = false;

bool y_u_pressed = false;
bool y_d_pressed = false;
bool y_center = false;
bool action_pressed = false;
bool action_fire = false;

// TOD IFDEF TOUCH
uint16_t touch_initial_x = 0;
uint16_t touch_initial_y = 0;

uint16_t touch_x = 0;
uint16_t touch_y = 0;
bool touch_btn = false;
bool touch_released = true;

unsigned int vid_width = 0;
unsigned int vid_height = 0;

FAST_DATA_RAM unsigned short
    backbuffer[(SCREENWIDTH * SCREENHEIGHT) + CONFIG_DOOM_X_RES];

#ifndef CONFIG_DOOM_NO_WIPE
FAST_DATA_RAM unsigned short frontbuffer[SCREENWIDTH * SCREENHEIGHT];
#endif

#if defined(CONFIG_DOOM_RGB565) || defined(CONFIG_DOOM_BGR565)
static uint16_t pl_565[256];
const byte *pl_game = (byte *)pl_565;
#else
const byte *pl_game;
#endif

const struct device *display_dev;

//**************************************************************************************

void I_uSleep(unsigned long usecs) { k_sleep(K_USEC(usecs)); }

//**************************************************************************************

void Z_DisplayThreadEntry(void) {
  struct display_buffer_descriptor buf_desc;

  buf_desc.buf_size = CONFIG_DOOM_X_RES * CONFIG_DOOM_Y_RES;
  buf_desc.pitch = CONFIG_DOOM_X_RES;
  buf_desc.width = CONFIG_DOOM_X_RES;
  buf_desc.height = 1;

  int i, j;

  uint16_t *framebuffer = (uint16_t *)&backbuffer[0];
  unsigned char *frame;

  while (1) {
    k_sem_take(&sem_renderer, K_FOREVER);
    frame = (char *)I_GetBackBuffer();
    int lineheight;
    int y_offset = 0;

    /* Re-use Backbuffer while converting indexed color (8-bit) to 565 (16-bit)
     */
    for (i = 1; y_offset < CONFIG_DOOM_Y_RES; i++) {
      lineheight = i + (1 << (i - 1));

      if (lineheight + y_offset >= CONFIG_DOOM_Y_RES) {
        lineheight = CONFIG_DOOM_Y_RES - y_offset;
      }

      for (j = 0; j < CONFIG_DOOM_X_RES * lineheight; j++) {
        framebuffer[j] = pl_565[frame[j]];
      }

      frame += CONFIG_DOOM_X_RES * lineheight;
      buf_desc.height = lineheight;

      display_write(display_dev, 0, y_offset, &buf_desc, framebuffer);

      y_offset += lineheight;
    }
    k_sem_give(&sem_display);
  }
}

//**************************************************************************************

void I_InitScreen_zephyr() {
  printf("I_CreateWindow_e32\n");

  display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  if (!device_is_ready(display_dev)) {
    LOG_ERR("Device %s not found. Aborting sample.", display_dev->name);
    return;
  }

  LOG_INF("Display sample for %s", display_dev->name);
  display_get_capabilities(display_dev, &d_capabilities);
  LOG_INF("Display %ix%i", d_capabilities.x_resolution,
          d_capabilities.y_resolution);

#ifdef CONFIG_DOOM_RGB888
  display_set_pixel_format(display_dev, PIXEL_FORMAT_RGB_888); // TODO
#elif defined(CONFIG_DOOM_RGB565)
  display_set_pixel_format(display_dev, PIXEL_FORMAT_RGB_565);
#elif defined(CONFIG_DOOM_BGR565)
  display_set_pixel_format(display_dev, PIXEL_FORMAT_BGR_565);
#endif
  display_blanking_off(display_dev);

#ifdef CONFIG_DOOM_SHOW_NXP_LOGO
  struct display_buffer_descriptor buf_desc;

  buf_desc.buf_size = 80 * 320 * 2;
  buf_desc.pitch = 80;
  buf_desc.width = 80;
  buf_desc.height = 320;
  for (int i = 0; i < 6; i++) {
    display_write(display_dev, 80 * i, 0, &buf_desc, nxp_80_320);
  }

#endif

  k_sem_init(&sem_display, 0, K_SEM_MAX_LIMIT);
  k_sem_init(&sem_renderer, 0, K_SEM_MAX_LIMIT);

  /* Start display thread*/
  k_thread_create(&display_thread, display_stack, STACKSIZE,
                  (k_thread_entry_t)Z_DisplayThreadEntry, NULL, NULL, NULL,
                  K_PRIO_COOP(7), 0, K_NO_WAIT);

  vid_width = SCREENWIDTH;
  vid_height = SCREENHEIGHT;

  for (int i = 0; i < KEYMAP_SIZE; i++) {
    if (keymap[i].node.port == 0) {
      printk("Error: key %ls is not configured\n", keymap[i].key);
    } else {
      gpio_pin_configure_dt(&keymap[i].node, GPIO_INPUT);
    }
  }
}

//**************************************************************************************

void I_BlitScreenBmp_e32() {}

//**************************************************************************************

void I_StartWServEvents_e32() {}

//**************************************************************************************

#define ADC_CENTER 2000
#define ADC_THRESH 600
#define ADC_RELEASE_TRESH 400

void I_PollWServEvents_e32() {
  event_t event = {0};

  if (x_center) {
    event.type = ev_keyup;
    event.data1 = key_left;
    D_PostEvent(&event);
    event.type = ev_keyup;
    event.data1 = key_right;
    D_PostEvent(&event);
  } else if (x_l_pressed) {
    event.type = ev_keydown;
    event.data1 = key_left;
    D_PostEvent(&event);
  } else if (x_r_pressed) {
    event.type = ev_keydown;
    event.data1 = key_right;
    D_PostEvent(&event);
  }

  x_l_pressed = false;
  x_r_pressed = false;
  x_center = false;

  if (y_center) {
    event.type = ev_keyup;
    event.data1 = key_up;
    D_PostEvent(&event);
    event.type = ev_keyup;
    event.data1 = key_down;
    D_PostEvent(&event);
  } else if (y_u_pressed) {
    event.type = ev_keydown;
    event.data1 = key_up;
    D_PostEvent(&event);
  } else if (y_d_pressed) {
    event.type = ev_keydown;
    event.data1 = key_down;
    D_PostEvent(&event);
  }

  y_u_pressed = false;
  y_d_pressed = false;
  y_center = false;

  if (action_pressed && action_fire) {
    event.type = ev_keydown;
    event.data1 = key_fire;
    D_PostEvent(&event);
    action_pressed = false;
  } else if (action_pressed && !action_fire) {
    event.type = ev_keydown;
    event.data1 = key_enter;
    D_PostEvent(&event);
    action_pressed = false;
  } else {
    event.type = ev_keyup;
    event.data1 = key_fire;
    D_PostEvent(&event);
    event.type = ev_keyup;
    event.data1 = key_enter;
    D_PostEvent(&event);
  }

  for (int i = 0; i < KEYMAP_SIZE; i++) {
    if (keymap[i].node.port != 0) {
      if (gpio_pin_get_dt(&keymap[i].node) > 0 &&
          keymap[i].eventstate == ev_keyup) {
        keymap[i].eventstate = ev_keydown;
        event.type = ev_keydown;
        event.data1 = *keymap[i].key;
        D_PostEvent(&event);
      } else if (gpio_pin_get_dt(&keymap[i].node) == 0 &&
                 keymap[i].eventstate == ev_keydown) {
        keymap[i].eventstate = ev_keyup;
        event.type = ev_keyup;
        event.data1 = *keymap[i].key;
        D_PostEvent(&event);
      }
    }
  }
}

#ifdef CONFIG_DOOM_ZEPHYR_ADC_JOYSTICK
static void input_adc_joy_cb(struct input_event *evt, void *user_data) {

  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_X) {
    if (evt->value == -1) {
      x_l_pressed = true;
    } else if (evt->value == 0) {
      x_center = true;
    } else if (evt->value == 1) {
      x_r_pressed = true;
    }
  }

  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_Y) {
    if (evt->value == -1) {
      y_d_pressed = true;
    } else if (evt->value == 0) {
      y_center = true;
    } else if (evt->value == 1) {
      y_u_pressed = true;
    }
  }
}

INPUT_CALLBACK_DEFINE(adc_joy_dev, input_adc_joy_cb, NULL);
#endif

#ifdef CONFIG_DOOM_ZEPHYR_TOUCH_SCREEN
static void input_touch_cb(struct input_event *evt, void *user_data) {

#ifdef CONFIG_DOOM_ZEPHYR_TOUCH_SCREEN_SWAP_XY
  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_X) {
    touch_y = evt->value;
  }

#ifdef CONFIG_DOOM_ZEPHYR_TOUCH_SCREEN_INVERT_X
  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_Y) {
    touch_x = d_capabilities.x_resolution - evt->value;
  }
#else
  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_Y) {
    touch_x = evt->value;
  }
#endif
#else
  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_X) {
    touch_x = evt->value;
  }

  if (evt->type == INPUT_EV_ABS && evt->code == INPUT_ABS_Y) {
    touch_y = evt->value;
  }
#endif
  if (evt->type == INPUT_EV_KEY && evt->code == INPUT_BTN_TOUCH) {
    touch_btn = evt->value;
  }

  if (evt->sync) {

    if (!touch_btn) {
      touch_released = true;

      x_center = true;
      y_center = true;
      // STOP MOVING
    } else if (touch_y >= CONFIG_DOOM_X_RES) {
      x_center = true;
      y_center = true;

      if (touch_btn && touch_released) {
        action_pressed = true;
        action_fire = touch_x < (CONFIG_DOOM_Y_RES / 2);
      }
    } else if (touch_btn && touch_released) {
      touch_initial_x = touch_x;
      touch_initial_y = touch_y;
      touch_released = false;
      x_center = true;
      y_center = true;
    } else {

      if (touch_btn) {
        // CALC distance
        double distance = sqrt(pow(touch_x - touch_initial_x, 2) +
                               pow(touch_y - touch_initial_y, 2));

        double angle =
            atan2(touch_initial_y - touch_y, touch_initial_x - touch_x);

        angle = (angle) * (180.0 / M_PI);
        if (distance > 20) {

          if ((angle >= -22.5 && angle <= 22.5)) {
            // Down
            x_center = true;
            y_u_pressed = false;
            y_d_pressed = true;
          } else if (angle > 22.5 && angle <= 67.5) {
            // Left-down
            x_l_pressed = true;
            x_r_pressed = false;
            y_u_pressed = false;
            y_d_pressed = true;
          } else if (angle > 67.5 && angle <= 112.5) {
            // Left
            x_l_pressed = true;
            x_r_pressed = false;
            y_center = true;
          } else if (angle > 112.5 && angle <= 157.5) {
            // Left-Up
            x_l_pressed = true;
            x_r_pressed = false;
            y_u_pressed = true;
            y_d_pressed = false;
          } else if ((angle > 157.5 && angle <= 180) ||
                     (angle >= -180 && angle <= -157.5)) {
            // Up
            x_center = true;
            y_u_pressed = true;
            y_d_pressed = false;
          } else if (angle > -157.5 && angle <= -112.5) {
            // Right-up
            x_l_pressed = false;
            x_r_pressed = true;
            y_u_pressed = true;
            y_d_pressed = false;
          } else if (angle > -112.5 && angle <= -67.5) {
            // Right
            x_l_pressed = false;
            x_r_pressed = true;
            y_center = true;
          } else if (angle > -67.5 && angle <= -22.5) {
            // Right-down
            x_l_pressed = false;
            x_r_pressed = true;
            y_u_pressed = false;
            y_d_pressed = true;
          } else {
            x_center = true;
            y_center = true;
          }

        } else {

          x_center = true;
          y_center = true;
        }
      }
    }
  }
}

INPUT_CALLBACK_DEFINE(touch_screen_dev, input_touch_cb, NULL);
#endif

//**************************************************************************************

void I_ClearWindow_e32() {}

unsigned short *I_GetBackBuffer() {
  return &backbuffer[CONFIG_DOOM_X_RES]; // Offset by SCREENWIDTH for color
                                         // conversion space
}

unsigned short *I_GetFrontBuffer() {
#ifndef CONFIG_DOOM_NO_WIPE
  return &frontbuffer[0];
#else
  return NULL;
#endif
}

//**************************************************************************************

void I_CreateWindow_e32() {
  // display_blanking_off(display_dev);
}

//**************************************************************************************

void I_CreateBackBuffer_zephyr() { I_CreateWindow_e32(); }

//**************************************************************************************

void I_FinishUpdate_zephyr(const byte *srcBuffer, const byte *pallete,
                        const unsigned int width, const unsigned int height) {

  k_sem_give(&sem_renderer);

  /* Wait for coop thread to let us have a turn */
  k_sem_take(&sem_display, K_FOREVER);
}

//**************************************************************************************

void I_SetPallete_zephyr(const byte *pallete) {
#if defined(CONFIG_DOOM_RGB565)
  for (int i = 0; i < 256; i++) {
    int r = *pallete++;
    int g = *pallete++;
    int b = *pallete++;
    int nr = r >> 3, ng = g >> 2, nb = b >> 3;
    pl_565[i] = (uint16_t)((nr << 11) | (ng << 5) | nb);
    pl_565[i] = doom_swap_s(pl_565[i]);
  }
#elif defined(CONFIG_DOOM_BGR565)
  for (int i = 0; i < 256; i++) {
    int r = *pallete++;
    int g = *pallete++;
    int b = *pallete++;
    int nr = r >> 3, ng = g >> 2, nb = b >> 3;
    pl_565[i] = (uint16_t)((nr << 11) | (ng << 5) | nb);
  }
#else
  pl_game = pallete;
#endif
}

//**************************************************************************************

int I_GetVideoWidth_zephyr() { return vid_width; }

//**************************************************************************************

int I_GetVideoHeight_zephyr() { return vid_height; }

//**************************************************************************************

void I_ProcessKeyEvents() { I_PollWServEvents_e32(); }

//**************************************************************************************

#define MAX_MESSAGE_SIZE 512

void I_Error(const char *error, ...) {
  char msg[MAX_MESSAGE_SIZE];

  va_list v;
  va_start(v, error);

  vsprintf(msg, error, v);

  va_end(v);

  printf("%s\n", msg);

  fflush(stderr);
  fflush(stdout);

  I_Quit_zephyr();
}

//**************************************************************************************

void I_Quit_zephyr() {}

//**************************************************************************************
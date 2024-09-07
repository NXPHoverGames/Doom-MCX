# Copyright 2024 NXP
# SPDX-License-Identifier: gpl-2.0-only

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app LANGUAGES C)

set(PRBOOM_FILES
    prboom2/src/am_map.c
    prboom2/src/d_client.c
    prboom2/src/d_items.c
    prboom2/src/d_main.c
    prboom2/src/doom_iwad.c
    prboom2/src/f_finale.c
    prboom2/src/f_wipe.c
    prboom2/src/g_game.c
    prboom2/src/global_data.c
    prboom2/src/hu_lib.c
    prboom2/src/hu_stuff.c
    prboom2/src/i_audio.c
    prboom2/src/i_main.c
    prboom2/src/info.c
    prboom2/src/i_system.c
    prboom2/src/i_video.c
    prboom2/src/lprintf.c
    prboom2/src/m_bbox.c
    prboom2/src/m_cheat.c
    prboom2/src/m_menu.c
    prboom2/src/m_random.c
    prboom2/src/p_ceilng.c
    prboom2/src/p_doors.c
    prboom2/src/p_enemy.c
    prboom2/src/p_floor.c
    prboom2/src/p_genlin.c
    prboom2/src/p_inter.c
    prboom2/src/p_lights.c
    prboom2/src/p_map.c
    prboom2/src/p_maputl.c
    prboom2/src/p_mobj.c
    prboom2/src/p_plats.c
    prboom2/src/p_pspr.c
    prboom2/src/p_setup.c
    prboom2/src/p_sight.c
    prboom2/src/p_spec.c
    prboom2/src/p_switch.c
    prboom2/src/p_telept.c
    prboom2/src/p_tick.c
    prboom2/src/p_user.c
    prboom2/src/r_data.c
    prboom2/src/r_draw.c
    prboom2/src/r_hotpath.iwram.c
    prboom2/src/r_main.c
    prboom2/src/r_patch.c
    prboom2/src/r_plane.c
    prboom2/src/r_things.c
    prboom2/src/sounds.c
    prboom2/src/s_sound.c
    prboom2/src/st_gfx.c
    prboom2/src/st_lib.c
    prboom2/src/st_stuff.c
    prboom2/src/tables.c
    prboom2/src/version.c
    prboom2/src/v_video.c
    prboom2/src/wi_stuff.c
    prboom2/src/w_wad.c
    prboom2/src/z_bmalloc.c
    prboom2/src/z_zone.c
)

if(CONFIG_DOOM_FIXED_RECIPROCAL_LUT)
list(APPEND PRBOOM_FILES prboom2/src/m_recip.c)
endif()


if(CONFIG_DOOM_SHOW_NXP_LOGO)
list(APPEND PRBOOM_FILES src/nxp_80x32.c)
endif()

target_include_directories(app PUBLIC prboom2/src)
target_include_directories(app PUBLIC src)

target_sources(app PRIVATE ${PRBOOM_FILES} src/i_system_zephyr.c)

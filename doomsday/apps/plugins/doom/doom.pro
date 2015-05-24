# The Doomsday Engine Project
# Copyright (c) 2011-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
# Copyright (c) 2011-2013 Daniel Swanson <danij@dengine.net>

include(../config_plugin.pri)
include(../common/common.pri)
include(../../dep_lzss.pri)
include(../../dep_gui.pri)

TEMPLATE = lib
TARGET   = doom
VERSION  = $$JDOOM_VERSION

DEFINES += __JDOOM__

gamedata.files = $$OUT_PWD/../../libdoom.pk3

macx {
    QMAKE_BUNDLE_DATA += gamedata
}
else {
    INSTALLS += gamedata
    gamedata.path = $$DENG_DATA_DIR/jdoom
}

INCLUDEPATH += include

HEADERS += \
    include/acfnlink.h \
    include/bossbrain.h \
    include/d_api.h \
    include/d_config.h \
    include/d_console.h \
    include/d_englsh.h \
    include/d_event.h \
    include/d_items.h \
    include/d_main.h \
    include/d_player.h \
    include/d_refresh.h \
    include/d_think.h \
    include/doomdata.h \
    include/doomdef.h \
    include/doomtype.h \
    include/doomv9mapstatereader.h \
    include/dstrings.h \
    include/g_game.h \
    include/hud/widgets/ammowidget.h \
    include/hud/widgets/armoriconwidget.h \
    include/hud/widgets/facewidget.h \
    include/hud/widgets/healthiconwidget.h \
    include/hud/widgets/maxammowidget.h \
    include/hud/widgets/weaponslotwidget.h \
    include/info.h \
    include/intermission.h \
    include/jdoom.h \
    include/m_cheat.h \
    include/m_random.h \
    include/oldinfo.h \
    include/p_enemy.h \
    include/p_inter.h \
    include/p_lights.h \
    include/p_local.h \
    include/p_maputl.h \
    include/p_mobj.h \
    include/p_pspr.h \
    include/p_setup.h \
    include/p_spec.h \
    include/p_telept.h \
    include/r_defs.h \
    include/st_stuff.h \
    include/tables.h \
    include/version.h

SOURCES += \
    src/acfnlink.c \
    src/bossbrain.cpp \
    src/d_api.c \
    src/d_console.cpp \
    src/d_items.c \
    src/d_main.cpp \
    src/d_refresh.cpp \
    src/doomv9mapstatereader.cpp \
    src/hud/widgets/ammowidget.cpp \
    src/hud/widgets/armoriconwidget.cpp \
    src/hud/widgets/facewidget.cpp \
    src/hud/widgets/healthiconwidget.cpp \
    src/hud/widgets/maxammowidget.cpp \
    src/hud/widgets/weaponslotwidget.cpp \
    src/intermission.cpp \
    src/m_cheat.cpp \
    src/m_random.c \
    src/p_enemy.c \
    src/p_inter.c \
    src/p_lights.cpp \
    src/p_maputl.c \
    src/p_mobj.c \
    src/p_pspr.c \
    src/p_setup.c \
    src/p_spec.cpp \
    src/p_telept.c \
    src/st_stuff.cpp \
    src/tables.c

win32 {
    deng_msvc:  QMAKE_LFLAGS += /DEF:\"$$PWD/api/doom.def\"
    deng_mingw: QMAKE_LFLAGS += --def \"$$PWD/api/doom.def\"

    OTHER_FILES += api/doom.def

    RC_FILE = res/doom.rc
}

macx {
    fixPluginInstallId($$TARGET, 1)
    linkToBundledLibcore($$TARGET)
    linkToBundledLiblegacy($$TARGET)
}
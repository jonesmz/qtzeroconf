include(../../project_settings.pri)
TARGET     = qtzeroconf-browser
TEMPLATE   = lib
CONFIG    += link_pkgconfig
PKGCONFIG += avahi-qt5 avahi-client

INCLUDEPATH += $$PROJ_DIR/include/
SOURCES     += zconfservicebrowser.cpp

include(../../project_settings.pri)
DEPENDENCY_LIBRARIES = qtzeroconf-common
include(../../dependency.pri)
TEMPLATE   = lib
CONFIG    += link_pkgconfig
PKGCONFIG += avahi-qt5 avahi-client

INCLUDEPATH += $$PROJ_DIR/include/
SOURCES     += zconfservice.cpp

include(project_settings.pri)
TARGET     = qtzeroconf
TEMPLATE   = lib
CONFIG    += link_pkgconfig
PKGCONFIG += avahi-qt5 avahi-client

INCLUDEPATH += include/

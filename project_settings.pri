PROJ_DIR     = $$PWD

QT          -= gui
QT          += core
CONFIG      += c++11 separate_debug_info
VERSION      = 0.0.1

DESTDIR      = $$PROJ_DIR/bin

MOC_DIR      = .moc
UI_DIR       = .ui
RCC_DIR      = .rcc
OBJECTS_DIR  = .obj

INCLUDEPATH += $$PROJ_DIR/include

QMAKE_CXXFLAGS += -Wall -Wcast-align -Wextra -Wfloat-equal -Wformat=2 -Wformat-nonliteral -Wmissing-braces -Wmissing-declarations -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-noreturn -Woverlength-strings -Wparentheses -Wpointer-arith -Wredundant-decls -Wreturn-type -Wsequence-point -Wsign-compare -Wswitch -Wuninitialized -Wunknown-pragmas -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable -Wwrite-strings -O3

*clang* {
    QMAKE_CXXFLAGS += -Wdeprecated-implementations -Wfour-char-constants -Wimplicit-atomic-properties -Wnewline-eof -Wswitch-default -Wshadow -Wbad-function-cast -Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wold-style-definition -Wstrict-prototypes -Wstrict-selector-match -Wundeclared-selector
}

# On windows, a shared object is a .dll
win32: SONAME=dll
else:  SONAME=so

# This function sets up the dependencies for libraries that are built with
# this project.  Specify the libraries you need to depend on in the variable
# DEPENDENCY_LIBRARIES and this will add
for(dep, DEPENDENCY_LIBRARIES) {
    #message($$TARGET depends on $$dep ($${DESTDIR}/lib$${dep}.$${SONAME}))
    LIBS               += -L$${DESTDIR} -l$${dep}
    PRE_TARGETDEPS     += $${DESTDIR}/lib$${dep}.$${SONAME}
    QMAKE_LFLAGS += -Wl,-rpath=\'\$\$ORIGIN\'
    QMAKE_LFLAGS += -Wl,-rpath-link=\'$${DESTDIR}\'
    QMAKE_LFLAGS_RPATH =
}

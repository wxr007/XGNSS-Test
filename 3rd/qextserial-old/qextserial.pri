HEADERS += ./qextserialport.h \
    ./qextserialport_global.h \
    ./qextserialport_p.h

SOURCES += ./qextserialport.cpp
win32 {
     SOURCES += ./qextserialport_win.cpp
}

unix {
     SOURCES += ./qextserialport_unix.cpp
}

OTHER_FILES += \
    ./qextserial/LICENSE

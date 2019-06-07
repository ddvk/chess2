
TEMPLATE = app
TARGET = chess
QT += qml quick
CONFIG += c++14

win32 {
    RC_FILE = chess.rc
}

OTHER_FILES =	Board.qml \
				Cell.qml \
				main.qml
				
SOURCES =	main.cpp \
			figures.cpp \
			board.cpp \
			game.cpp
			
HEADERS =	figures.hpp \
			board.hpp \
			game.hpp \
			signals.hpp

RESOURCES = resources.qrc

DISTFILES += \
    UndoBtn.qml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += android/AndroidManifest.xml

linux-oe-g++ {
    message("rm")
    LIBS += -lqsgepaper
    DEFINES += RM
}

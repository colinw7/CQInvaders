TEMPLATE = app

TARGET = CQInvaders

QT += widgets

DEPENDPATH += .

INCLUDEPATH += .

# Input
HEADERS += CQSpaceInvaders.h CSpaceInvaders.h CQSound.h CSDLSound.h
SOURCES += CQSpaceInvaders.cpp CQSound.cpp CSDLSound.cpp

unix:LIBS += -lSDL -lSDL_mixer

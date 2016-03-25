TEMPLATE = app

TARGET = CQInvaders

QT += widgets multimedia

DEPENDPATH += .

INCLUDEPATH += .

# Input
HEADERS += CQSpaceInvaders.h CSpaceInvaders.h CQSound.h CSDLSound.h
SOURCES += CQSpaceInvaders.cpp CQSound.cpp CSDLSound.cpp

unix:LIBS += -lSDL2 -lSDL2_mixer

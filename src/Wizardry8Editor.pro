VERSION=0.1.4

# "These comments are in quotes because the apostrophes mix up the"
# "syntax highlighting otherwise."

# "I'd rather this was evaluated in the Makefile than here (which can be done"
# "through the use of the undocumented QMAKE_EXTRA_VARIABLES syntax, ie."
# " QMAKE_EXTRA_VARIABLES += GCC_ARCH"
# " GCC_ARCH=$$quote($(shell $(CC) -dumpmachine))"
# "(provided you don't mind EXPORT_ prepended on the front)"
# "or just including my own makefile with also undocumented QMAKE_EXTRA_INCLUDES"
# "But moc doesn't put its output in the right folder with that. And qmake"
# "doesn't generate the output folder if it doesn't already exist."

# "I do all this because I hate having to clean everything and recompile from"
# "scratch when swapping between different cross compile targets during development"

CC_ARCH=$$system($${QMAKE_CC} -dumpmachine)

OBJECTS_DIR=.$${CC_ARCH}
MOC_DIR=.$${CC_ARCH}
RCC_DIR=.$${CC_ARCH}

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS = -g -O0 -DQ_ASSERT_IS_BROKEN

QMAKE_CXXFLAGS += "-Wno-implicit-fallthrough -Wno-write-strings"

QMAKE_LIBS += -lbz2

QT       += core
QT       += xml

QT       += gui
QT       += widgets
QT       += multimedia

TARGET = Wizardry8Editor
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += "VERSION=$${VERSION}"

SOURCES += main.cpp \
           Wizardry8Style.cpp \
           WButton.cpp \
           WCheckBox.cpp \
           WDDL.cpp \
           WImage.cpp \
           WItem.cpp \
           WLabel.cpp \
           WLineEdit.cpp \
           WListWidget.cpp \
           WScrollBar.cpp \
           WSpinBox.cpp \
           WStatBar.cpp \
           MainWindow.cpp \
           RIFFFile.cpp \
           SLFFile.cpp \
           STItoQImage.cpp \
           TGAtoQImage.cpp \
           StringList.cpp \
           Dialog.cpp \
           DialogAbout.cpp \
           DialogAddItem.cpp \
           DialogBegin.cpp \
           DialogChooseColumns.cpp \
           DialogDuration.cpp \
           DialogInfo.cpp \
           DialogItemInfo.cpp \
           DialogSpellInfo.cpp \
           DialogPatchExe.cpp \
           DialogNewFile.cpp \
           Screen.cpp \
           ScreenCommon.cpp \
           ScreenItems.cpp \
           ScreenLevels.cpp \
           ScreenMagic.cpp \
           ScreenSkills.cpp \
           ScreenAttribs.cpp \
           ScreenPersonality.cpp \
           WindowDroppedItems.cpp \
           WTableWidgetItem.h \
           WTableWidget.h \
           WindowItemsList.cpp \
           dbHelper.cpp \
           character.cpp \
           party.cpp \
           item.cpp \
           spell.cpp \
           bspatch.c

HEADERS += Wizardry8Style.h \
           Wizardry8Scalable.h \
           WButton.h \
           WCheckBox.h \
           WDDL.h \
           WImage.h \
           WItem.h \
           WLabel.h \
           WLineEdit.h \
           WListWidget.h \
           WScrollBar.h \
           WSpinBox.h \
           WStatBar.h \
           MainWindow.h \
           RIFFFile.h \
           SLFFile.h \
           STItoQImage.h \
           TGAtoQImage.h \
           StringList.h \
           Dialog.h \
           DialogAbout.h \
           DialogAddItem.h \
           DialogBegin.h \
           DialogChooseColumns.h \
           DialogDuration.h \
           DialogInfo.h \
           DialogItemInfo.h \
           DialogSpellInfo.h \
           DialogPatchExe.h \
           DialogNewFile.h \
           Screen.h \
           ScreenCommon.h \
           ScreenItems.h \
           ScreenLevels.h \
           ScreenMagic.h \
           ScreenSkills.h \
           ScreenAttribs.h \
           ScreenPersonality.h \
           WindowDroppedItems.h \
           WindowItemsList.h \
           bspatch.h \
           dbHelper.h \
           character.h \
           party.h \
           item.h \
           spell.h \
           constants.h \
           common.h \
           pinata.xpm


# Referenced in the qrc resource file
unix {
    RESOURCES += blobs.qrc

    BLOBS =  WizFontOtfReg.bspatch \
             WizFontOtfBold.bspatch \
             death.u4 \
             death.v4
}

win32 {
    contains(QT_ARCH, x86_64) {
        RESOURCES += blobs.qrc

        RC_ICONS = Win64.ico

        BLOBS =  WizFontOtfReg.bspatch \
                 WizFontOtfBold.bspatch \
                 death.u4 \
                 death.v4
    }
    else {
        RESOURCES += win32_blobs.qrc

        RC_ICONS = WinXP.ico

        BLOBS =  WizFontTtfReg.bspatch \
                 WizFontTtfBold.bspatch \
                 death.u4 \
                 death.v4
    }
}

QMAKE_EXTRA_TARGETS += $${TARGET}.zip

$${TARGET}.zip.commands = zip $${TARGET}.zip $${SOURCES} $${HEADERS} $${RESOURCES} $${TARGET}.pro $${BLOBS} $${RC_ICONS}
$${TARGET}.zip.depends  = $${SOURCES} $${HEADERS} $${RESOURCES} $${TARGET}.pro $${BLOBS}

$${OBJECTS_DIR}/qrc_blobs.cpp.depends += $${TARGET}.zip

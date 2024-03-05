VERSION=0.2.0

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

# If we're compiling with plain old gcc this gives us plain old cmake
# If we're cross compiling with MXE, it copies the platform prefix from the compiler across
CMAKE=$$replace(QMAKE_CC, gcc, cmake)

OBJECTS_DIR=.$${CC_ARCH}
MOC_DIR=.$${CC_ARCH}
RCC_DIR=.$${CC_ARCH}

URHO3D=Urho3D-1.8

unix {
    URHO3D_TYPE=OpenGL
}

win32 {
# FIXME: What does this look like for Windows? esp. compiling for DirectX. Also saw a warning on forum:
# To build the STATIC version of Urho3D for Qt Creator MinGW 8.1.0 you need to use the master branch of Urho3D. The -liphlpapi key is required to avoid this error: ￼￼undefined reference to `GetAdapterAddresses@20'
    DIRECTX {
        URHO3D_ADDITIONAL_CONFIGS=-DURHO3D_OPENGL=FALSE
        URHO3D_TYPE=DirectX
    }
    else {
        URHO3D_TYPE=OpenGL
    }
}

URHO3D_DIR=$${URHO3D_TYPE}/$${URHO3D}

PRE_TARGETDEPS = .$${CC_ARCH}/$${URHO3D_DIR}/lib/libUrho3D.a

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE  = -O3 -msse -msse2
QMAKE_CXXFLAGS_DEBUG    = -g  -msse -msse2 -O0 -DQ_ASSERT_IS_BROKEN -fpermissive
QMAKE_LFLAGS_DEBUG     += -Xlinker -Map=Wizardry8Editor.map 
QMAKE_CLEAN += Wizardry8Editor.map 

# We use -isystem instead of -I when including the Urho include folders because that suppresses the ton of warnings
# they generate during compile otherwise
QMAKE_CXXFLAGS += -Wno-implicit-fallthrough -Wno-write-strings -isystem ${OBJECTS_DIR}/$${URHO3D_DIR}/include -isystem ${OBJECTS_DIR}/$${URHO3D_DIR}/include/Urho3D/ThirdParty -isystem ${OBJECTS_DIR}/$${URHO3D_DIR}/include/Urho3D/ThirdParty/Bullet -DURHO3D_TYPE=$${URHO3D_TYPE}

# pthread is already a dependency but Urho3D needs it as well, and doesn't get it unless it's listed AFTER Urho
# so we have to double up
QMAKE_LIBS += -lbz2 -L${OBJECTS_DIR}/$${URHO3D_DIR}/lib -lUrho3D -lpthread -ldl -lm

unix {
    QMAKE_LIBS += -lrt -lGL
}

win32 {
    DIRECTX {
        QMAKE_LIBS += -ld3dcompiler -ld3d9
    }
    else {
        QMAKE_LIBS += -lopengl32
    }
    QMAKE_LIBS += -lversion -lwinmm -lws2_32 -lgdi32 -limm32 -lsetupapi
}

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
           Localisation.cpp \
           MainWindow.cpp \
           RIFFFile.cpp \
           SLFFile.cpp \
           SLFDeserializer.cpp \
           STI.cpp \
           TGAtoQImage.cpp \
           StringList.cpp \
           PortraitsDb.cpp \
           Dialog.cpp \
           DialogAbout.cpp \
           DialogAboutUrho3D.cpp \
           DialogAddItem.cpp \
           DialogBegin.cpp \
           DialogChooseColumns.cpp \
           DialogCurrentPosition.cpp \
           DialogDuration.cpp \
           DialogInfo.cpp \
           DialogItemInfo.cpp \
           DialogSpellInfo.cpp \
           DialogParallelWorlds.cpp \
           DialogPatchExe.cpp \
           DialogNewFile.cpp \
           ReplacePortrait.cpp \
           Screen.cpp \
           ScreenCommon.cpp \
           ScreenItems.cpp \
           ScreenLevels.cpp \
           ScreenMagic.cpp \
           ScreenSkills.cpp \
           ScreenAttribs.cpp \
           ScreenPersonality.cpp \
           Level.cpp \
           Touch.cpp \
           Window3DNavigator.cpp \
           WindowDroppedItems.cpp \
           WTableWidgetItem.h \
           WTableWidget.h \
           WindowItemsList.cpp \
           WindowFactEditor.cpp \
           Avatar.cpp \
           dbHelper.cpp \
           character.cpp \
           facts.cpp \
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
           Localisation.h \
           MainWindow.h \
           RIFFFile.h \
           SLFFile.h \
           SLFDeserializer.h \
           STI.h \
           TGAtoQImage.h \
           StringList.h \
           PortraitsDb.h \
           Dialog.h \
           DialogAbout.h \
           DialogAboutUrho3D.h \
           DialogAddItem.h \
           DialogBegin.h \
           DialogChooseColumns.h \
           DialogCurrentPosition.h \
           DialogDuration.h \
           DialogInfo.h \
           DialogItemInfo.h \
           DialogSpellInfo.h \
           DialogParallelWorlds.h \
           DialogPatchExe.h \
           DialogNewFile.h \
           ReplacePortrait.h \
           Screen.h \
           ScreenCommon.h \
           ScreenItems.h \
           ScreenLevels.h \
           ScreenMagic.h \
           ScreenSkills.h \
           ScreenAttribs.h \
           ScreenPersonality.h \
           UrhoTools.h \
           Level.h \
           Touch.h \
           Window3DNavigator.h \
           WindowDroppedItems.h \
           WindowItemsList.h \
           WindowFactEditor.h \
           Avatar.h \
           bspatch.h \
           dbHelper.h \
           character.h \
           facts.h \
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

QMAKE_EXTRA_TARGETS += .deps/$${URHO3D}.tar.gz \
                       .$${CC_ARCH}/$${URHO3D_DIR}/CMakeLists.txt \
                       .$${CC_ARCH}/$${URHO3D_DIR}/Makefile \
                       .$${CC_ARCH}/$${URHO3D_DIR}/lib/libUrho3D.a \
                       .$${CC_ARCH}/$${URHO3D_DIR}/LICENSE \
                       $${URHO3D_OpenGL_RESOURCES} \
                       $${URHO3D_DirectX_RESOURCES} \
                       urho3d_license.xxd \
                       urho3d_authors.xxd \
                       CoreData.pak \
                       .$${CC_ARCH}/$${URHO3D_DIR}/bin/tool/PackageTool \
                       $${TARGET}.zip

URHO3D_COMMON_RESOURCES =  \
    Fonts/"Anonymous\ Pro.ttf" \
    UI/DefaultStyle.xml \
    Urho2D/sun.png \
    Urho2D/sun.pex \
    Materials/Skybox.xml \
    Materials/Water.xml \
    Models/Box.mdl \
    Models/Mutant/Mutant.mdl \
    Models/Mutant/Mutant_Idle0.ani \
    Models/Mutant/Mutant_Idle1.ani \
    Models/Mutant/Mutant_Jump1.ani \
    Models/Mutant/Mutant_Kick.ani \
    Models/Mutant/Mutant_Punch.ani \
    Models/Mutant/Mutant_Run.ani \
    Models/Mutant/Mutant_Swipe.ani \
    Models/Mutant/Mutant_Walk.ani \
    Models/Mutant/Materials/mutant_M.xml \
    Models/Mutant/Textures/Mutant_diffuse.jpg \
    Models/Mutant/Textures/Mutant_normal.jpg \
    Techniques/Diff.xml \
    Techniques/DiffAlpha.xml \
    Techniques/DiffAlphaTranslucent.xml \
    Techniques/DiffNormal.xml \
    Techniques/DiffSkybox.xml \
    Techniques/NoTexture.xml \
    Techniques/NoTextureAOAlpha.xml \
    Techniques/Water.xml \
    Textures/FishBoneLogo.png \
    Textures/Ramp.png \
    Textures/Spot.png \
    Textures/UI.png \
    Textures/UrhoDecal.dds \
    Textures/WaterNoise.dds \
    RenderPaths/Forward.xml

URHO3D_LOCAL_RESOURCES = \
    UrhoData/Textures/BlowtorchBlueFlame.png \
    UrhoData/Textures/BlowtorchOrange1.png   \
    UrhoData/Textures/BlowtorchOrange2.png   \
    UrhoData/Textures/BlowtorchOrange3.png   \
    UrhoData/Textures/BlowtorchOrange4.png   \
    UrhoData/Textures/Skybox.xml \
    UrhoData/Textures/SkyStars_PosX.png \
    UrhoData/Textures/SkyStars_NegX.png \
    UrhoData/Textures/SkyStars_PosY.png \
    UrhoData/Textures/SkyStars_NegY.png \
    UrhoData/Textures/SkyStars_PosZ.png \
    UrhoData/Textures/SkyStars_NegZ.png

# FIXME:
URHO3D_DirectX_RESOURCES =  $${URHO3D_COMMON_RESOURCES} \
    Shaders/HLSL/Urho2D.hlsl \
    Shaders/HLSL/Basic.hlsl \
    Shaders/HLSL/Fog.hlsl \
    Shaders/HLSL/Lighting.hlsl \
    Shaders/HLSL/LitSolid.hlsl \
    Shaders/HLSL/Samplers.hlsl  \
    Shaders/HLSL/ScreenPos.hlsl \
    Shaders/HLSL/Shadow.hlsl \
    Shaders/HLSL/Skybox.hlsl \
    Shaders/HLSL/Stencil.hlsl \
    Shaders/HLSL/Transform.hlsl \
    Shaders/HLSL/Uniforms.hlsl \
    Shaders/HLSL/Water.hlsl

URHO3D_OpenGL_RESOURCES =  $${URHO3D_COMMON_RESOURCES} \
    Shaders/GLSL/Urho2D.glsl \
    Shaders/GLSL/Basic.glsl \
    Shaders/GLSL/Fog.glsl \
    Shaders/GLSL/Lighting.glsl \
    Shaders/GLSL/LitSolid.glsl \
    Shaders/GLSL/Samplers.glsl  \
    Shaders/GLSL/ScreenPos.glsl \
    Shaders/GLSL/Shadow.glsl \
    Shaders/GLSL/Skybox.glsl \
    Shaders/GLSL/Stencil.glsl \
    Shaders/GLSL/Transform.glsl \
    Shaders/GLSL/Uniforms.glsl \
    Shaders/GLSL/Water.glsl

$${TARGET}.zip.commands = zip $@ $+
$${TARGET}.zip.depends  = $${SOURCES} $${HEADERS} $${RESOURCES} $${TARGET}.pro $${URHO3D_LOCAL_RESOURCES} \
                          $${BLOBS} $${RC_ICONS} .deps/Urho3D_QFile.patch
QMAKE_CLEAN += $${TARGET}.zip

$${OBJECTS_DIR}/qrc_blobs.cpp.depends += $${TARGET}.zip CoreData.pak

.deps/$${URHO3D}.tar.gz.commands = mkdir -p .deps && \
                                   curl -L -o $@ https://github.com/urho3d/Urho3D/archive/refs/tags/1.8.tar.gz
.deps/$${URHO3D}.tar.gz.depends  =

# The version of the 1.8 archive on Githib was updated in-place to change the case of
# the top level folder. That's all well and good on Windows but screws things up compiling on linux.
# archive with  MD5SUM: 5210a6f0e1a843c49984b3b01965459e            : Urho3D-1.8
# archive with  MD5SUM: 9aa2190b7bdf89fa4a7a581ad3163c78 (current)  : urho3d-1.8

.$${CC_ARCH}/$${URHO3D_DIR}/CMakeLists.txt.commands = mkdir -p .$${CC_ARCH}/$${URHO3D_TYPE} \
                                                      $$escape_expand(\n\t)tar xf .deps/$${URHO3D}.tar.gz -C .$${CC_ARCH}/$${URHO3D_TYPE} \
                                                      $$escape_expand(\n\t)@-/bin/echo \"9aa2190b7bdf89fa4a7a581ad3163c78 $$escape_expand( ).deps/$${URHO3D}.tar.gz\" | md5sum -c - && ([ \$\$? -eq 0 ] && mv .$${CC_ARCH}/$${URHO3D_TYPE}/urho3d-1.8 .$${CC_ARCH}/$${URHO3D_DIR}) \
                                                      $$escape_expand(\n\t)(cd .$${CC_ARCH}/$${URHO3D_TYPE} ; patch -p0 < ../../.deps/Urho3D_QFile.patch)
# The pipe symbol prevents the datestamp on the archive being compared to the datestamp on the CMakeLists.txt
# extracted from it (which is always going to be older) and would force this extraction to happen every time otherwise
.$${CC_ARCH}/$${URHO3D_DIR}/CMakeLists.txt.depends  = | .deps/$${URHO3D}.tar.gz

# FIXME: This is possibly where it goes pear shaped for cross compile -DMINGW_PREFIX= ?? -DCMAKE_CROSSCOMPILING=TRUE ??
# FIXME: -DURHO3D_OPENGL=FALSE on Windows (Windows supports both DirectX and OpenGL, but only at compile time)
# ALSO: On Windows platform Direct3D11 can be optionally chosen: -DURHO3D_D3D11=TRUE
# Using Direct3D11 on non-MSVC compiler may require copying and renaming Microsoft official libraries (.lib to .a), else link failures or non-functioning graphics may result

.$${CC_ARCH}/$${URHO3D_DIR}/Makefile.commands = (cd .$${CC_ARCH}/$${URHO3D_DIR} ; $${CMAKE} -DURHO3D_ANGELSCRIPT=FALSE -DURHO3D_LUA=FALSE -DURHO3D_LUAJIT=FALSE -DURHO3D_PLAYER=FALSE -DURHO3D_SAMPLES=FALSE $${URHO3D_ADDITIONAL_CONFIGS} .)
.$${CC_ARCH}/$${URHO3D_DIR}/Makefile.depends  = .$${CC_ARCH}/$${URHO3D_DIR}/CMakeLists.txt

.$${CC_ARCH}/$${URHO3D_DIR}/lib/libUrho3D.a.commands = (cd .$${CC_ARCH}/$${URHO3D_DIR} ; make)
.$${CC_ARCH}/$${URHO3D_DIR}/lib/libUrho3D.a.depends  = .$${CC_ARCH}/$${URHO3D_DIR}/Makefile

urho3d_authors.xxd.commands = gawk \'/$${LITERAL_HASH}$${LITERAL_HASH}/ { cap=0; } /$${LITERAL_HASH}$${LITERAL_HASH} Credits/ { cap=1; } { if (cap==1) print $0 }\' $< | xxd -i > $@
urho3d_authors.xxd.depends  = .$${CC_ARCH}/$${URHO3D_DIR}/README.md
QMAKE_CLEAN += urho3d_authors.xxd

urho3d_license.xxd.commands = xxd -i < $< > $@
urho3d_license.xxd.depends  = .$${CC_ARCH}/$${URHO3D_DIR}/LICENSE
QMAKE_CLEAN += urho3d_license.xxd

.$${CC_ARCH}/$${URHO3D_DIR}/LICENSE.depends = .$${CC_ARCH}/$${URHO3D_DIR}/CMakeLists.txt


equals(URHO3D_TYPE,"OpenGL") {
    CoreData_FILES = $${URHO3D_OpenGL_RESOURCES}
} else {
    CoreData_FILES = $${URHO3D_DirectX_RESOURCES}
}

# The CoreData_FILES variable contains a list of files which are scattered over 2 different folder
# in the source tree (CoreData and Data). It isn't easy to turn this into a list of dependencies here
# (could split into 2 separate lists but it's just a pest maintaining them that way). Instead just not
# specifying them as deps here at all. I don't expect these to change anyway; they should be the
# ones distributed with Urho without modification.
# Any files of our own ARE in a separate list.
# PackageTool uses the File component of Urho itself, and as a result compiles for the TARGET architecture
# rather than getting linked for the HOST. Extremely annoying. I'm not wastign time fixing this because
# Urho3D is already discontinued upstream. For now you'll have to manually compile Urho3D for your host
# OS yourself and copy the PackageTool command into the tree at
# .x86_64-w64-mingw32.static/( OpenGL | DirectX )/Urho3D-1.8/bin/tool/PackageTool
# if you are cross-compiling
CoreData.pak.depends  = .$${CC_ARCH}/$${URHO3D_DIR}/lib/libUrho3D.a $${TARGET}.pro .$${CC_ARCH}/$${URHO3D_DIR}/bin/tool/PackageTool
CoreData.pak.commands = mkdir -p .$${CC_ARCH}/CoreData && \
                       echo "Building PAK for $${URHO3D_TYPE}" && \
                       tar c -C .$${CC_ARCH}/$${URHO3D_DIR}/bin/Data $${CoreData_FILES} 2>/dev/null| tar x -C .$${CC_ARCH}/CoreData  && \
                       tar c -C .$${CC_ARCH}/$${URHO3D_DIR}/bin/CoreData $${CoreData_FILES} 2>/dev/null| tar x -C .$${CC_ARCH}/CoreData  && \
                       cp -r UrhoData/* .$${CC_ARCH}/CoreData  && \
                       .$${CC_ARCH}/$${URHO3D_DIR}/bin/tool/PackageTool .$${CC_ARCH}/CoreData $@ && \
                       rm -rf .$${CC_ARCH}/CoreData
QMAKE_CLEAN += CoreData.pak

.$${CC_ARCH}/$${URHO3D_DIR}/bin/tool/PackageTool.commands = @/bin/echo \""PackageTool is missing. This is likely because you are cross-compiling."\" \
                       $$escape_expand(\n\t)@/bin/echo \""PackageTool depends on the Urho3D libraries themselves, and hence"\" \
                       $$escape_expand(\n\t)@/bin/echo \""gets compiled for TARGET, not HOST. You'll need to manually compile"\" \
                       $$escape_expand(\n\t)@/bin/echo \""Urho3D for your host OS and copy the PackageTool binary into this folder"\" \
                       $$escape_expand(\n\t)@/bin/echo \""in order to compile:"\" \
                       $$escape_expand(\n\t)@/bin/echo \""    .$${CC_ARCH}/$${URHO3D_DIR}/bin/tool/"\" \
                       $$escape_expand(\n\t)/bin/false

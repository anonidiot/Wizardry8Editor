# Wizardry8Editor
QT Based Save Game editor for Wizardry 8

Latest Release 0.2.3

- BUGFIX: Fix inverted logic that broke magic spells loading and saving with v1.28 in v0.2.2

**Win32 version** - should run on Windows XP even with old hardware but not the smoothest performance in the 3d navigator
(OpenGL only, because DirectX has DLL requirements not supported by Windows XP). No HiDPI support:

53e80d18da1cc3ae6fbf0e41cd42a4c6 Wizardry8Editor_win32.exe

**Win64 versions** with HiDPI support:

703a52f3efa7daf22d5c1d947716884b  Wizardry8Editor_win64_DirectX.exe
3dd9a6270b430674957e5d937c7a6afb  Wizardry8Editor_win64_OpenGL.exe


Scripts for making Docker images containing the MXE compiler necessary to do the release builds are now included. Previously I made the releases using a dedicated VM, but have changed to these for this release. They have only been tested running cross-compiles from linux, but I believe should work from Windows too.

If you run the host's native qmake to generate a Makefile, the necessary commands for invoking the container cross-compile builds all exist as main Makefile commands, and the process is easier, but for those not interested in installing native host qmake just for this, the expanded commands below can be used directly:

32 Bit Windows compile (OpenGL only, because the DirectX version requires D3DCOMPILER_47.dll which isn't supported on Windows XP)
======================

If you have a native host Makefile created by qmake, you can just use:

    make docker_win32

to make the docker image. This performs the equivalent of these commands, which can be invoked manually if you don't have the Makefile:

    docker build -t mxe_i686 -f Dockerfile.i686.win32 . && touch docker_win32

It will take at least an hour even on a modern machine, but only needs to be done once.

The actual compile can be done with:

    make win32_opengl

of for those without the Makefile:

    rm -f Wizardry8Editor_resource.rc Wizardry8Editor_resource.rc wizardry8editor_plugin_import.cpp
    touch Wizardry8Editor.zip CoreData.pak
    docker run --rm -it --mount type=bind,src=/home/bpurcell/src/Wizardry8Editor,target=/mnt -u `id -u`:`id -g` mxe_i686 qmake Wizardry8Editor.pro -o Makefile.win32_opengl
    rm Wizardry8Editor.zip CoreData.pak
    docker run --rm -it --mount type=bind,src=/home/bpurcell/src/Wizardry8Editor,target=/mnt -u `id -u`:`id -g` mxe_i686 make -f Makefile.win32_opengl

64 Bit Windows compile
======================

If you have a native host Makefile created by qmake, you can just use:

    make docker_win64

to make the docker image. This performs the equivalent of these commands, which can be invoked manually if you don't have the Makefile:

    docker build -t mxe_x86_64 -f Dockerfile.x86_64.win64 . && touch docker_win64

It will take at least an hour and a half even on a modern machine, but only needs to be done once.

The actual compile can be done with:

    make win64_opengl
or
    make win64_directx

of for those without the Makefile, use this for OpenGL

    rm -f wizardry8editor_plugin_import.cpp Wizardry8Editor_resource.rc wizardry8editor_plugin_import.cpp
    docker run --rm -it --mount type=bind,src=/home/bpurcell/src/Wizardry8Editor,target=/mnt -u `id -u`:`id -g` mxe_x86_64 qmake Wizardry8Editor.pro -o Makefile.win64_opengl
    docker run --rm -it --mount type=bind,src=/home/bpurcell/src/Wizardry8Editor,target=/mnt -u `id -u`:`id -g` mxe_x86_64 make -f Makefile.win64_opengl

and this for DirectX:

    rm -f Wizardry8Editor_resource.rc Wizardry8Editor_resource.rc wizardry8editor_plugin_import.cpp
    docker run --rm -it --mount type=bind,src=/home/bpurcell/src/Wizardry8Editor,target=/mnt -u `id -u`:`id -g` mxe_x86_64 qmake Wizardry8Editor.pro CONFIG+=DIRECTX -o Makefile.win64_directx
    docker run --rm -it --mount type=bind,src=/home/bpurcell/src/Wizardry8Editor,target=/mnt -u `id -u`:`id -g` mxe_x86_64 make -f Makefile.win64_directx



The win32 version defaults to using "prescott" (a pentium 4 equivalent) and the win64 version defaults to "x86_64". The supported cmake target for compiling Urho only works if SSE is enabled, so it isn't possible to compile for an old generic architecture like "i686" without additional changes; nor is it possible to go much newer than "x86_64" as the "x86-64-v2" and "x86-64-v3" archs aren't recognised by the MXE compiler in the docker images.

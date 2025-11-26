# Wizardry8Editor
QT Based Save Game editor for Wizardry 8

Latest Release 0.2.2

- Get the Win32 executable functional again by heavily patching Urho (OpenGL only)
- Docker container build scripts and methods for compiling the targets added to the tree
- Simple preferences dialog added with some minimal options for those who didn't know about the command line options for reseting the wizardry path, mainly.
- Switch on Fractional scaling in HighDPI, which might improve the visuals for some users
- Add an id column to the fact editor, since the numbers get used a lot.
- BUGFIX: Original games under Wizardry 28 have their own headaches, because certain elements like spells are still in the original format, but everything else changes - no clean way to detect this situation so relying on the module name if ParallelWorlds active, but no solution yet if it isn't.
- BUGFIX: Try to resolve some of the problems surrounding dead characters in a party being edited.

**Win32 version** - should run on Windows XP even with old hardware but not the smoothest performance in the 3d navigator
(OpenGL only, because DirectX has DLL requirements not supported by Windows XP). No HiDPI support:

964fc5b42fc1740f9f54dbde4c830afc Wizardry8Editor_win32.exe

**Win64 versions** with HiDPI support:

69d686a92bb1de1b183594d28f8c2ae9 Wizardry8Editor_win64_DirectX.exe
1111b66be1b22bd4603eb419c09cee0f Wizardry8Editor_win64_OpenGL.exe

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



There's no -march= compile switches in the compiles invoked here and there really should be. Without it they can generate instructions that don't work on all platforms - and this is noticeably the case when generating images capable of running on 32-bit Windows XP. I just used an old machine to do the release builds instead, rather than pinning down a target architecture.

Release 0.2.0

Adds Wizardry 1.2.8 Fanpatch (Partial) compatibility

Supports the extended portraits range and the ParallelWorlds layout. Partially supports Localisation.
Does not support increased spell ranges, increased voices, facts, or levels.
Tested against 1.2.8 Build 6200 which it can successfully patch.
Later builds may work with the editor, but won't be patchable.

Release 0.1.6

Adds support for navigating levels to change the current saved position and portal locations.

MANY KNOWN ISSUES - all relating to the 3d rendering
*Clipping is bad.* - Not likely to improve, sorry. Part of this is due to needing to work with an externally defined level file format not optimised for our own usage, but mainly it's due to my lack of knowledge over how to properly fix it.
*More detailed (eg. anything internal) levels are much slower to render.* - Same excuse as above.
*Slow level load and no 'Loading' to tell you its working.*
*Lot of objects missing.* - Anything classed as a dynamic object is omitted. This includes most lava, treasure chest lids, doorways etc. Supporting this in the editor requires a good bit more additional work. ie. in addition to parsing those elements in the PVL file properly (instead of skipping them) interactive elements would need to be supported in the level navigation to eg. recognise doors and provide ways to open them regardless of the in-game mechanism used (eg. push door, control panel, or key in lock). Otherwise a lot more no-clip mode becomes necessary to navigate a level. It would be a nice to have all this, but not happening any time soon.
*Not all textures rendering correctly.* - This is particularly relevant to textures which are supposed to render transmissively rather than diffusely (eg. some of the cobwebs in Lower Marten's Bluff coming in from the Mine Tunnels).
*Some trees completely walkthrough.* - Wizardry implements its trees as 2 vertical rectangles at 90 degrees to each other in a cross piece and then just textures them with a bitmap with large alpha areas. Urho3Ds collision logic depends on shape primitives, so we can't selectively collide just where it is non-alpha. Because some levels become unnavigable if the complete tree is treated as unpassable even in the alpha areas I chose to err in the other direction and make them completely pass-through instead. This opens up a problem, though, where if a portal or current position is saved inside one of these tree objects you'll be trapped in it when it loads in the game. The editor can't prevent you doing this for the same reason it can't process the alpha regions properly to begin with, so the user needs to be careful themselves.
*Some doorways (eg. upper level of Croc's) can't be navigated in one direction.* - Even tiny steps up in confined spaces like doorways don't work well. You can usually jump them, but sometimes it takes many attempts to work. Switch to no-clip mode if nothing else works.
*No elevators or portals work.* - You can jump directly to any of the portal arrival points using the Jump to Point function. To get between vertical levels normally linked by an elevator you'll need to resort to no-clip mode.
*Gaping holes in the middle of the map.* - Some areas, eg. lava flow, are implemented with dynamic objects which we ignore. If you fall into a hole you'll fall forever and will need to reset your position by using Jump to Point. Same if you run off the edge of a map into the abyss.
*Sometimes Jumping to a Point or loading a level causes player to drop through ground level and fall.* - Unsolved bug. Positions which work sometimes can fail at others. If no amount of Jumping to Position works, turn on no-clip mode and try again. This usually fixes it even if you turn off noclip mode straight away without giving yourself additional vertical height off the ground.
*Skies look wrong.* - The original Wizardry skies aren't being used here - just a static one the same for all levels, even Cosmic Circle and underwater.
*Can walk off edge of map.* - The usual edge of map teleports aren't implemented. If you wish to swap maps, load it manually using the toolbar. This behaviour does let you see some parts of maps you can't normally get to, but don't advise saving positions in these areas unless you're prepared for unexpected results loading it in the real game.
*Sometimes you're blocked by a shadow or lightbeam.* - Some textures that shouldn't be are treated as solid. I'll fix these as I identify them.
*Positions changed in the editor sometimes don't work with the real game. Various problems.* ie.
   * *You drop every time you load* - if this is the worst that happens ignore it and think yourself lucky. If it bothers you too much save your position or reset your portal within the game and that will fix the co-ordinates properly. If you write the numbers it saves down (the ones in the textboxes in the editor) you can enter them manually in future in order to accurately get that position, rather than deal with the inaccuracies introduced by the Navigator.
   * *You get stuck in an object not visible in the editor or not-collidable in the editor* - reload in the editor and nudge your x and z co-ordinates in one direction or another until you're out of trouble.
   * *You get stuck in the floor, the wall, the roof or fall to your death through any of them* - reload in the editor and nudge your y co-ordinate up if it it's a stuck in floor or falling problem, otherwise nudge the x and z co-ordinates in one direction or another until you're out of trouble.
*Urho3D engine discontinued.* - And the support forums with it. :-( At some point the codebase will need to swap to one of the forks. Just not for this release.

Prebuilt Windows binaries provided:

Wizardry8Editor_win64_DirectX.exe    MD5:9c794072dcdbd0ff357c5094ed35fd56
Wizardry8Editor_win64_OpenGL.exe     MD5:ff92a113866c99b2b0217eca569fc769


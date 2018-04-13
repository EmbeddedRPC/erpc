Building erpcgen for Windows using Visual Studio
================================================


Requirements
------------

1. Visual Studio Express 2013 or later.

    This version is required for the C++11 support.

2. Win flex-bison

    Direct link to the latests win flex-bison release download:
        [https://sourceforge.net/projects/winflexbison]

    Extract win flex-bison zip contents directly into the erpc/erpcgen/VisualStudio_v14 directory
    (not into a subdirectory).

3. boost [http://boost.org]

    Direct link to the boost 1.66 release downloads:
        [https://sourceforge.net/projects/boost/files/boost-binaries/1.66.0/]

    Install binary for your version of visual studio (x86 architecture)
    into current directory (for Visual Studio 2017: boost_1_66_0-msvc-14.1-32.exe):
        erpc\erpcgen\VisualStudio_v14\

        whole path has to be:
        erpc\erpcgen\VisualStudio_v14\boost_1_66_0\

    For different version of Visual Studio boost library you need update project:
        in Properties\Linker\General update Additional Library Directories

Build output
------------

Visual Studio build output is available in these directories:

    erpc/erpcgen/VisualStudio_v14/Debug
    erpc/erpcgen/VisualStudio_v14/Release


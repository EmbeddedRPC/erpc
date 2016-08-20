Building erpcgen for Windows using Visual Studio
================================================


Requirements
------------

1. Visual Studio Express 2013 or later.

    This version is required for the C++11 support.

2. Win flex-bison

    Direct link to the latests win flex-bison release download:
        [http://sourceforge.net/p/winflexbison]

    Extract win flex-bison zip contents directly into the erpc/erpcgen/VisualStudio_v12 directory
    (not into a subdirectory).

3. boost [http://boost.org]

    Direct link to the boost 1.57 release downloads:
        [https://sourceforge.net/projects/boost/files/boost/1.57.0/]

    Extract and install boost 1.57 into C:\boost_1_57_0\.

    Run these commands in the boost install directory to build boost using Visual Studio 2013:
        1.  bootstrap.bat
        2.  for x86 (32-bit):
                b2 --toolset=msvc-12.0 --build-type=complete stage
            for x64 (64-bit):
                b2 --toolset=msvc-12.0 --build-type=complete architecture=x86 address-model=64 stage

        For release build, extend second command with:
            runtime-link=static runtime-debugging=off variant=release


Build output
------------

Visual Studio build output is available in these directories:

    erpc/erpcgen/VisualStudio_v12/Debug
    erpc/erpcgen/VisualStudio_v12/Release


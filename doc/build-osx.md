Mac OS X Build Instructions and Notes
====================================
This guide will show you how to build Infinitum Core for OS X.

Notes
-----

* Tested on OS X 10.7 through 10.11 on 64-bit Intel processors only.

* All of the commands should be executed in a Terminal application. The
built-in one is located in `/Applications/Utilities`.

Preparation
-----------

You need to install Xcode with all the options checked so that the compiler
and everything is available in /usr not just /Developer. Xcode should be
available on your OS X installation media, but if not, you can get the
current version from https://developer.apple.com/xcode/. If you install
Xcode 4.3 or later, you'll need to install its command line tools. This can
be done in `Xcode > Preferences > Downloads > Components` and generally must
be re-done or updated every time Xcode is updated.

You will also need to install [Homebrew](http://brew.sh) in order to install library
dependencies.

The installation of the actual dependencies is covered in the instructions
sections below.

Instructions: Homebrew
----------------------

#### Install dependencies using Homebrew

    brew install autoconf automake berkeley-db4 libtool boost miniupnpc openssl pkg-config protobuf qt5 libevent

NOTE: Building with Qt4 is still supported, however, could result in a broken UI. As such, building with Qt5 is recommended.

### Building `infinitum`

1. Clone the GitHub tree to get the source code and go into the directory.

        git clone https://github.com/infinitum/infinitum.git
        cd infinitum

2.  Build infinitum-core:
    This will configure and build the headless infinitum binaries as well as the gui (if Qt is found).
    You can disable the gui build by passing `--without-gui` to configure.

        ./autogen.sh
        ./configure
        make

3.  It is also a good idea to build and run the unit tests:

        make check

4.  (Optional) You can also install infinitumd to your path:

        make install

Use Qt Creator as IDE
------------------------
You can use Qt Creator as IDE, for debugging and for manipulating forms, etc.
Download Qt Creator from https://www.qt.io/download/. Download the "community edition" and only install Qt Creator (uncheck the rest during the installation process).

1. Make sure you installed everything through Homebrew mentioned above
2. Do a proper ./configure --enable-debug
3. In Qt Creator do "New Project" -> Import Project -> Import Existing Project
4. Enter "infinitum-qt" as project name, enter src/qt as location
5. Leave the file selection as it is
6. Confirm the "summary page"
7. In the "Projects" tab select "Manage Kits..."
8. Select the default "Desktop" kit and select "Clang (x86 64bit in /usr/bin)" as compiler
9. Select LLDB as debugger (you might need to set the path to your installation)
10. Start debugging with Qt Creator

Creating a release build
------------------------
You can ignore this section if you are building `infinitumd` for your own use.

infinitumd/infinitum-cli binaries are not included in the Infinitum-Qt.app bundle.

If you are building `infinitumd` or `Infinitum Core` for others, your build machine should be set up
as follows for maximum compatibility:

All dependencies should be compiled with these flags:

 -mmacosx-version-min=10.7
 -arch x86_64
 -isysroot $(xcode-select --print-path)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk

Once dependencies are compiled, see [doc/release-process.md](release-process.md) for how the Infinitum Core
bundle is packaged and signed to create the .dmg disk image that is distributed.

Running
-------

It's now available at `./infinitumd`, provided that you are still in the `src`
directory. We have to first create the RPC configuration file, though.

Run `./infinitumd` to get the filename where it should be put, or just try these
commands:

    echo -e "rpcuser=infinitumrpc\nrpcpassword=$(xxd -l 16 -p /dev/urandom)" > "/Users/${USER}/Library/Application Support/Infinitum/infinitum.conf"
    chmod 600 "/Users/${USER}/Library/Application Support/Infinitum/infinitum.conf"

The next time you run it, it will start downloading the blockchain, but it won't
output anything while it's doing this. This process may take several hours;
you can monitor its process by looking at the debug.log file, like this:

    tail -f $HOME/Library/Application\ Support/Infinitum/debug.log

Other commands:
-------

    ./infinitumd -daemon    # to start the infinitum daemon.
    ./infinitum-cli --help  # for a list of command-line options.
    ./infinitum-cli help    # When the daemon is running, to get a list of RPC commands

Using Qt official installer while building
------------------------------------------

If you prefer to use the latest Qt installed from the official binary
installer over the brew version, you have to make several changes to
the installed tree and its binaries (all these changes are contained
in the brew version already). The changes needed are described in
[#7714](https://github.com/infinitum/infinitum/issues/7714). We do not
support building Infinitum Core this way though.

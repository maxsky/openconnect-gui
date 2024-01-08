### Building release version and create a package 

The main development activities can be found in 'main', in 'feature/...' or other similarly named branches.

#### Version scheme

and main programming activities are in 'main' or 'feature/*' branches.
Version string auto=generated on compilation from Git info into following format:

    <major>.<minor>.<patch>[-rev_count-sha1][-dirty]

#### Building packages

Always check if you have committed all changes or move work-in-progress work into stash!!
Following steps illustrate how to create application:

    $ git clone https://gitlab.com/openconnect/openconnect-gui
    $ cd openconnect-gui

To build a release package, review released changes in `CHANGELOG.md`,
update planned release version in `CMakeLists.txt` and start a release
process with target tag:

    $ git checkout main
    $ git tag vX.Y.Z
    $ git push main
    $ git push --tags

And then, continue with compilation process:

    $ cd ..
    $ md build-release
    $ cd build-release
    $ cmake -DCMAKE_BUILD_TYPE=Release -G"MinGW Makefiles" ..\openconnect-gui
    $ mingw32-make -j5
    $ mingw32-make package




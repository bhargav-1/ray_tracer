This overlays folder is important. We want to use vcpkg for cpp libraries
that aren't in apt etc, but we don't want to use it for things like qt as
it'll download the source and then build it from scratch.

No point doing that when we can just get it from apt.

Unfortunately, vcpkg hides system dependencies, so we need to trick it into
using them:

https://devblogs.microsoft.com/cppblog/using-system-package-manager-dependencies-with-vcpkg/

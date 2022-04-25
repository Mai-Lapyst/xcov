# Xcov
xcov is an coverage generator c/c++ projects

## License
xcov is licensed under the AGPLv3 license. For the complete license, please see the `LICENSE` file.

## Dependencys
- [nlohmann json](https://github.com/nlohmann/json)
- [inja](https://github.com/pantor/inja)
- [GNU source-highlight](https://www.gnu.org/software/src-highlite/source-highlight.html)
- `boost::process` (`boost::filesystem`), `boost::asio`, `boost/algorithm/string.hpp`
- gcc's `gcov` binary
- `gzip` (to decompress `.gz` files)

## How to use
xcov can generate html coverage reports.

To do this you need a bit of perperation; you need to compile your code to generate coverage files.
For gcc this is easy done by adding the `--coverage` flag when invoking `gcc`.
You then need to actually run your binary once to generate the relevant runtime data.

Once this is done, you can invoke xcov: `xcov -o reports`; this will generate html reports for all your sourcefiles
inside the folder `reports`.

If you know where your `.gcda` and `.gcno` files lay, you also can restrict xcov's search by using `--root`.
For example let's say we have an `build` dir where all `.o`, `.gcda` and `.gcno` files are.
You now can run `xcov -r build -o reports` instead, and xcov will only search in `build`.

## Configuration files
Nearly all commandline switches can also be configured via a config file.

xcov searches following paths for an config file (and also applies them in this order):
- The systems configuration dir (`/usr/local/etc/xcov/default.cfg` when installed through `make install-local`)
- The user's configuration dir (`$HOME/.config/xcov/user.cfg`)
- xcov searches all parent directories from it's invokation directory for `xcov.cfg`.
    These files are included top-to-bottom, top being here the system root and bottom the current directory. This makes it possible to invoke xcov in subfolders of your project and still getting the correct config.
- The `--config` argument
- All other cli flags

The format is an simple `.ini` style syntax:
```ini
# Lines with an hashtag in front of it are line-comments; they are ignored

# These are sections; currently there are two supported: 'main' and 'source_highlight'
[main]

# Inside an section there are key=value pairs. The value MUST be quoted, but the spaces around the equal sign
# are optional. Note that the only thing allowed after the value are spaces or comments.
title = "Code Coverage Report"
```

For documentation what keys are available, please have a look into `src/config.cpp`.
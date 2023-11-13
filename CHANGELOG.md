# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- [README.md](README.md), [CHANGELOG.md](CHANGELOG.md) and [LICENSE](LICENSE),
  replacing three original README files and DICE-LICENSE.TXT. The new files are
  named and formatted according to current standard practice for GitHub-hosted
  projects.

### Changed

- Tabs to spaces. I realise this may upset some people, but DICE source code is
  consistent that the indentation is 4 spaces - but that every 8 spaces gets
  collapsed into a single tab. This style of identation is uncommon these days
  and VSCode (and likely many other IDEs and editors) does not handle it
  cleanly.

### Fixed

- Compiles on Linux, specifically Ubuntu 20.04 and gcc 9.4.0 - also still
  compiles on DragonFly BSD.
- Adds .gitignore files


## [1.15] - 2023-10-17

### Changed

- Change the primary copyright to the BSD license.
  Note that this distribution may still contain a few Commodore files
  and other files that Commodore made freely distributable, and I
  can't 100% vouch for those.  But all the C code and header files
  outside of the Commodore .fd files were written by Matthew Dillon,
  John Toebes, or Bryce Nesbitt.

### Fixed

- Do some cleanups so the dice suite compiles cleanly.  Mostly related
  to uses of sprintf() and strncpy() and such.  Includes a fix or
  two for cases that I don't think the compiler even hits (at least
  not in its original Amiga version).


## [1.14] - 2015-12-28

### Added

- Port DICE to 64-bit unix environments.  For the most part a shitload of
  cases where I assumed that 'long' (in the Amiga days) meant 32-bits.  Of
  course, it isn't on a 64-bit DragonFlyBSD, other BSD, or linux.

### Fixed

- Don't crash dobj on bad opcodes.
- Translate longs and quads with -CDTOD from machine-native C.

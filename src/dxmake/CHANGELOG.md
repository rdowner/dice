# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
as of the next released version.

## [Unreleased]

### Added

- Reinstate Amiga support. DCC can now build this for the Amiga and it works as
  expected.

### Changed

- Tabs to spaces. I realise this may upset some people, but DICE source code is
  consistent that the indentation is 4 spaces - but that every 8 spaces gets
  collapsed into a single tab. This style of identation is uncommon these days
  and VSCode (and likely many other IDEs and editors) does not handle it
  cleanly.

### Fixed


## 2.13

### Fixed

Fix GCC warnings.

### Changed

Re-license as BSD


## 2.12

### Fixed

Fix a bug for the following sequence:

    install: target1 target2

    target1: objectX
	...

    target2: objectX
	...

In this case target2 was not being executed if it did not exist.


## 2.11

### Fixed

Fix a nasty bug when reassigning a variable in the form: `X= $(X) a b c`


## 2.10

### Fixed

Conditionalize use of stat `st_gen`, linux doesn't implement the field.


## 2.9

### Changed

Create a port for dmake, rename dmake to dxmake to avoid a port
conflict with someone else's dmake.

Remove old AMIGA ifdefs

### Added

Add `.ifhost`, `.ifos`, and `.ifarch` conditionals


## 2.8

### Fixed

Save and restore TOPDIR when running the .include directive in
order to support multiple .include directives.

Do not misinterpret .. and ./ as dot directives.

Add `.iffile` directive

When multiple dependancies for a target exist, and one is stale,
run ALL the related commands.  In the example below we would run
cmds A and cmds B if any dependancy is stale.

    $(OBJS) : $(OD)
        cmds A....

    $(OBJS) : $(SRCS)
        cmds B....

When we execute a dependancy set, if the target file exists we
check whether the timestamp has changed or not.  If it has not
changed we assume a passive update.. that is, a situation such
as with prototype generation where the rule executed but no
change was made.  When this case occurs we touch the target file
to update its timestamp and flag the dependancy as a passive update.
This may propogate and touch other targets (such as object files).


## 2.7

### Fixed

Fix the list ordering so commands for this situation:

    $(OBJS) : $(OD)
        cmds....

    $(OBJS) : $(SRCS)
        cmds....

Are executed in the proper order.


## 2.6

### Added

Added `.include <path>`

Added automatic `$(TOP)` calculation for `.include` sequences

### Changed

Rewrote dependancy handler


## 2.2

### Fixed

Found bug in parse.c relating to symbol buffers, would screw up
symbolic replace in multiple-dependancy line.

# Process Memory Access Library

**Process Memory Access** library (**PMA**) allows your application to **read** a memory of an external process running on your system. Works similarly as [CheatEngine](https://www.cheatengine.org/). It is a base of ... my second project for adding Achievements/Challenges to older video games.

> Legal Notice: Reading a memory of a process is in many cases considered a **violation** of *terms of service* or *end-user license agreement*.
Doing so for a personal use with no malicious intend is something of a gray area.

## Requirements
- Some windows SDK shit
- Library has to be run with **Admin privileges**


## Installation
TODO
get repo
run cmake + build
get pma.lib + include folder
done

## Supported platforms
Windows

## How it works
PMA library attaches to a specified process and acts in the same mode as *debuggers* do. That allows the PMA to access memory regions belonging to the process.

> [!CAUTION]
> Do NOT attach to any software that might monitor external accesses to its process space (i.e. Games for cheating). This library might be viewed by such software as a malicious actor.

### Features
- Attach via ProcessId/WindowHandle
- Specify what modules (exe, dll) you want to read
	- Read as an offset from beginning of their memory space
- MultiLevelPointers (with offset on each level)

### TODOs
- Make it work also for .exe
- Figure out if it is necessary to use ReadProcessMemory or if it is possible to read memory directly since we load the dlls into the our process space
- when read big chunk of memory, translate pointers from process memory space into locally stored read memory (to not force reading again and again)... kinda difficult -> not in the beginning
- Full memory snapshot

# Achi App
This repository serves as a base for game extension applications. It uses **link to LIB** to access game process' memory which allows to read game's state.
<h1 align="center">HobShell</h1>

<p align="center">
  An interactive <strong>UEFI shell</strong> that runs directly on firmware — before any operating system.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-x86--64%20UEFI-blue" alt="platform">
  <img src="https://img.shields.io/badge/language-C-00599C?logo=c&logoColor=white" alt="C">
  <img src="https://img.shields.io/badge/toolchain-clang%20%2B%20lld-orange" alt="toolchain">
  <img src="https://img.shields.io/badge/run-QEMU%20%2B%20OVMF-6f42c1" alt="qemu">
</p>

---

<p align="center">
  <img src="./screenshots/HobShell.png" alt="hobbyOS kernel rendering to the framebuffer" width="85%">
</p>

---

HobShell boots straight into a `boot>` prompt where you can explore the firmware
environment: browse the boot volume, dump the UEFI memory map, and inspect the
graphics output protocol. It's a small, freestanding UEFI application — a handy
starting point for firmware experiments and OS-dev bring-up.

```text
Welcome to HobbyOS bootloader shell.
boot>help
Commands:
  help    - show this help
  clear   - clear the screen
  ls      - list files
  print   - print text
  memmap  - show memory map
  gopinfo - show Graphics Output Protocol info
boot>ls
Files:
[DIR]  EFI
       kernel.elf (24576 bytes)
boot>
```

## Features

- **Interactive line editor** — `readline` with backspace and input flushing
- **Extensible command table** — register a command with a single array entry
- **`printf`-style formatter** (`VSPrint`) — `%d %u %x %s %c`, plus the `l`
  modifier (`%lx`, `%lu`) for 64-bit values
- **Built-in commands**
  | Command | Description |
  |---------|-------------|
  | `ls [path]` | list the boot volume (long format with sizes) |
  | `memmap` | decode and print the UEFI memory map |
  | `gopinfo` | resolution, pixel format, and framebuffer of the active GOP |
  | `print` · `clear` · `help` | basics |

## Quick start

**Prerequisites**

| Tool | Purpose | Arch package |
|------|---------|--------------|
| `clang` + `lld` | cross-compile to a PE UEFI image | `clang lld` |
| gnu-efi headers | UEFI type/protocol definitions (`/usr/include/efi`) | `gnu-efi` |
| `qemu-system-x86_64` | run the image | `qemu-full` |
| OVMF firmware | UEFI firmware for QEMU | `edk2-ovmf` |

**Build**

```sh
make
```

Produces `BOOTX64.EFI` and stages it at `diskimg/EFI/BOOT/BOOTX64.EFI`, where
UEFI firmware looks for a bootable image on a removable volume.

**Run**

The `run` target expects the OVMF firmware images in a local `OVMF/` directory
(they're gitignored — supply them from your distro):

```sh
mkdir -p OVMF
cp /usr/share/edk2/x64/*.fd OVMF/
make run
```

QEMU boots OVMF, which launches `BOOTX64.EFI` from the `diskimg` FAT volume and
drops you at the `boot>` prompt.

> Firmware paths vary by distro — `/usr/share/OVMF` on Debian/Ubuntu,
> `/usr/share/edk2/ovmf` on Fedora — and files may be named `OVMF_CODE_4M.fd`.
> `OVMF_CODE` is read-only; keep a **per-project writable copy** of `OVMF_VARS`.

## How it works

Two layers sit on top of the firmware:

- a **freestanding UEFI runtime** — memory helpers, a `readline` line editor,
  and a small `printf`-style formatter, with no libc linked in; and
- the **shell** — a REPL that splits each line, matches the first word against
  a command table, and dispatches to its handler.

Commands are decoupled from the core, so adding one is just a handler plus a
row in the table — and `help` reflects whatever is registered.

## One binary, two roles

A single build flag decides what HobShell *is*:

```sh
make                     # standalone UEFI shell (default)
make WITH_LOADKERNEL=1   # + `load`: an ELF64 kernel loader with a boot handoff
```

With `WITH_LOADKERNEL=1`, the `load` command and its boot protocol compile in —
that's how [hobbyOS](https://github.com/Soham-Kakkar/hobbyOS) reuses the shell
as its kernel bootloader (it supplies the boot-protocol header the loader needs).

## Related

HobShell is also the foundation of the bootloader in
**[hobbyOS](https://github.com/Soham-Kakkar/hobbyOS)**, where it grows an ELF64
kernel loader and a boot-info handoff. This repository is the standalone shell:
it depends on no kernel or OS headers and builds on its own.

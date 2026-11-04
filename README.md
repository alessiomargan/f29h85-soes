# f29h85-soes

EtherCAT SOES application for TI F29H85x CPU1 on the controlCARD. The image is
linked for the application slot owned by `bl-f29h85x-ecat` and supports a warm
handoff to that bootloader for File-over-EtherCAT (FoE) updates.

## Repository setup

Clone both pinned dependencies:

```bash
git clone --recurse-submodules https://github.com/alessiomargan/f29h85-soes.git
cd f29h85-soes
```

For an existing clone:

```bash
git submodule update --init --recursive
```

`external/SOES` supplies the EtherCAT stack. `external/uc-test` supplies the
application object dictionary, hooks, parameters, globals, and build metadata.
CCS and CMake use these repository-local paths; no sibling checkout is needed.

No `external/uc-test/f29h85x` directory is required. F29H85x-specific code
belongs to this repository, while only the platform-neutral
`external/uc-test/common_src/soes_test` sources are compiled.

Application-owned C sources and headers live in `src/`. SysConfig input,
linker command files, build wrappers, and target configuration files remain at
the project root so CCS and standalone CMake use the same project assets.

## Build

### Code Composer Studio

Import the project and build configuration `APP_CC`. This is the only board
configuration: F29H85x is used with the controlCARD.

`APP_CC` produces `f29h85-soes.out` and a flat `f29h85-soes.bin`. The flat
binary is for the FoE packer; it is not a ROM-boot certificate image and must
not be programmed at the bootloader address.

### CMake with TI C29 Clang

CMake is supported independently of CCS and uses the same sources and linker
command file:

```bash
make build
```

The default tools are under `/home/amargan/ti`; override them when needed:

```bash
make build \
  TI_CLANG_ROOT=/path/to/ti-cgt-c29 \
  MCU_SDK_ROOT=/path/to/f29h85x-sdk \
  SYSCONFIG_ROOT=/path/to/sysconfig
```

## Create the FoE image

```bash
make foe-package
```

This performs a clean CMake rebuild, packages `build_vscode/f29h85-soes.bin`
as `f29h85x.bin`, and installs it by default at:

```text
~/.ecat_master/firmware/f29h85x.bin
```

Useful overrides are:

```bash
make foe-package \
  APP_BIN=/path/to/f29h85-soes.bin \
  FOE_IMAGE=f29h85x.bin \
  FW_BIN_DIR=/path/to/master/firmware
```

The packaged stream is compact: a 32-byte manifest followed by the actual
application payload, without full-slot padding. The manifest contains the
target identifier, payload length, load address, payload CRC, and header CRC.
The bootloader uses FoE password `0xF290`, erases only the required payload
sectors, and commits the image only after full validation.

## Memory contract

| Region | Address range | Owner |
|---|---:|---|
| ROM certificate and bootloader | `0x10000000-0x1003FFFF` | `bl-f29h85x-ecat` |
| Application payload | `0x10040000-0x101FDFFF` | This project |
| Manifest | `0x101FE000-0x101FEFFF` | FoE bootloader |
| Commit sector | `0x101FF000-0x101FFFFF` | FoE bootloader |
| Retained request | `0x200FFFF8-0x200FFFFF` | Shared NOINIT handoff |

Application codestart is fixed at `0x10040000`. The application and bootloader
linker files must keep the retained request at the identical SRAM address.

## Update handoff

When an update is requested, `jump_to_bootloader()` disables interrupts, writes
the magic value and its inverse into retained SRAM, and enters bootloader
codestart at `0x10001000`. The bootloader consumes and clears the request,
preserves the integrated ESC for the warm transition, and stays in recovery
until a valid committed image is available.

## Codestart source

The CCS project links `codestartbranch.asm` from the installed F29H85x SDK.
CMake compiles the same SDK source explicitly. Do not add another local copy
unless the SDK-linked source is removed, or `code_start` will be defined twice.

## Related project

The matching bootloader is `bl-f29h85x-ecat`. Its flash map and image format
must remain synchronized with `f29h85x_flash_lnk.cmd` and
`tools/pack_foe_image.py` in this repository.

#!/usr/bin/env python3
# flash.py - flash the firmware via J-Link
# copyright Grant Freese 2026
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Wraps JLinkExe with the repo's CommandFile.jlink: checks the ELF exists, suppresses
# the JLinkExe transcript on success, and spills it on failure. After programming, the
# command file resets the MCU so the new firmware boots immediately.
# Run from anywhere; paths resolve relative to the repo root.

import argparse
import pathlib
import subprocess
import sys
import time

DEFAULT_COMMAND_FILE = "CommandFile.jlink"

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent


def main() -> int:
    parser = argparse.ArgumentParser(description="Flash the firmware via J-Link.")
    parser.add_argument(
        "--command-file",
        default=DEFAULT_COMMAND_FILE,
        help=f"JLinkExe command file, relative to repo root (default: {DEFAULT_COMMAND_FILE})",
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Print the raw JLinkExe transcript. Default suppresses on success "
             "and prints only on flash failure.",
    )
    args = parser.parse_args()

    command_file = REPO_ROOT / args.command_file
    elf = REPO_ROOT / "build" / "bin" / "app.elf"

    if not elf.is_file():
        print(f"flash.py: {elf} not found -- run 'ninja -C build' first", file=sys.stderr)
        return 1
    if not command_file.is_file():
        print(f"flash.py: {command_file} not found", file=sys.stderr)
        return 1

    # -NoGui 1 suppresses the GTK flash-progress dialog; -ExitOnError 1 makes JLinkExe
    # exit non-zero on a script error so subprocess sees it. Capture the transcript so
    # success can elide it; -v re-enables it.
    jlink_cmd = ["JLinkExe", "-NoGui", "1", "-ExitOnError", "1", "-CommandFile", str(command_file)]
    jlink_start = time.monotonic()
    if args.verbose:
        result = subprocess.run(jlink_cmd, cwd=str(REPO_ROOT))
    else:
        result = subprocess.run(jlink_cmd, cwd=str(REPO_ROOT), capture_output=True, text=True)
    jlink_elapsed = time.monotonic() - jlink_start

    if not args.verbose:
        if result.returncode != 0:
            if result.stdout:
                sys.stdout.write(result.stdout)
            if result.stderr:
                sys.stderr.write(result.stderr)
            print(f"flash.py: JLinkExe exited {result.returncode} after {jlink_elapsed:.2f} s", file=sys.stderr)
        else:
            print(f"flash.py: flashed in {jlink_elapsed:.2f} s")

    return result.returncode


if __name__ == "__main__":
    sys.exit(main())

# sysupdate

A lightweight, high-performance system update utility for Arch Linux written in pure C. It consolidates multiple package manager tasks (`pacman`, `paru / yay`, `flatpak`) into a single fast binary execution, saving machine overhead compared to conventional shell scripts.

## Features

* **Minimal Footprint:** Native compiled machine code utilizing POSIX system execution.
* **Pre-flight Checks:** Automatically tests for active `sudo` authentication tokens and network connectivity before kicking off system-wide compiles.
* **Safer Execution:** Implements strict error-handling loops; stops gracefully if primary package sync mirrors fail.
* **Flexible Actions:** Allows choice of a machine shutdown sequence, machine reboot, or diagnostic checks.

## Compilation

Compile the binary locally using standard C optimization flags via `gcc`:

```bash
gcc -Wall -Wextra -O2 sysupdate.c -o sysupdate
```
## Installation

Move the binary to your `$PATH`:

```bash
sudo mv sysupdate /bin/
```

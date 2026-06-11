#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/types.h>

// ── Colors ────────────────────────────────────────────────────────────────────
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[0;36m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"

// Global tag name
const char *tag = "sysupdate";

// ── Helpers ────────────────────────────────────────────────────────────────────
void log_info(const char *msg) { printf("%s%s[%s]%s %s\n", CYAN, BOLD, tag, RESET, msg); }
void success(const char *msg)  { printf("%s%s  ✔%s  %s\n", GREEN, BOLD, RESET, msg); }
void warn(const char *msg)     { printf("%s%s  ⚠%s  %s\n", YELLOW, BOLD, RESET, msg); }
void error(const char *msg)    { fprintf(stderr, "%s%s  ✘%s  %s\n", RED, BOLD, RESET, msg); }
void step(const char *msg)     { printf("\n%s━━━  %s  ━━━%s\n", BOLD, msg, RESET); }

// Print help menu
void print_help(const char *prog_name) {
    printf("Usage: %s [options]\n\n", prog_name);
    printf("Options:\n");
    printf("  -h, --help      Show this help message and exit\n");
    printf("  -s, --shutdown  Run updates, then shut down the system (default)\n");
    printf("  -r, --reboot    Run updates, then reboot the system\n");
}

// Checks if an executable command exists in the user's PATH
bool command_exists(const char *cmd) {
    char check_cmd[128];
    snprintf(check_cmd, sizeof(check_cmd), "command -v %s >/dev/null 2>&1", cmd);
    return (system(check_cmd) == 0);
}

// ── Verify sudo privileges availability ────────────────────────────────────────
void check_sudo(void) {
    if (system("sudo -v >/dev/null 2>&1") != 0) {
        error("Could not obtain sudo privileges. Aborting.");
        exit(1);
    }
}

// ── Verify network connectivity ────────────────────────────────────────────────
void check_network(void) {
    if (system("ping -c1 -W3 archlinux.org >/dev/null 2>&1") != 0) {
        warn("No internet connection detected. Continuing anyway...");
    } else {
        success("Internet connection OK.");
    }
}

// ── Pacman ────────────────────────────────────────────────────────────────────
void update_pacman(void) {
    step("Pacman — System Update");
    if (system("sudo pacman -Syu --noconfirm") == 0) {
        success("pacman completed.");
    } else {
        error("pacman failed. Aborting.");
        exit(1);
    }
}

// ── AUR (paru / yay) ──────────────────────────────────────────────────────────
void update_aur(void) {
    step("AUR Update");
    if (command_exists("paru")) {
        log_info("Found paru. Starting AUR update...");
        if (system("paru -Syu --noconfirm") == 0) {
            success("paru completed.");
        } else {
            warn("paru failed. Continuing...");
        }
    } else if (command_exists("yay")) {
        log_info("Found yay. Starting AUR update...");
        if (system("yay -Syu --noconfirm") == 0) {
            success("yay completed.");
        } else {
            warn("yay failed. Continuing...");
        }
    } else {
        warn("Neither paru nor yay found. Skipping AUR.");
    }
}

// ── Flatpak ───────────────────────────────────────────────────────────────────
void update_flatpak(void) {
    step("Flatpak — Application Update");
    if (command_exists("flatpak")) {
        if (system("flatpak update -y") == 0) {
            success("flatpak completed.");
        } else {
            warn("flatpak failed. Continuing...");
        }
    } else {
        warn("flatpak not found. Skipping.");
    }
}

// ── Action (Shutdown or Reboot) ───────────────────────────────────────────────
void finish_action(bool reboot_mode) {
    if (reboot_mode) {
        step("Rebooting the system");
        log_info("The system will reboot in 5 seconds... (Ctrl+C to cancel)");
        sleep(5);
        system("sudo reboot now");
    } else {
        step("Shutting down the system");
        log_info("The system will shut down in 5 seconds... (Ctrl+C to cancel)");
        sleep(5);
        system("sudo shutdown now");
    }
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main(int argc, char *argv[]) {
    bool reboot_mode = false;

    static struct option long_options[] = {
        {"help",     no_argument, 0, 'h'},
        {"shutdown", no_argument, 0, 's'},
        {"reboot",   no_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "hsr", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                print_help(argv[0]);
                return 0;
            case 's':
                reboot_mode = false;
                break;
            case 'r':
                reboot_mode = true;
                break;
            default:
                print_help(argv[0]);
                return 1;
        }
    }

    tag = reboot_mode ? "rupdate" : "supdate";

    printf("\n%s╔══════════════════════════════════════╗%s\n", BOLD, RESET);
    printf("%s║      %s — Update + %s       ║%s\n", BOLD, tag, reboot_mode ? "Reboot" : "Shutdown", RESET);
    printf("%s╚══════════════════════════════════════╝%s\n\n", BOLD, RESET);

    check_sudo();
    check_network();
    update_pacman();
    update_aur();
    update_flatpak();

    printf("\n%s%sAll updates completed successfully.%s\n", GREEN, BOLD, RESET);

    finish_action(reboot_mode);

    return 0;
}

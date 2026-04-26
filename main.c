#include "city_commands.h"

int main(int argc, char *argv[]) {
    AppContext context;

    parse_arguments(argc, argv, &context);

    if (strlen(context.role) == 0 || strlen(context.user) == 0 || strlen(context.command) == 0) {
        fprintf(stderr, "Eroare: Argumente obligatorii lipsa.\n");
        return 1;
    }

    if (!setup_and_log_action(&context)) {
        return 1;
    }

    if (strcmp(context.command, "add") == 0) {
        add_report(&context);
    } else if (strcmp(context.command, "list") == 0) {
        list_reports(&context);
    } else {
        printf("comanda '%s' inca nu este implementata\n", context.command);
    }

    return 0;
}
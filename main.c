#include "city_commands.h"

int main(int argc, char *argv[]) {
    AppContext context;

    parse_arguments(argc, argv, &context);

    if (strlen(context.role) == 0 || strlen(context.user) == 0 || strlen(context.command) == 0) {
        fprintf(stderr, "Eroare: Argumente obligatorii lipsa.\n");
        return 1;
    }

    verify_district_symlink(&context);
    // check for dagling syslinks

    if (!setup_and_log_action(&context)) {
        return 1;
    }

    if (strcmp(context.command, "add") == 0) {
        add_report(&context);
    } else if (strcmp(context.command, "list") == 0) {
        list_reports(&context);
    } else if (strcmp(context.command, "view") == 0) {
        view_report(&context);
    } else if (strcmp(context.command, "remove_report") == 0) {
        remove_report(&context);
    } else if (strcmp(context.command, "update_threshold") == 0) {
        update_threshold(&context);
    } else if (strcmp(context.command, "filter") == 0) {
        filter_reports(&context, argc, argv);
    } else if(strcmp(context.command, "remove_district") == 0) {
        remove_district(&context);
    } else {
        printf("Comanda \"%s\" e invalida.\n", context.command);
    }

    return 0;
}
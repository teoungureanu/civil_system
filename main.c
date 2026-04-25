#include "city_commands.h"

int main(int argc, char *argv[]) {
    AppContext context;
    parse_arguments(argc, argv, &context);
    printf("Succesfully compiled\n");

    return 0;
}

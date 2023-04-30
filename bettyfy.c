#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 2048
#define PROGRESS_BAR_WIDTH 20

int main(int argc, char *argv[]) {

    // Check that the program was given a filename to format
    if (argc < 2) {
        printf("Usage: %s file_to_format\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Build the clang-format command to apply to the file
    char command[MAX_COMMAND_LENGTH] = {0};
    snprintf(command, MAX_COMMAND_LENGTH, "clang-format -style=\"{BasedOnStyle: Google, UseTab: Always, IndentWidth: 8, ColumnLimit: 80, AccessModifierOffset: -8, AlignConsecutiveAssignments: false, AlignConsecutiveDeclarations: false, AlignEscapedNewlines: Left, AllowAllParametersOfDeclarationOnNextLine: false, AllowShortBlocksOnASingleLine: false, AllowShortCaseLabelsOnASingleLine: false, AllowShortFunctionsOnASingleLine: None, AllowShortIfStatementsOnASingleLine: false, AllowShortLoopsOnASingleLine: false, AllowShortFunctionsOnASingleLine: InlineOnly, AlwaysBreakAfterReturnType: All, AlwaysBreakBeforeMultilineStrings: false, AlwaysBreakTemplateDeclarations: true, BinPackArguments: true, BinPackParameters: true, BraceWrapping: { AfterClass: true, AfterControlStatement: true, AfterEnum: true, AfterFunction: true, AfterNamespace: true, AfterStruct: true, AfterUnion: true, BeforeCatch: true, BeforeElse: true, IndentBraces: false, SplitEmptyFunction: true }, BreakBeforeBinaryOperators: false, BreakBeforeBraces: Allman, BreakBeforeInheritanceComma: true, BreakBeforeTernaryOperators: true, KeepEmptyLinesAtTheStartOfBlocks: true}\" -i %s", argv[1]);

    printf("Formatting file %s...\n", argv[1]);

    // Get the size of the file
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0L, SEEK_END);
    long file_size = ftell(file);
    char* file_buffer = malloc(file_size + 1);
    fseek(file, 0L, SEEK_SET);
    fread(file_buffer, 1, file_size, file);
    fclose(file);
    file_buffer[file_size] = '\0';

    // Run the clang-format command and show a progress bar
    printf("Formatting file...");
    fflush(stdout);
    int progress = 0;
    int chars_read = 0;
    int chars_per_step = file_size / (PROGRESS_BAR_WIDTH * 5); // Update progress bar every 10,000 characters
    for (int i = 0; i < file_size; i++) {
        if (chars_read >= chars_per_step) {
            system("clear"); // Clear the console to update the progress bar
            progress = ((i + chars_per_step) * PROGRESS_BAR_WIDTH) / file_size;
            printf("[%.*s%*s] %ld%%", progress, "####################", PROGRESS_BAR_WIDTH - progress, "", (i + chars_per_step) * 100 / file_size);
            fflush(stdout);
            chars_read = 0;
        }
        chars_read++;
    }
    system("clear"); // Clear the console to show progress bar at 100%

    // Ask the user if they want to preview the formatted code
    printf("\nPreview the formatted code? (Y/n): ");
    char input[10];
    fgets(input, sizeof(input), stdin);
    if (input[0] == 'n' || input[0] == 'N') {
        printf("File %s formatted successfully.\n", argv[1]);
        exit(EXIT_SUCCESS);
    }

    // Preview the formatted code
    FILE *preview_file = fopen(argv[1], "w+");
    fwrite(file_buffer, 1, file_size, preview_file);
    rewind(preview_file);

    char preview_line[1024];
    int line_number = 1;
    printf("\nPreview of the formatted code:\n");

    // Generate a "diff" of the old and new code to show the differences
    char* diff_buffer = malloc(file_size * 3 + 1);
    diff_buffer[0] = '\0';
    int last_printed_line_number = 0;
    while (fgets(preview_line, sizeof(preview_line), preview_file) != NULL) {
        if (strcmp(preview_line, "\n") == 0) {
            strcat(diff_buffer, "\n"); // Add a newline to the diff buffer
            last_printed_line_number = line_number;
            line_number++;
            continue;
        }
        char* diff_line = malloc(strlen(preview_line) * 3 + 1);
        snprintf(diff_line, strlen(preview_line) * 3 + 1, "\033[38;2;0;255;0m+%s\033[0m\033[31m-%s\033[0m", preview_line, file_buffer);
        strcat(diff_buffer, diff_line); // Add the line to the diff buffer
        free(diff_line);
        last_printed_line_number = line_number;
        line_number++;
    }
    fclose(preview_file);

    // Print the diff to the console
    printf("%s", diff_buffer);

    // Ask the user if they want to apply the changes
    printf("\nDo you want to apply the changes? (Y/n): ");
    fgets(input, sizeof(input), stdin);
    if (input[0] == 'n' || input[0] == 'N') {
        printf("File %s not modified.\n", argv[1]);
        exit(EXIT_SUCCESS);
    }

    // Apply the changes
    file = fopen(argv[1], "w+");
    fwrite(file_buffer, 1, file_size, file);
    fclose(file);

    printf("File %s formatted successfully.\n", argv[1]);

    return 0;
}

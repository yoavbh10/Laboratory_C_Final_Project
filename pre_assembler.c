#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "macro_table.h"
#include "pre_assembler.h"
#include "error_list.h"

#define MAX_LINE_LENGTH 100

static void trim_whitespace(char* line) {
    char* end;
    while (isspace((unsigned char)*line)) line++;
    end = line + strlen(line) - 1;
    while (end > line && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

static int parse_define(const char* line, char* name, char* value) {
    if (sscanf(line, ".define %30s = %80s", name, value) == 2)
        return 1;
    return 0;
}

static void replace_macros(char* line, const Macro* macro_table) {
    char result[MAX_LINE_LENGTH] = "";
    char temp_line[MAX_LINE_LENGTH];
    char *token;
    const char *delim = " \t,";

    strcpy(temp_line, line);
    token = strtok(temp_line, delim);

    while (token != NULL) {
        const char* val = find_macro(macro_table, token);
        strcat(result, val ? val : token);
        strcat(result, " ");
        token = strtok(NULL, delim);
    }
    strcpy(line, result);
}

int pre_assembler(const char* filename, ErrorList *errors) {
    char line[MAX_LINE_LENGTH];
    char macro_name[MAX_MACRO_NAME_LEN];
    char macro_value[MAX_MACRO_VALUE_LEN];
    char input_filename[FILENAME_MAX];
    char output_filename[FILENAME_MAX];
    FILE *infile, *outfile;
    Macro* macro_table = NULL;

    sprintf(input_filename, "%s", filename);
    sprintf(output_filename, "%s.am", filename);

    infile = fopen(input_filename, "r");
    if (!infile) {
        add_error(errors, 0, "Cannot open source file");
        return 0;
    }

    outfile = fopen(output_filename, "w");
    if (!outfile) {
        fclose(infile);
        add_error(errors, 0, "Cannot create output file");
        return 0;
    }

    while (fgets(line, MAX_LINE_LENGTH, infile)) {
        trim_whitespace(line);
        if (line[0] == ';' || line[0] == '\0') {
            fprintf(outfile, "%s\n", line);
        } else if (strncmp(line, ".define", 7) == 0) {
            if (parse_define(line, macro_name, macro_value)) {
                if (add_macro(&macro_table, macro_name, macro_value) != 0)
                    add_error(errors, 0, "Duplicate macro ignored");
            } else {
                add_error(errors, 0, "Malformed .define line");
            }
        } else {
            replace_macros(line, macro_table);
            fprintf(outfile, "%s\n", line);
        }
    }

    fclose(infile);
    fclose(outfile);
    free_macro_table(macro_table);
    return 1;
}


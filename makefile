assembler: main.c pre_assembler.c macro_table.c \
           pre_assembler.h macro_table.h
	gcc -ansi -pedantic -Wall -Wextra \
	    main.c pre_assembler.c macro_table.c \
	    -o assembler

test_pre: test_pre.c pre_assembler.c macro_table.c error_list.c
	gcc -ansi -pedantic -Wall -Wextra \
	    test_pre.c pre_assembler.c macro_table.c error_list.c \
	    -o test_pre

test_symbol_table: test_symbol_table.c symbol_table.c \
                   symbol_table.h
	gcc -ansi -pedantic -Wall -Wextra \
	    test_symbol_table.c symbol_table.c \
	    -o test_symbol_table

test_addressing_modes: test_addressing_modes.c addressing_modes.c \
                       addressing_modes.h
	gcc -ansi -pedantic -Wall -Wextra \
	    test_addressing_modes.c addressing_modes.c \
	    -o test_addressing_modes

test_first_pass: test_first_pass.c first_pass.c \
                 symbol_table.c memory_image.c error_list.c instruction_set.c \
                 first_pass.h symbol_table.h memory_image.h error_list.h instruction_set.h
	gcc -ansi -pedantic -Wall -Wextra \
	    test_first_pass.c first_pass.c symbol_table.c memory_image.c \
	    error_list.c instruction_set.c \
	    -o test_first_pass

test_first_pass_data: test_first_pass_data.c first_pass.c symbol_table.c memory_image.c \
                      error_list.c instruction_set.c
	gcc -ansi -pedantic -Wall -Wextra \
	    test_first_pass_data.c first_pass.c symbol_table.c memory_image.c \
	    error_list.c instruction_set.c \
	    -o test_first_pass_data

test_first_pass_entry: test_first_pass_entry.c first_pass.c symbol_table.c memory_image.c \
                       error_list.c instruction_set.c \
                       first_pass.h symbol_table.h memory_image.h error_list.h instruction_set.h
	gcc -ansi -pedantic -Wall -Wextra \
	    test_first_pass_entry.c first_pass.c symbol_table.c memory_image.c \
	    error_list.c instruction_set.c \
	    -o test_first_pass_entry

test_second_pass: test_second_pass.c second_pass.c instruction_encoder.c output_files.c \
                  symbol_table.c memory_image.c error_list.c instruction_set.c addressing_modes.c
	gcc -ansi -pedantic -Wall -Wextra \
        test_second_pass.c second_pass.c instruction_encoder.c output_files.c \
        symbol_table.c memory_image.c error_list.c instruction_set.c addressing_modes.c \
        -o test_second_pass

clean:
	rm -f assembler test_pre test_symbol_table test_addressing_modes \
	      test_first_pass test_first_pass_data test_first_pass_entry \
	      test_second_pass *.o *.am


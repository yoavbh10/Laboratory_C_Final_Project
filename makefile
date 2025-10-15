test_second_pass: test_second_pass.c first_pass.c second_pass.c instruction_encoder.c output_files.c \
                  symbol_table.c memory_image.c error_list.c instruction_set.c addressing_modes.c \
                  first_pass.h second_pass.h instruction_encoder.h output_files.h \
                  symbol_table.h memory_image.h error_list.h instruction_set.h addressing_modes.h
	gcc -ansi -pedantic -Wall -Wextra \
        test_second_pass.c first_pass.c second_pass.c instruction_encoder.c output_files.c \
        symbol_table.c memory_image.c error_list.c instruction_set.c addressing_modes.c \
        -o test_second_pass

clean:
	rm -f test_second_pass *.o *.am


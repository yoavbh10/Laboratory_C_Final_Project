assembler: main.c pre_assembler.c first_pass.c second_pass.c instruction_encoder.c output_files.c \
           symbol_table.c memory_image.c error_list.c instruction_set.c addressing_modes.c
	gcc -ansi -pedantic -Wall -Wextra \
	    main.c pre_assembler.c first_pass.c second_pass.c instruction_encoder.c output_files.c \
	    symbol_table.c memory_image.c error_list.c instruction_set.c addressing_modes.c \
	    -o assembler

clean:
	rm -f assembler *.o *.ob *.ent *.ext


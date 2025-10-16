; invalid_2.as – duplicate labels, extern/entry conflict
LOOP:   inc r1
LOOP:   dec r2        ; ERROR: label LOOP already defined

.entry EXT1
.extern EXT1          ; ERROR: cannot be both entry and extern

        stop


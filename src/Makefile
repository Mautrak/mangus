CC      = gcc
PROF    = -g
NOCRYPT =
C_FLAGS = -Wall $(PROF) $(NOCRYPT) -Wno-misleading-indentation -Wno-format-overflow
L_FLAGS =  $(PROF)

O_FILES = act_comm.o act_hera.o act_info.o act_move.o act_obj.o act_wiz.o ban.o \
          cJSON.o comm.o const.o data.c db.o db2.o dilek.o kirliisler.o effects.o turkish_morphology.o \
          turkish_suffix_helper.o exlist.o fight.o flags.o handler.o handler_room.o healer.o interp.o lookup.o mafya.o \
          magic.o magic2.o martial_art.o mob_creator.o mob_prog.o obj_creator.o obj_prog.o \
          quest.o recycle.o save.o  skills.o special.o tables.o update.o

mangus: $(O_FILES)
	rm -f mangus
	$(CC) $(L_FLAGS) -o mangus $(O_FILES)

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	@rm -f *.o mangus

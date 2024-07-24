piko: dynarr.o lzstack.o lzhtable.o lzarea.o lzallocator.o vm_memory.o vm.o dumpper.o error_report.o memory.o scanner.o parser.o compiler.o
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-I ./include \
	-L ./bin \
	-o ./bin/piko \
	./src/piko.c \
	./bin/dynarr.o ./bin/lzstack.o ./bin/lzhtable.o ./bin/lzarea.o ./bin/lzallocator.o \
	./bin/vm_memory.o ./bin/vm.o ./bin/dumpper.o ./bin/error_report.o \
	./bin/memory.o ./bin/scanner.o ./bin/parser.o ./bin/compiler.o
				
compiler.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-I ./include \
	-I ./include/compiler \
	-c -o ./bin/compiler.o \
	./src/compiler/compiler.c

parser.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-I ./include \
	-I ./include/compiler \
	-c -o ./bin/parser.o \
	./src/compiler/parser.c
	
scanner.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-I ./include \
	-I ./include/compiler \
	-c -o ./bin/scanner.o \
	./src/compiler/scanner.c

memory.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-c -o ./bin/memory.o \
	-I ./include \
	-I ./include/compiler \
	./src/compiler/memory.c

error_report.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-I ./include \
	-I ./include/compiler \
	-c -o ./bin/error_report.o \
	./src/compiler/error_report.c
	
dumpper.o: vm.o
	gcc \
	-std=c99 \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-I ./include \
	-I ./include/vm \
	-c -o ./bin/dumpper.o \
	./src/vm/dumpper.c

vm.o: vm_memory.o
	gcc \
	-std=c99 \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-I ./include \
	-I ./include/vm \
	-c -o ./bin/vm.o \
	./src/vm/vm.c

vm_memory.o:
	gcc \
	-std=c99 \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-parameter \
	-I ./include \
	-I ./include/vm \
	-c -o ./bin/vm_memory.o \
	./src/vm/vm_memory.c
	
lzallocator.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-I ./include/essentials \
	-c -o ./bin/lzallocator.o \
	./src/essentials/lzallocator.c

lzarea.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-function \
	-I ./include/essentials \
	-c -o ./bin/lzarea.o \
	./src/essentials/lzarea.c


lzhtable.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-unused-function \
	-I ./include/essentials \
	-c -o ./bin/lzhtable.o \
	./src/essentials/lzhtable.c

	
lzstack.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-I ./include/essentials \
	-c -o ./bin/lzstack.o \
	./src/essentials/lzstack.c
	
dynarr.o:
	gcc \
	-Wall \
	-Wextra \
	-Werror \
	-I ./include/essentials \
	-c -o ./bin/dynarr.o \
	./src/essentials/dynarr.c


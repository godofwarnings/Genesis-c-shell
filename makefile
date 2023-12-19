CC := gcc
# CFLAGS := -g

shell:
	# gcc src/*.c -fsanitize=address -fsanitize=undefined -o  genesis
	gcc src/*.c -o  genesis


all: classifier 

classifier: dectree.c classifier.c
	gcc -g -Wall -std=gnu99 -o classifier dectree.c classifier.c -lm

.PHONY: clean all

clean:	
	rm classifier

	gdb --args classifier /h/u17/c9/00/zhaoti76/zhaoti76/a2/datasets/training_data.bin /h/u17/c9/00/zhaoti76/zhaoti76/a2/datasets/testing_data.bin

./classifier training_data.bin testing_data.bin
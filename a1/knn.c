#include <stdio.h>
#include <math.h>    // Need this for sqrt()
#include <stdlib.h>
#include <string.h>

#include "knn.h"

/* Print the image to standard output in the pgmformat.  
 * (Use diff -w to compare the printed output to the original image)
 */
void print_image(unsigned char *image) {
    printf("P2\n %d %d\n 255\n", WIDTH, HEIGHT);
    for (int i = 0; i < NUM_PIXELS; i++) {
        printf("%d ", image[i]);
        if ((i + 1) % WIDTH == 0) {
            printf("\n");
        }
    }
}

/* Return the label from an image filename.
 * The image filenames all have the same format:
 *    <image number>-<label>.pgm
 * The label in the file name is the digit that the image represents
 */
unsigned char get_label(char *filename) {
    // Find the dash in the filename
    char *dash_char = strstr(filename, "-");
    // Convert the number after the dash into a integer
    return (char) atoi(dash_char + 1);
}

/* ******************************************************************
 * Complete the remaining functions below
 * ******************************************************************/


/* Read a pgm image from filename, storing its pixels
 * in the array img.
 * It is recommended to use fscanf to read the values from the file. First, 
 * consume the header information.  You should declare two temporary variables
 * to hold the width and height fields. This allows you to use the format string
 * "P2 %d %d 255 "
 * 
 * To read in each pixel value, use the format string "%hhu " which will store
 * the number it reads in an an unsigned character.
 * (Note that the img array is a 1D array of length WIDTH*HEIGHT.)
 */
void load_image(char *filename, unsigned char *img) {
    // Open corresponding image file, read in header (which we will discard)
    FILE *f2 = fopen(filename, "r");
    if (f2 == NULL) {
        perror("fopen");
        exit(1);
    }
    int height;
    int width;
	fscanf(f2, "P2 %d %d 255 ", &width, &height);
	for(int i = 0;i<height*width;i++){
	    fscanf(f2, "%hhu ", &img[i]);
	}
    fclose(f2);
}


/**
 * Load a full dataset into a 2D array called dataset.
 *
 * The image files to load are given in filename where
 * each image file name is on a separate line.
 * 
 * For each image i:
 *  - read the pixels into row i (using load_image)
 *  - set the image label in labels[i] (using get_label)
 * 
 * Return number of images read.
 */
int load_dataset(char *filename, 
                 unsigned char dataset[MAX_SIZE][NUM_PIXELS],
                 unsigned char *labels) {
    // We expect the file to hold up to MAX_SIZE number of file names
    FILE *f1 = fopen(filename, "r");
    if (f1 == NULL) {
        perror("fopen");
        exit(1);
    }
    char name[MAX_NAME];
    int i = 0;
    while(fgets(name, MAX_NAME,f1) != NULL){
        load_image(name, dataset[i]);
        labels[i] = get_label(name);
        i++;
    }
    fclose(f1);
    return i;
}

/** 
 * Return the euclidean distance between the image pixels in the image
 * a and b.  (See handout for the euclidean distance function)
 */
double distance(unsigned char *a, unsigned char *b) {
    double d = 0;
    for(int i = 0; i<WIDTH*HEIGHT; i++){
        d += (a[i]-b[i])*(a[i]-b[i]);
    }
    return sqrt(d);
}

/**
 * Return the most frequent label of the K most similar images to "input"
 * in the dataset
 * Parameters:
 *  - input - an array of NUM_PIXELS unsigned chars containing the image to test
 *  - K - an int that determines how many training images are in the 
 *        K-most-similar set
 *  - dataset - a 2D array containing the set of training images.
 *  - labels - the array of labels that correspond to the training dataset
 *  - training_size - the number of images that are actually stored in dataset
 * 
 * Steps
 *   (1) Find the K images in dataset that have the smallest distance to input
 *         When evaluating an image to decide whether it belongs in the set of 
 *         K closest images, it will only replace an image in the set if its
 *         distance to the test image is strictly less than all of the images in 
 *         the current K closest images.
 *   (2) Count the frequencies of the labels in the K images
 *   (3) Return the most frequent label of these K images
 *         In the case of a tie, return the smallest value digit.
 */ 

int knn_predict(unsigned char *input, int K,
                unsigned char dataset[MAX_SIZE][NUM_PIXELS],
                unsigned char *labels,
                int training_size) {
    struct Tuple {
        int label;
        double dis;
    };
    struct Tuple k_set[K];
    double smallest = distance(input, dataset[0]);
    k_set[0].label = labels[0];
    k_set[0].dis = distance(input, dataset[0]);
    for (int i = 1; i < K; i++) {
        k_set[i].label = labels[i];
        k_set[i].dis = distance(input, dataset[i]);
        if (distance(input, dataset[i]) < smallest) {
            smallest = distance(input, dataset[i]);
        }
    }
    for (int j = K; j < training_size; j++) {
        double new_dis = distance(input, dataset[j]);
        if (new_dis < smallest) {
            smallest = new_dis;
            int largest_index = 0;
            for (int k = 0; k < K; k++) {
                if (k_set[k].dis > k_set[largest_index].dis) {
                    largest_index = k;
                }
            }
            k_set[largest_index].dis = new_dis;
            k_set[largest_index].label = labels[j];
        }
    }
    int result[10];
    for (int i = 0; i < K; i++) {
        if (k_set[i].label == 0) {
            result[0] += 1;
        } else if (k_set[i].label == 1) {
            result[1] += 1;
        } else if (k_set[i].label == 2) {
            result[2] += 1;
        } else if (k_set[i].label == 3) {
            result[3] += 1;
        } else if (k_set[i].label == 4) {
            result[4] += 1;
        } else if (k_set[i].label == 5) {
            result[5] += 1;
        } else if (k_set[i].label == 6) {
            result[6] += 1;
        } else if (k_set[i].label == 7) {
            result[7] += 1;
        } else if (k_set[i].label == 8) {
            result[8] += 1;
        } else if (k_set[i].label == 9) {
            result[9] += 1;
        }
    }
    int small = result[0];
    int index = 0;
    for(int i = 1; i <10; i++){
        if(result[i] < small){
            small = result[i];
            index = i;
        }if(result[i] == small){
            if(index > i){
                index = i;
            }
        }
    }
    return index;
}

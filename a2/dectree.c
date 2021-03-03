/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC209H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Mustafa Quraish, Bianca Schroeder, Karen Reid
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2021 Karen Reid
 */

#include "dectree.h"

/**
 * Load the binary file, filename into a Dataset and return a pointer to 
 * the Dataset. The binary file format is as follows:
 *
 *     -   4 bytes : `N`: Number of images / labels in the file
 *     -   1 byte  : Image 1 label
 *     - NUM_PIXELS bytes : Image 1 data (WIDTHxWIDTH)
 *          ...
 *     -   1 byte  : Image N label
 *     - NUM_PIXELS bytes : Image N data (WIDTHxWIDTH)
 *
 * You can set the `sx` and `sy` values for all the images to WIDTH. 
 * Use the NUM_PIXELS and WIDTH constants defined in dectree.h
 */
Dataset *load_dataset(const char *filename) {
    FILE *file_loaded;
    file_loaded = fopen(filename, "rb");
    if (file_loaded == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    Dataset *dataset = malloc(sizeof(Dataset));
    int num_i;
    fread(&num_i, sizeof(int), 1, file_loaded);
    dataset->num_items = num_i;
    unsigned char *label_arr = malloc(sizeof(unsigned char)*num_i);
    Image *image_arr = malloc(sizeof(Image)*num_i);
    for(int i = 0; i<num_i; i++){
        unsigned char temp;
        fread(&temp, sizeof(unsigned char), 1, file_loaded);
        label_arr[i] = temp;
        Image *image = malloc(sizeof(image));
        image->sx = WIDTH;
        image->sy = WIDTH;
        unsigned char *data_arr = malloc(sizeof(unsigned char)*NUM_PIXELS);
        for(int j=0;j<NUM_PIXELS; j++){
            fread(&data_arr[j], sizeof(unsigned char), 1, file_loaded);
        }
        image->data = data_arr;
        image_arr[i] = *image;
    }
    dataset->labels = label_arr;
    dataset->images = image_arr;
    return dataset;
}

/**
 * Compute and return the Gini impurity of M images at a given pixel
 * The M images to analyze are identified by the indices array. The M
 * elements of the indices array are indices into data.
 * This is the objective function that you will use to identify the best 
 * pixel on which to split the dataset when building the decision tree.
 *
 * Note that the gini_impurity implemented here can evaluate to NAN 
 * (Not A Number) and will return that value. Your implementation of the 
 * decision trees should ensure that a pixel whose gini_impurity evaluates 
 * to NAN is not used to split the data.  (see find_best_split)
 * 
 * DO NOT CHANGE THIS FUNCTION; It is already implemented for you.
 */
double gini_impurity(Dataset *data, int M, int *indices, int pixel) {
    int a_freq[10] = {0}, a_count = 0;
    int b_freq[10] = {0}, b_count = 0;

    for (int i = 0; i < M; i++) {
        int img_idx = indices[i];

        // The pixels are always either 0 or 255, but using < 128 for generality.
        if (data->images[img_idx].data[pixel] < 128) {
            a_freq[data->labels[img_idx]]++;
            a_count++;
        } else {
            b_freq[data->labels[img_idx]]++;
            b_count++;
        }
    }

    double a_gini = 0, b_gini = 0;
    for (int i = 0; i < 10; i++) {
        double a_i = ((double)a_freq[i]) / ((double)a_count);
        double b_i = ((double)b_freq[i]) / ((double)b_count);
        a_gini += a_i * (1 - a_i);
        b_gini += b_i * (1 - b_i);
    }

    // Weighted average of gini impurity of children
    return (a_gini * a_count + b_gini * b_count) / M;
}

/**
 * Given a subset of M images and the array of their corresponding indices, 
 * find and use the last two parameters (label and freq) to store the most
 * frequent label in the set and its frequency.
 *
 * - The most frequent label (between 0 and 9) will be stored in `*label`
 * - The frequency of this label within the subset will be stored in `*freq`
 * 
 * If multiple labels have the same maximal frequency, return the smallest one.
 */
void get_most_frequent(Dataset *data, int M, int *indices, int *label, int *freq) {
    int arr[M];
    for(int i = 0; i<M; i++){
        arr[i] = data->labels[indices[i]];
    }
    int count = 0;
    int result_label = -1;
    for (int i = 0; i<M;i++){
        int temp_freq = 1;
        for (int j = i+1;j<M;j++){
            if (arr[j] == arr[i]) {
                temp_freq = temp_freq + 1;
            }
        if (count < temp_freq){
            count = temp_freq;
            result_label = arr[i];
        }
        else if (count == temp_freq){
            if(arr[count]>arr[temp_freq]){
                result_label = arr[temp_freq];
            }
            }
        }
    }
    *label = result_label;
    *freq = count;
}

/**
 * Given a subset of M images as defined by their indices, find and return
 * the best pixel to split the data. The best pixel is the one which
 * has the minimum Gini impurity as computed by `gini_impurity()` and 
 * is not NAN. (See handout for more information)
 * 
 * The return value will be a number between 0-783 (inclusive), representing
 *  the pixel the M images should be split based on.
 * 
 * If multiple pixels have the same minimal Gini impurity, return the smallest.
 */
int find_best_split(Dataset *data, int M, int *indices) {
    int pixel = -1;
    double gini = INFINITY;
    for(int i = 0; i<M;i++){
        Image image = data->images[indices[i]];
        for(int j = 0;j<NUM_PIXELS;j++){
            if(gini > gini_impurity(data,M, indices, image.data[j])){
                gini = gini_impurity(data,M, indices, image.data[j]);
                pixel = j;
            }else if(gini == gini_impurity(data,M, indices, image.data[j])){
                if(pixel > image.data[j]){
                    pixel = j;
                }
            }
        }
    }
    return pixel;
}

/**
 * Create the Decision tree. In each recursive call, we consider the subset of the
 * dataset that correspond to the new node. To represent the subset, we pass 
 * an array of indices of these images in the subset of the dataset, along with 
 * its length M. Be careful to allocate this indices array for any recursive 
 * calls made, and free it when you no longer need the array. In this function,
 * you need to:
 *
 *    - Compute ratio of most frequent image in indices, do not split if the
 *      ration is greater than THRESHOLD_RATIO
 *    - Find the best pixel to split on using `find_best_split`
 *    - Split the data based on whether pixel is less than 128, allocate 
 *      arrays of indices of training images and populate them with the 
 *      subset of indices from M that correspond to which side of the split
 *      they are on
 *    - Allocate a new node, set the correct values and return
 *       - If it is a leaf node set `classification`, and both children = NULL.
 *       - Otherwise, set `pixel` and `left`/`right` nodes 
 *         (using build_subtree recursively). 
 */
DTNode *build_subtree(Dataset *data, int M, int *indices) {
    DTNode *node = malloc(sizeof(DTNode));
    int frequent = -1;
    int label = -1;
    get_most_frequent(data, M, indices, &label, &frequent);
    if(frequent / M > THRESHOLD_RATIO){
        node->classification = label;
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    int pixel = find_best_split(data, M, indices);
    node->pixel = pixel;
    int less_count = 0;
    int more_count = 0;
    node->classification = -1;
    for(int i = 0;i < M;i++){
        if(data->images[indices[i]].data[pixel] <128){
            less_count ++;
        }else{
            more_count ++;
        }
    }
    int *less_arr = malloc(sizeof(int)*less_count);
    int *more_arr = malloc(sizeof(int)* more_count);
    int less_index = 0;
    int more_index = 0;
    for(int i = 0;i < M;i++){
        if(data->images[indices[i]].data[pixel] <128){
            less_arr[less_index] = i;
            less_index++;
        }else{
            more_arr[more_index] = i;
            more_index++;
        }
    }
    node->left = build_subtree(data, less_count, less_arr);
    node->right = build_subtree(data, more_count, more_arr);
    free(less_arr);
    free(more_arr);
    return node;
}

/**
 * This is the function exposed to the user. All you should do here is set
 * up the `indices` array correctly for the entire dataset and call 
 * `build_subtree()` with the correct parameters.
 */
DTNode *build_dec_tree(Dataset *data) {
    // TODO: Set up `indices` array, call `build_subtree` and return the tree.
    // HINT: Make sure you free any data that is not needed anymore
    int *arr = malloc(sizeof(int)*data->num_items);
    for(int i = 0; i < data->num_items;i++){
        arr[i] = i;
    }
    DTNode *node = build_subtree(data, data->num_items, arr);
    free(arr);
    return node;
}

/**
 * Given a decision tree and an image to classify, return the predicted label.
 */
int dec_tree_classify(DTNode *root, Image *img) {
    // TODO: Return the correct label
    if(root->classification != -1){
        return root->classification;
    }else{
        if(img->data[root->pixel] < 128){
            return dec_tree_classify(root->left, img);
        }else{
            return dec_tree_classify(root->right, img);
        }
    }
}

/**
 * This function frees the Decision tree.
 */
void free_dec_tree(DTNode *node) {
    // TODO: Free the decision tree
    if(node->classification != -1){
        free(node);
    }else{
        free_dec_tree(node->left);
        free_dec_tree(node->right);
    }
}

/**
 * Free all the allocated memory for the dataset
 */
void free_dataset(Dataset *data) {
    // TODO: Free dataset (Same as A1)
    for(int i = 0; i < data->num_items; i++){
        free(data->images[i].data);
        free(&data->images[i]);
    }
    free(data->images);
    free(data->labels);
    free(data);
}

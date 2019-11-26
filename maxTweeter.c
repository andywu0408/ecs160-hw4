#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILE_LINES_MAX 20000
#define LINE_CHARS_MAX 1024
#define TOP_NAMES 10

void error();
void fillTopTenNamesAndFreq(char* names[], int num_names, char* top_ten_names[], int top_ten_freqs[]);
int getNameColumn (char* first_line);
int getTweetersName(FILE* file_ptr, int num_col, char* tweetersName[]);
void printTopTen(char* top_ten_names[], int top_ten_freqs[]);

// helper functions
int cmpfunc (const void * a, const void * b);
int fillNamesAndFreq(char* names[], int num_names, char* final_names[], int final_freqs[]);
void quote_processor(char* tweeter_name);

// gdb note: p final_freqs[x]@10

int main (int argc, char* argv[]) {
    char* tweetersName[FILE_LINES_MAX]; // for storing name of all tweeters
    
	// Check for valid program input
	// Must have 1 command line argument (filename)
	if (argc != 2) {
		error();
	}

	// Store and open file
	const char* file_name = argv[1];
	FILE* file_ptr = fopen(file_name, "r");

	// Check if file exists
	if (file_ptr == NULL) {
		error();
	}

	// Store first line 
	char first_line[LINE_CHARS_MAX + 2]; // "+2" for potential overflow char + null term char
    if (fgets(first_line, LINE_CHARS_MAX + 2, file_ptr) == NULL) { // fgets includes null-terminated char (strlen does not)
        error(); 
    } else if (strlen(first_line) > LINE_CHARS_MAX) { 
        // if we read in more than 1024 chars
        error();
    }

	// Get column position of "name" 
	int num_col = getNameColumn(first_line);

    // Fill tweetersName array with all names, may have repeated names,
    int num_names = getTweetersName(file_ptr, num_col, tweetersName); // total number of names
    
    // Store top 10
    char* top_ten_names[10]; // top 10 tweeters with most tweets
    int top_ten_freqs[10]; // frequencies of tweets made by top 10 tweeters
    
    // Get names and frequencies of 10 different tweeters who made most tweets
    fillTopTenNamesAndFreq(tweetersName, num_names, top_ten_names, top_ten_freqs);
    
    // Print the names and frequencies of top 10 tweeters
    printTopTen(top_ten_names, top_ten_freqs);
    
    fclose(file_ptr);
}

void printTopTen(char* top_ten_names[], int top_ten_freqs[]){
    for(int i = 0; i < TOP_NAMES; i++){
        printf("%s: %d\n", top_ten_names[i], top_ten_freqs[i]);
    }
}

// Get names and frequencies of 10 different tweeters who made most tweets
void fillTopTenNamesAndFreq(char* names[], int num_names, char* top_ten_names[], int top_ten_freqs[]){
    char* final_names[FILE_LINES_MAX]; // all names of tweeters, with no repeated element
    int final_freqs[FILE_LINES_MAX]; // frequencies of tweets made by all individual tweeters
    int num_final_elements = fillNamesAndFreq(names, num_names, final_names, final_freqs);
    
    // Sort frequencies in descending order
    qsort(final_freqs, num_final_elements, sizeof(int), cmpfunc);
    
    int counter = 0;
    
    for(int i = 0; i < TOP_NAMES; i++) { // get top ten highest
        top_ten_names[counter] = final_names[i];
        top_ten_freqs[counter] = final_freqs[i];
        counter++;
    };
}
// Comparable function for qsort
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)b - *(int*)a );
}

// helper function for fillTopTenNamesAndFreq function
int fillNamesAndFreq(char* names[], int num_names, char* final_names[], int final_freqs[]){
    int freq; // counter
    char* current;
    int num_final_elements = 0; // unique names
    int tweeter_name_length; 

    for(int i = 0; i < num_names; i++){
        if(strcmp(names[i], "") == 0){
            // skip because the element is repeated (we changed all repeated elements to empty string)
            continue; 
        } else {
            freq = 1; // initialize frequency to 1 
            current = names[i]; // store current name

            // check quoting of name
            quote_processor(names[i]);

            for (int j = i + 1; j < num_names; j++) {

                // check quoting of compared name
                quote_processor(names[j]);

                if (strcmp(names[j], "") == 0){
                    // skip because the element is repeated (we changed all repeated elements to empty string)
                    continue;
                }
                if (strcmp(current, names[j]) == 0) { // find another occurence of the element
                    freq++;
                    // change it to empty string, so we can handle this repeated value when we encounter it later on
                    strcpy(names[j], "");
                }
            }
            final_names[num_final_elements] = current;
            final_freqs[num_final_elements] = freq;
            num_final_elements++;
        }
    }
    
    if (num_final_elements <= 0) {
        error();
    }
    
    return num_final_elements;
}

/* quote validation helper function
    1. validate that we have an even amount of quotes
    2. strip off outer layer of quotes directly from names pointer
*/
void quote_processor(char* tweeter_name) {
    // store length of name
    int tweeter_name_length = strlen(tweeter_name);

    // check how many quotes in a name
    int quote_count = 0;
    for (int i = 0; i < tweeter_name_length; i++) {
        if (tweeter_name[i] == '\"') {
            quote_count++;
        }
    }

    // if uneven number of quotes, then error 
    if (quote_count % 2 == 1) {
        error();
    }

    // if the name is quoted properly, then remove them
    if (tweeter_name[0] == '\"' && 
        tweeter_name[tweeter_name_length - 1] == '\"') {
        // copy unquoted portion over
        strncpy(tweeter_name, tweeter_name + 1, tweeter_name_length - 2);
        // cut off oustanding quote with a null terminator
        tweeter_name[tweeter_name_length - 2] = '\0';
    }
}


/*
 Store names of all tweeters into tweetersName[] and return total number of names
 NOTE: The returned array may contain repeated names, or "empty" if no value given
*/
int getTweetersName(FILE* file_ptr, int num_col, char* tweetersName[]) {
    char line[LINE_CHARS_MAX + 2]; // one line in file
    char* temp; // store pointer to line
    int col_counter;
    int counter = 0;
    
    // get one line from file if not end of file yet
    while(fgets(line, LINE_CHARS_MAX + 2, file_ptr ) != NULL ) {
        if (strlen(line) > LINE_CHARS_MAX) { 
            // if we read in more than 1024 chars
            error();
        }

        temp = strdup(line);
        col_counter = 1;
        char* token = strsep(&temp, ",");
        // get name of tweeter from this line of file
        while(token != NULL){
            if(col_counter == num_col + 1){ // if at the name column
                if(strcmp(token, "") == 0){ // name is "empty" if receive empty value
                    token = "empty";
                }
                tweetersName[counter] = token;
                break;
            }
            token = strsep(&temp, ",");
            col_counter++;
        }
        counter++;
    }
    return counter;
}

// Get the column number for names
int getNameColumn (char* first_line) {
	
	int num_col = 0;
    int name_flag = 0;

	// Check if first line exists
	if (first_line == NULL) {
		error();
	}
	else {
		// Search first line for name column
		char* token = strtok(first_line, ",");

		// Looking for quoted "name" or unquoted name
        // In either Windows/Linux file format, since "name" could be the last column
		char name_string[10] = "name";
        char name_string_windows[10] = "name\r\n";
        char name_string_linux[10] = "name\n";
        char name_string_quoted[10] = "\"name\"";
        char name_string_quoted_windows[10] = "\"name\"\r\n";
        char name_string_quoted_linux[10] = "\"name\"\n";

        // Iterate through comma separated values
		for (int i = 0; token != NULL; i++) {
			// Check string equality to either variant of name
			if (strcmp(token, name_string) == 0 ||
                strcmp(token, name_string_windows) == 0 ||
                strcmp(token, name_string_linux) == 0 ||
                strcmp(token, name_string_quoted) == 0 ||
                strcmp(token, name_string_quoted_windows) == 0 ||
                strcmp(token, name_string_quoted_linux) == 0) {
				num_col = i;
                name_flag = 1;
			}
			token = strtok(NULL, ",");
		}
	}

    // if name was not found
    if (name_flag == 0) {
        error();
    } else { // otherwise return column position of name
	   return num_col;
    }
}

void error(){
    printf("Invalid Input Format");
    exit(1); // unsuccessful exit
}

/*********************************************************************************
  Written by Daniel Dukeshire, 10.12.2019 - 11.1.2019

 This program is designed to take multiple input files from the command line, and
 return the total number of words, unique words, interesting bigrams, unique int-
 eresting bigrams, interesting trigrams, and unique interesting trigrams of those
 files.

 After calculating these metrics, the system displays the top 50 words of those
 inputted documents, the top 20 interesting bigrams, and top 12 interesting trigrams.

 This function utilizes UTHash, a hash imlementation found online with given
 "ut.hash.h", which contains a library of functions and hash table maintainability.
 More can be found here: https://troydhanson.github.io/uthash/userguide.html#_string_keys

 On top of this, although the previously noted functionality does not utilize it,
 this contains a regex_match function, which can be called if necessary.
 *********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "uthash.h"

/*********************************************************************************
 my_struct: This is a struct declared for storage in the UT hash table.
            Each separate UT Hash handle represents a different hash table. I use a
            separate hash to represent each the total number of words, interesting
            bigrams, and interesting trigrams.
 *********************************************************************************/
struct my_struct {
    int id;
    char name[60];
    int count;
    UT_hash_handle ah;          // hash struct for all words
    UT_hash_handle hh;          // hash table for bigram
    UT_hash_handle bh;          // hash table for trigram
};

/*********************************************************************************
 count_sort:    This is a function utilized by UTHash to sort the hash table based
                on the highest # of words represented for each key in the set.
 *********************************************************************************/
int count_sort(struct my_struct *a, struct my_struct *b) {
    return (b->count - a->count);
}

/*********************************************************************************
 sort_by_name:  This is a function utilized by UTHash to sort the hash table based
                on the name of the struct stored in the hash
 *********************************************************************************/
int sort_by_name(struct my_struct *a, struct my_struct *b) {
    return strcmp(a->name, b->name);
}

// Global Variables --> Used to represent each of the hash tables, acting as the "head" of the table
struct my_struct *w, *tmp1, *words = NULL;      // Representation for the standard word hash
struct my_struct *b, *tmp2, *bigrams = NULL;    // Representation for the bigram hash
struct my_struct *t, *tmp3, *trigrams = NULL;   // Representation for the trigram hash
int id=0, Bid=0, Tid=0;                         // The current id location for each hash table

// Stop words to be excluded from the bigrams/trigrams
char skip[48][60] = { "the", "of", "to", "and", "in", "said", "for", "that", "was", "on",
    "he", "is", "with", "at", "by", "it" , "from", "as", "be", "were",
    "an", "have", "his", "but", "has", "are", "not", "who", "they", "its",
    "had", "will", "would", "about", "been", "this", "their", "new", "or", "which",
    "we", "more", "after", "us", "percent", "up", "one", "people" };


/*********************************************************************************
 num_lines:  Called by main() when reading in each input file

            Loops through the input file to determine the number of
            lines in that particular file. Counts the number of '\n'
            This number is used for memory allocation for the one_line

            Parameters: a FILE pointer representing the file opened

            Returns: The number of lines
 *********************************************************************************/
int num_lines(FILE* inputFile)
{
    int numLines = 0;
    char chr = getc(inputFile);                     // Retrieves one charater at a time, loops through the
                                                    // entire file, counting the number of newline characters
    int addition = 1;
    while(chr != EOF)
    {
        if(chr == '\n') numLines +=1;
        char temp = chr;
        chr = getc(inputFile);
        if(temp == '\n' && chr == EOF) addition = 0;
    }
    rewind(inputFile);

    return numLines+addition;                       // Amounts to the number of lines in the text file
}

/*********************************************************************************
 one_line:   Called by main() when reading each input file

            Reads in the entire file into one single string. Returns that string
            with the new-line characters replaced with spaces.

            Parameters: A FILE pointer representing the file opened, and the size of
            the file, which is the number of newLine chracters in the text (i.e.
            the number of lines.

            Returns: Allocates a string to represent the entire file, returns
            that string.
 *********************************************************************************/
char* one_line(FILE* inputFile, int size)
{
    int charSize = 1024*size;
    char *readLine = calloc(charSize, sizeof(char));        // puts the entire file into one string! no \n
    char *currentLine = calloc(1024, sizeof(char));

    while(fgets(currentLine, 1024, inputFile) != NULL)      // Loops through the input file
    {
        if(currentLine[strlen(currentLine)-1] == '\n')      // If a new line character is found
            currentLine[strlen(currentLine)-1] = ' ';       // replace it with a space
        strcat(readLine, currentLine);
    }

    readLine[strlen(readLine)] = '\0';
    free(currentLine);

    return readLine;
}

/********************************************************************************
 one_or_more:   Called by regex_match() when parsing reaches a '+' or '*'

                Starts from the back of the input line (readLine) string, finding
                the character preceeding the '+' in regex. If found, the system
                then loops from the current offset position in the readLine, ensuring
                that are the values between the offset and ending character are valid.

                Parameters: Integer 'k' as the current parse location in the regex string
                'expression', and an integer 'offset' value for the location in line
                number 'o',

                Returns: The new offset location in the readLine string. If the '+'
                or '*' is invalid, the system returns a 0.
 ********************************************************************************/
int one_or_more(int k, int offset, int o, const char* expression, const char* readLine, int l)
{
    bool isEndingCharacter;                                             // used to store if the ending-character is found

    for(int i = strlen(readLine)-2; i>=(offset+o); i--)                 // loop from the back, searching for the ending character
    {
        isEndingCharacter = false;
        if(k == strlen(expression)-1){ isEndingCharacter = true; }      // if + is the last expression statement
        else if(k+2 < strlen(expression) && expression[k+1] == '\\')
        {
            if(expression[k+2] == 'd' && isdigit(readLine[i])!=0) isEndingCharacter = true;         // Similar checks to the main as
            else if(expression[k+2] == 'D' && isdigit(readLine[i])==0) isEndingCharacter = true;    // well as below, applying the
            else if(expression[k+2] == 'w' && isalpha(readLine[i])!=0) isEndingCharacter = true;    // regex syntax
            else if(expression[k+2] == 'W' && isalpha(readLine[i])==0) isEndingCharacter = true;
            else if(expression[k+2] == 's' && isspace(readLine[i])!=0) isEndingCharacter = true;
            else if(expression[k+2] == '\\' && readLine[i] == '\\') isEndingCharacter = true;
        }
        else if(expression[k+1] == '[')                     // Takes the brackets into consideration
        {
            bool found = false;
            bool is_not = false;
            int y = k+2;
            for( ; y<strlen(expression); y++)               // Loops until the end of the brackets
            {
                if(expression[y] == ']') break;
                if(expression[y] == '^') is_not = true;     // Finds the carrot "not"
                else if(expression[k]=='.'){                // Repeats previous code
                    found = true;                           // This is to account for this syntax WITHIN the brackets
                }
                else if(expression[y]=='\\' && expression[y+1] == 'd' && isdigit(readLine[i]) != 0)
                    found = true;
                else if(expression[y]=='\\' && expression[y+1] == 'D' && isdigit(readLine[i]) == 0)
                    found = true;
                else if(expression[y]=='\\' && expression[y+1] == 'w' && isalpha(readLine[i]) != 0)
                    found = true;
                else if(expression[y]=='\\' && expression[y+1] == 'W' && isalpha(readLine[i]) == 0)
                    found = true;
                else if(expression[y]=='\\' && expression[y+1] == 's' && isspace(readLine[i]) != 0)
                    found = true;
                else if(expression[y]=='\\' && expression[y+1] == '\\' && readLine[i] == '\\')
                    found = true;
                else if(expression[y]=='\\' ) y+=1;
                else if(readLine[i] == expression[y]) found = true;
            }
            if(found == true && is_not == false) isEndingCharacter = true;
            else if(found == false && is_not == true) isEndingCharacter = true;
        }
        else if(expression[k+1] == '.') isEndingCharacter = true;
        else if(expression[k+1] == readLine[i]) { isEndingCharacter = true; }

        if(isEndingCharacter == true)                                   // If the rightmost character is found....
        {
            int j = 0;
            for(j = (offset+o); j<i; j++)                               // we loop forwards from offset, until we get to i (end character)
            {
                if(k>=2-l && expression[k-2-l] == '\\')                 // checks to see if every value until i is of the right type :
                {                                                                           // BREAKS IF:
                    if(expression[k-1-l] == 'd' && isdigit(readLine[j])==0) break;          // Should be a digit
                    else if(expression[k-1-l] == 'D' && isdigit(readLine[j])!=0) break;     // Shouldnt be a digit
                    else if(expression[k-1-l] == 'w' && isalpha(readLine[j])==0) break;     // Should be a alpha
                    else if(expression[k-1-l] == 'W' && isalpha(readLine[j])!=0) break;     // Shouldnt be an alpha
                    else if(expression[k-1-l] == 's' && isspace(readLine[j])==0) break;     // Should be a space
                    else if(expression[k-1-l] == '\\' && readLine[j] != '\\') break;        // Should be a '\'
                }
                else if(expression[k-1-l] == ']')                       // Accounting for bracket representation
                {
                    bool found = false;
                    bool is_not = false;
                    int y = k-2;
                    for( ; y>0; y--) if(expression[y] == '[') break;    // Determines where to start the loop
                    y+=1;                                               // Moves the location nack to
                    for(; y<strlen(expression); y++)
                    {
                        if(expression[y] == ']') break;
                        if(expression[y] == '^') is_not = true;         // Finds the carrot "not"
                        else if(expression[k]=='.'){                    // Repeats previous code
                            found = true;                               // This is to account for this syntax WITHIN the brackets
                        }
                        else if(expression[y]=='\\' && expression[y+1] == 'd' && isdigit(readLine[j]) != 0)
                            found = true;
                        else if(expression[y]=='\\' && expression[y+1] == 'D' && isdigit(readLine[j]) == 0)
                            found = true;
                        else if(expression[y]=='\\' && expression[y+1] == 'w' && isalpha(readLine[j]) != 0)
                            found = true;
                        else if(expression[y]=='\\' && expression[y+1] == 'W' && isalpha(readLine[j]) == 0)
                            found = true;
                        else if(expression[y]=='\\' && expression[y+1] == 's' && isspace(readLine[j]) != 0)
                            found = true;
                        else if(expression[y]=='\\' && expression[y+1] == '\\' && readLine[j] == '\\')
                            found = true;
                        else if(expression[y]=='\\' ) y+=1;
                        else if(readLine[j] == expression[y]) found = true;
                    }
                    if(found == true && is_not == true) break;
                    else if(found == false && is_not == false) break;       // Breaks out of the j loop if the conditions
                }                                                           // are not met.
                else if(expression[k-1-l] == '.') continue;                 // any character, can continue
                else if(expression[k-1-l] != readLine[j]) break;            // values arent the same
            }
            if(j==i && l==0) return i;                                      // If j gets to i, all characters matched.
            else if(j==i && l>0) return i-1;
        }                                                                   // Now, the main offset it that value returned.
    }
    if(isEndingCharacter==false)                                            // This does not account for \w+[fdsafas]?
    {
        if(k+3<strlen(expression) && expression[k+1] == '\\' && (expression[k+3] == '?' || expression[k+3] ==  '*'))
        {
            return one_or_more(k+3, offset, o, expression, readLine, 3)+1;    // Calls the program recursively, now with a new ending
        }
        else if(k+2<strlen(expression) && (expression[k+2] == '?' || expression[k+2] == '*'))
        {
            return one_or_more(k+2, offset, o, expression, readLine, 2)+1;
        }
        else if(expression[k+1]=='[')                                       // Accounts for []
        {
            for(int m=k+1; m<strlen(expression); m++)
            {
                if(expression[m] == '?')
                    return one_or_more(m, offset, o, expression, readLine, k-m);
            }
        }
    }
    return 0;          // if never returns an i value, return 0. This means doesItMatch = false
}                      // in the main function.



/*********************************************************************************
 regex_match:   Called by Main() for #ifdef USE_SUBMITTY_MAIN

                Loops through the input file line by line to determine
                all lines matching the regex expression, which are added to the
                matches array if found.

                Parameters: The filename as a string to read, string regex
                as the regex value, and a NULL pointer to a matches array.

                Returns: An integer representing the number of lines added to the
                matches array. Initializes the array, adds each found line.
 ********************************************************************************/
int regex_match(const char* filename, const char* regex, char*** matches)
{
    FILE* inputFile = fopen(filename, "r");
    if(inputFile == NULL)                                       // Error Checking
    {
        fprintf(stderr, "ERROR: Invalid arguments\n");
        fprintf(stderr, "USAGE: a.out <input-file1> [ <input-file2> ... ]\n");
        exit(EXIT_FAILURE);                                     // Terminates the script
    }
    else if(regex == NULL) return 0;
    int size = num_lines(inputFile);

    char *readLine = calloc(1024, sizeof(char));                // Allocates space for each line in the file
    char **matches2 = calloc(size, sizeof(char *) );
    for(int m=0; m< size; m++) {
        matches2[m] = calloc(1024, sizeof(char));
    }

    int matched_lines = 0;                                      // The number of matches lines, and the current word
    int curWord = 0;                                            // in the matches2 array
    for(int i=0; i<size; i++)                                   // Loops through the entire input file
    {
        bool didPrint = false;                                  // A boolean to exit the loop if the line is printed
        fgets(readLine, 1024, inputFile);                       // Reads the line in
        for(int j=0; j<strlen(readLine); j++)                   // Loops across each line in the input file
        {
            if(didPrint == true) break;                         // Breaks the loop if the line has already been printed
            int offset = 0;                                     // allows the inner loop to continue without loosing j index
            for(int k=0; k<strlen(regex); k++)                  // Loops across the expression
            {
                if((offset+j)>strlen(readLine)) break;
                bool doesItMatch = false;                       // boolean which stores if the current character matches the regex
                // ACCOUNTING FOR REGEX SYNTAX
                if(regex[k] == '?') { offset -=1; doesItMatch = true; }
                else if(regex[k]=='.')                          // All characters, dont need to check readLine
                {
                    doesItMatch = true;
                }
                else if(regex[k]=='\\' && regex[k+1] == 'd' && isdigit(readLine[j+offset]) != 0)
                {                                               // Is it a Digit?
                    doesItMatch = true;
                    k +=1;                                      // increment k twice, to comprehend for the expression[k+1]
                }
                else if(regex[k]=='\\' && regex[k+1] == 'D' && isdigit(readLine[j+offset]) == 0)
                {                                               // Not a Digit?
                    doesItMatch = true;
                    k+=1;                                       // increment k twice, to comprehend for k+1
                }
                else if(regex[k]=='\\' && regex[k+1] == 'w' && isalpha(readLine[j+offset]) != 0)
                {                                               // Is a Letter?
                    doesItMatch = true;
                    k+=1;                                       // increment k twice, to comprehend for k+1
                }
                else if(regex[k]=='\\' && regex[k+1] == 'W' && isalpha(readLine[j+offset]) == 0)
                {                                               // Not a Letter?
                    doesItMatch = true;
                    k+=1;                                       // increment k twice, to comprehend for k+1
                }
                else if(regex[k]=='\\' && regex[k+1] == 's' && isspace(readLine[j+offset]) != 0)
                {                                               // Is whitespace?
                    doesItMatch = true;
                    k+=1;                                       // increment k twice, to comprehend for k+1
                }
                else if(regex[k]=='\\' && regex[k+1] == '\\' && readLine[j+offset] == '\\')
                {                                               // Is a literal Backslash?
                    doesItMatch = true;
                    k+=1;                                       // increment k twice, to comprehend for k+1
                }
                else if(regex[k] == '[')                        // ACCOUNTING FOR THE BRACKETS
                {
                    bool found = false;
                    bool is_not = false;
                    int y = k+1;
                    for( ; y<strlen(regex); y++)                // Loops until the end of the brackets
                    {
                        if(regex[y] == ']') break;
                        if(regex[y] == '^') { is_not = true;}   // Finds the carrot "not"

                        else if(regex[k]=='.'){                 // Repeats tje above code.
                            found = true;                       // This is to account for this syntax WITHIN the brackets
                        }
                        else if(regex[y]=='\\' && regex[y+1] == 'd' && isdigit(readLine[j+offset]) != 0){
                            found = true;
                            y+=1;
                        }
                        else if(regex[y]=='\\' && regex[y+1] == 'D' && isdigit(readLine[j+offset]) == 0){
                            found = true;
                            y+=1;
                        }
                        else if(regex[y]=='\\' && regex[y+1] == 'w' && isalpha(readLine[j+offset]) != 0){
                            found = true;
                            y+=1;
                        }
                        else if(regex[y]=='\\' && regex[y+1] == 'W' && isalpha(readLine[j+offset]) == 0){
                            found = true;
                            y+=1;
                        }
                        else if(regex[y]=='\\' && regex[y+1] == 's' && isspace(readLine[j+offset]) != 0){
                            found = true;
                            y+=1;
                        }
                        else if(regex[y]=='\\' && regex[y+1] == '\\' && readLine[j+offset] == '\\'){
                            found = true;
                            y+=1;
                        }
                        else if(regex[y]=='\\' ) y+=1;                          // Prevents the loop from missing \o, but then returning o
                        else if(readLine[j+offset] == regex[y]) found = true;   // on the next iteration.
                    }
                    if(found == true && is_not == false) doesItMatch = true;
                    else if(found == false && is_not == true) doesItMatch = true;
                    k=y;
                }
                else if(regex[k]=='+'|| regex[k]=='*')                          // Accounting for the looping MORE THAN ONE
                {
                    int temp = 0;                                               // Stores the temporary offset returned by function
                    temp = one_or_more(k, offset-1, j, regex, readLine, 0);     // Determines if there are more values!
                    if(temp != 0 )
                    {

                        offset = temp-j;                                        // As temp is o (j) and offset combined, must subtract j
                        if(k+1< strlen(regex) && regex[k+1] == '[') while(regex[k]!=']') k+=1;
                        else if(k+3 < strlen(regex) && regex[k+3] == ('*' | '?') && regex[k+1] == '\\') k+=3;
                        else if(k+2 < strlen(regex) && regex[k+2] == ('*' | '+' | '?')) k+=2;
                        else if(k+2<strlen(regex) && regex[k+1] == '\\') k+=2;
                        else if (k+1<strlen(regex) && regex[k+1] != '\0') k+=1; // If the + was the last character, only add one!

                        doesItMatch = true;
                    }
                }
                else if(regex[k] == readLine[j+offset]){ doesItMatch = true; }          // Standard comparison

                if ( doesItMatch == false )                             // If none of the regex values/standard comparison could be found ...
                {
                    if(regex[k+1] == '?'||(regex[k] == '\\' && regex[k+2] == '?') )     // Could be none (with the ? or *)
                    {
                        if(regex[k+1] == '?') k+=1;                     // double increments the expression iterator over the current '?'
                        else k +=2 ;
                        offset-=1;                                      // keeps the string iterator on the same line

                    }
                    else if((k+1 < strlen(regex) && regex[k+1] == '*') ||
                            (k+2 < strlen(regex) && regex[k] == '\\' && regex[k+2]=='*'))
                    {                                                   // If the k value matched, it would continue to the oneOrMore, like the +
                        if(regex[k+1] == '*') k+=1;                     // call above.
                        else k+=2;
                        offset-=1;
                    }
                    else
                    {
                        offset = 0;                                     // reset the offset variable
                        break;                                          // exit the loop
                    }
                }
                if(k == strlen(regex)-1)                                // If the loop reaches the last characer
                {                                                       // Then we have found a match
                    if((readLine[strlen(readLine)-1] == '\n') || (readLine[strlen(readLine)-1] == '\r')) readLine[strlen(readLine)-1] = '\0';
                    strcpy(matches2[curWord],readLine);
                    matched_lines +=1;
                    curWord +=1;
                    offset = 0;
                    didPrint = true;                                    // Sets this boolean, which prevents printing the same line
                    break;                                              // more than once
                }
                offset += 1;                                            // Increment offet if the loop hasnt broken by now
            }
        }
    }

    free(readLine);
    *matches = matches2;
    return matched_lines;
}

/*********************************************************************************
 validate:  Called by main()

            Takes in a string representing the entire input file, validates the
            proper words. Skips scripts, tags, other stop words. Adds
            the necessary words and other information to the hashtables

            Parameters: The input file represented as one long string

            Returns: Nothing, modifies the hash tables
 *********************************************************************************/
void validate(char* line)
{
    bool isScript = false;                                                          // Boolean to represent if currently in a script tag

    // The words stored as each moves along ... if it found, move
    // word --> word2, and word2 --> word1, where word1 is forgotten
    // Represented in the bigram hash by "--word2-- --word--"
    // Represented in the trigram hash by "--word1-- --word2-- --word--"
    char* word = calloc(60, sizeof(char));
    char* word1 = calloc(60, sizeof(char));
    char* word2 = calloc(60, sizeof(char));
    char* temp = calloc(60, sizeof(char));
    int stringL = strlen(line);

    int wordLoc = 0;
    bool firstQuote = false;

    for(int i = 0; i < stringL; i++) {                                          // Looping Across the entire input file as a line
        if(isScript == true)
        {
            if(i+8 < stringL && line[i] == '<' && line[i+1] =='/'               // Checking to see if there is the ending of the script tag
               && line[i+2] =='s' && line[i+3] == 'c' && line[i+4] == 'r'       // if so... allow words to be added
               && line[i+5]=='i' && line[i+6] == 'p' && line[i+7] == 't'
               && line[i+8] == '>')
            {
                isScript = false;
                i+=8;
            }
            continue;                                                           // and move onto the next character
        }
        else if(isScript == false)
        {
            if(i+6<stringL && line[i] == '<' && line[i+1] =='s'                 // just the opposite... if a script tag is found
               && line[i+2] =='c' && line[i+3] == 'r' && line[i+4] == 'i'       // we set a booleab, and continu
               && line[i+5] =='p' && line[i+6] == 't')
            {
                i+=6;
                isScript = true;
                continue;
            }
                                                                                // if its not a script, we can focus on adding to a word
            int placeHolder = 0;
            if(line[i] == '<'){                                                 // removing tags... if a open tag is found that is not a
                placeHolder = i;                                                // a script tag, we loop until the matching ending tag is found
                while(line[i]!='\0')
                {
                    if(line[i] == '>') break;
                    i++;
                }
                if(line[i] == '\0') i=placeHolder;                              // If the matching tag is not found, use the < as a space delimeter
            }

            if(line[i] == '&')                                                  // Skipping over the &_____ stopwords
            {
                bool is_skip  = false;
                if(i+5<stringL && line[i+1] == 'n' && line[i+2] == 'b'
                   && line[i+3] == 's'  && line[i+4] == 'p' && line[i+5] == ';' ) {i+=5; is_skip = true; }
                else if(i+5<stringL && line[i+1] == 'q' && line[i+2] == 'u'
                        && line[i+3] == 'o'  && line[i+4] == 't' && line[i+5] == ';' ) {i+=5; is_skip = true; }
                else if (i+4<stringL && line[i+1] == 'a' && line[i+2] == 'm'
                         && line[i+3] == 'p' && line[i+4] == ';'  ) {i+=4; is_skip = true; }
                else if (i+3<stringL && line[i+1] == 'l' && line[i+2] == 't'
                         && line[i+3] == ';' ) {i+=3; is_skip = true; }
                else if (i+3<stringL && line[i+1] == 'g' && line[i+2] == 't'
                         && line[i+3] == ';' ) {i+=3; is_skip = true; }
                if(is_skip == true) continue;
            }

            if(line[i] == '\'' && wordLoc != 0 && isalpha(line[i+1]) && firstQuote == false){
                word[wordLoc] = line[i];                                        // Determines the number of quotations
                wordLoc++;                                                      // allowed in the word, which is one.
                i++;                                                            // THis is represented by the firstQuote bool.
                firstQuote = true;
            }

            if(isalpha(line[i]))                                                // If all else prevails, the word is an alpha value
            {                                                                   // and can be added to the word
                word[wordLoc] = tolower(line[i]);
                wordLoc++;
            }

            // Adding the current word to the three hash tables -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
            if(!isalpha(line[i]) || line[i+1] == '\0'){                         // Checking to see if the word is valid on a space delimeter
                word[wordLoc] = '\0';
                if(strlen(word) >= 2)
                {                                                               // ADDING TO THE WORDS HASH
                    HASH_FIND(ah, words, word, strlen(word), tmp1);             // Checks the users hash to see if it is already in it
                    if(tmp1!=NULL){
                        tmp1->count +=1;                                        // if it is... simply add to the count of that string
                        tmp1 = NULL;
                    }
                    else
                    {
                        w = (struct my_struct *)malloc(sizeof *w);              // If not, I create a new entry in the hash
                        strcpy(w->name, word);
                        w->id = id;
                        w->count = 1;
                        HASH_ADD_KEYPTR( ah, words, w->name, strlen(w->name), w );
                        id+=1;
                    }
                    if(strlen(word2)>0)
                    {
                        bool did_pass = true;                                           // ADDING TO THE BIGRAM HASH:
                        for(int l=0; l<48; l++)                                         // Looping across the stop words array...
                        {                                                               // ensuring that word and word2 are not
                            if(strcmp(skip[l], word)==0) did_pass = false;              // stop words
                            if(strcmp(skip[l], word2)==0) did_pass = false;
                        }

                        if( did_pass == true && strlen(word2) >=2)
                        {
                            strcpy(temp, word2);
                            strcat(temp, " ");
                            strcat(temp, word);                                         // We create a bigram of the two words
                            HASH_FIND(hh, bigrams, temp, strlen(temp), tmp2);           // See if the bigram is already in that hash
                            if(tmp2 != NULL)
                            {
                                tmp2->count+=1;                                         // Add to the counter if it is
                                tmp2 = NULL;
                            }
                            else
                            {
                                b = (struct my_struct *)malloc(sizeof *b);              // If not, I add to the bigram hash
                                strcpy(b->name, temp);
                                b->id = Bid;
                                b->count = 1;
                                HASH_ADD_KEYPTR( hh, bigrams, b->name, strlen(b->name), b );
                                Bid+=1;
                            }
                            bool did_pass2 = true;                                      // ADDING TO THE TRIGRAM HASH
                            for(int l=0; l<48; l++){                                    // Looping across the stop words array..
                                if(strcmp(skip[l], word1)==0) did_pass2 = false;        // ensuring that word1 isnt a stop word
                            }
                            if(did_pass2 == true && strlen(word1) >=2)
                            {
                                strcat(word1, " ");                                     // I create the Trigram using the Bigram
                                strcat(word1, temp);                                    // We create a bigram of the two words
                                HASH_FIND( bh, trigrams, word1, strlen(word1), tmp3);   // See if the bigram is already in that hash
                                if(tmp3 != NULL)
                                {
                                    tmp3->count+=1;                                     // Add to the counter if it is
                                    tmp3 = NULL;
                                }
                                else
                                {
                                    t = (struct my_struct *)malloc(sizeof *t);          // If not, I add to the trigram hash
                                    strcpy(t->name, word1);
                                    t->id = Tid;
                                    t->count = 1;
                                    HASH_ADD_KEYPTR( bh, trigrams, t->name, strlen(t->name), t);
                                    Tid+=1;
                                }

                            }
                        }

                    }
                    strcpy(word1, word2);   // Moving word2-->word1
                    strcpy(word2, word);    // Moving word-->word2
                }
                wordLoc = 0;                // Reseting all of the values used to store the word
                firstQuote = false;
                free(word);
                word = calloc(60, sizeof(char));
                continue;
                // FInished dding the current word to the three hash tables -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
            }
        }
    }

    // Freeing the used memory above
    free(temp);
    free(word);
    free(word1);
    free(word2);
}

/*********************************************************************************
 print_hash():  Called by main()

                Prints the hash tables and their statistics

                Parameters: The number of arguments in the command line

                Returns: Nothing
 *********************************************************************************/
void print_hash(int arguments)
{
    struct my_struct *m;
    HASH_SRT(ah, words, sort_by_name);                              // Sorts the words hash by name
    HASH_SRT(ah, words, count_sort);                                // Sorts the words hash by the count of each word
    HASH_SRT(hh, bigrams, sort_by_name);                            // Sorts the bigrams hash by name
    HASH_SRT(hh, bigrams, count_sort);                              // Sorts the bigrams by the count of each bigram
    HASH_SRT(bh, trigrams, sort_by_name);                           // Sorts the trigrams by the name of each trigram
    HASH_SRT(bh, trigrams, count_sort);                             // Sorts the trigram by the count of each trigram

    int unique = 0, totalWords = 0;
    for(m=words; m != NULL; m=(struct my_struct*)(m->ah.next)) {    // Looping over the words hash with an iterator
        unique+=1;                                                  // Calculating the number of unique words
        totalWords+=m->count;                                       // Calculating the number of total Words
    }

    int uniqueBigrams = 0, totalBigrams = 0;
    for(m=bigrams; m != NULL; m=(struct my_struct*)(m->hh.next)) {  // Looping over the bigrams hash with an iterator
        uniqueBigrams+=1;                                           // Calculating the number of unique bigrams
        totalBigrams+=m->count;                                     // Calculating the number of total bigrams
    }

    int uniqueTrigrams = 0, totalTrigrams = 0;
    for(m=trigrams; m != NULL; m=(struct my_struct*)(m->bh.next)) { // Looping over the trigrams hash with an iterator
        uniqueTrigrams+=1;                                          // Calculating the number of unique trigrams
        totalTrigrams+=m->count;                                    // Calculating the number of total trigrams
    }

    // Printing the Calculated Statistics
    printf("Total number of documents: %d\n", arguments-1);
    printf("Total number of words: %d\n", totalWords);
    printf("Total number of unique words: %d\n", unique);
    printf("Total number of interesting bigrams: %d\n", totalBigrams);
    printf("Total number of unique interesting bigrams: %d\n", uniqueBigrams);
    printf("Total number of interesting trigrams: %d\n", totalTrigrams);
    printf("Total number of unique interesting trigrams: %d\n", uniqueTrigrams);

    printf("\nTop 50 words:\n");
    // PRINTING THE TOP 50 WORDS
    int j = 0;
    for(m=words; m != NULL; m=(struct my_struct*)(m->ah.next)) {
        if(j==50) break;
        printf("%d %s\n", m->count, m->name);
        j+=1;
    }
    printf("\nTop 20 interesting bigrams:\n");
    // PRINTING THE TOP 20 INTERESTING BIGRAMS
    j=0;
    for(m=bigrams; m != NULL; m=(struct my_struct*)(m->hh.next)) {
        if(j==20) break;
        printf("%d %s\n", m->count, m->name);
        j+=1;
    }
    printf("\nTop 12 interesting trigrams:\n");
    // PRINTING THE TOP 12 INTERESTING TRIGRAMS
    j=0;
    for(m=trigrams; m != NULL; m=(struct my_struct*)(m->bh.next)) {
        if(j==12) break;
        printf("%d %s\n", m->count, m->name);
        j+=1;
    }
}


/*********************************************************************************
 main:  Reads in the files from the command line.

        Calls: num_ines, one_line, validate, and print_hash
 *********************************************************************************/
#ifndef USE_SUBMITTY_MAIN
int main(int argv, char* argc[])
{
    setvbuf( stdout, NULL, _IONBF, 0 );
    if(argv<2)                                      // Error Checking, handling correct number of arguments
    {
        fprintf(stderr, "ERROR: Invalid arguments\n");
        fprintf(stderr, "USAGE: a.out <input-file1> [ <input-file2> ... ]\n");
        exit(EXIT_FAILURE);                         // Terminates the script
    }

    for(int i=1; i<argv; i++)                       // Loop through all the files specified in the command line
    {
        FILE* inputFile = fopen(argc[i], "r");      // Reads in the file
        if(inputFile == NULL)                       // Error Handling continued --> checks to see if file
        {                                           // actually exists
            fprintf(stderr, "ERROR: Invalid arguments\n");
            fprintf(stderr, "USAGE: a.out <input-file1> [ <input-file2> ... ]\n");
            exit(EXIT_FAILURE);                     // Terminates the script
        }
        if (NULL != inputFile) {
            fseek(inputFile, 0, SEEK_END);
            int csize = ftell(inputFile);

            if (0 == csize) {
                printf("file is empty\n");
            }
            rewind(inputFile);
        }

        int fileSize = num_lines(inputFile);            // calls numLines, calcs number of lines in the file
        char *readLine = one_line(inputFile, fileSize); // puts the entire file into one string! no
        validate(readLine);                             // Calculates everything for the document, stores in hash

        free(readLine);
        fclose(inputFile);
    }

    print_hash(argv);

    HASH_ITER(ah, words, w, tmp1) {                     // Words Hash Memory Clean
        HASH_DELETE(ah, words, w);
        free(w);
    }

    HASH_ITER(hh, bigrams, b, tmp2) {                   // Bigram Hash Memory Clean
        HASH_DELETE(hh, bigrams, b);
        free(b);
    }

    HASH_ITER(bh, trigrams, t, tmp3) {                  // Trigram Hash Memory Clean
        HASH_DELETE(bh, trigrams, t);
        free(t);
    }

    free(tmp1);                                         // Freeing the struct iterators
    free(tmp2);
    free(tmp3);

    return EXIT_SUCCESS;
}
#endif

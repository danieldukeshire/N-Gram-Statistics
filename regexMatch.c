#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>


/*********************************************************************************
 numLines: Called by main, loops through the input file to determine the number of
 lines. Outputs an integer, takes FILE inputFile as a parameter.
 *********************************************************************************/
int numLines(FILE* inputFile)
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

/********************************************************************************
 oneOrMore: Takes int: current expression iterator k, current readLine iterator
 offset and o (location is offset + o, where o (j in main) is the location and
 offset is the number of characters away from the o), and char*: expression string,
 and readline string as char*. Focuses on Greedily finding the character to end on
 proceeding the '+', then looping backwards to determine if all characters between
 the two boundary characters are valid. Called by parseLines.
 Does not account for +? and +* or *? or *+
 ********************************************************************************/
int oneOrMore(int k, int offset, int o, const char* expression, const char* readLine, int l)
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
                if(k>=2-l && expression[k-2-l] == '\\')                     // checks to see if every value until i is of the right type :
                {                                                                           // BREAKS IF:
                    if(expression[k-1-l] == 'd' && isdigit(readLine[j])==0) break;          // Should be a digit
                    else if(expression[k-1-l] == 'D' && isdigit(readLine[j])!=0) break;     // Shouldnt be a digit
                    else if(expression[k-1-l] == 'w' && isalpha(readLine[j])==0) break;     // Should be a alpha
                    else if(expression[k-1-l] == 'W' && isalpha(readLine[j])!=0) break;     // Shouldnt be an alpha
                    else if(expression[k-1-l] == 's' && isspace(readLine[j])==0) break;     // Should be a space
                    else if(expression[k-1-l] == '\\' && readLine[j] != '\\') break;        // Should be a '\'
                }
                else if(expression[k-1-l] == ']')                     // Accounting for bracket representation
                {
                    bool found = false;
                    bool is_not = false;
                    int y = k-2;
                    for( ; y>0; y--) if(expression[y] == '[') break;// Determines where to start the loop
                    y+=1;                                           // Moves the location nack to
                    for(; y<strlen(expression); y++)
                    {
                        if(expression[y] == ']') break;
                        if(expression[y] == '^') is_not = true;     // Finds the carrot "not"
                        else if(expression[k]=='.'){                // Repeats previous code
                            found = true;                           // This is to account for this syntax WITHIN the brackets
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
            return oneOrMore(k+3, offset, o, expression, readLine, 3)+1;      // Calls the program recursively, now with a new ending
        }
        else if(k+2<strlen(expression) && (expression[k+2] == '?' || expression[k+2] == '*'))
        {
            return oneOrMore(k+2, offset, o, expression, readLine, 2)+1;
        }
        else if(expression[k+1]=='[')                                       // Accounts for []
        {
            for(int m=k+1; m<strlen(expression); m++)
            {
                if(expression[m] == '?')
                    return oneOrMore(m, offset, o, expression, readLine, k-m);
            }
        }
    }
    return 0;          // if never returns an i value, return 0. This means doesItMatch = false
}                      // in the main function.



/*********************************************************************************
 regexmatch: Called by main, loops through the input file line by line to determine
 all lines matching the regex expression, which are immediately printed if found.
 Takes the int size of the inputFile (# of lines), string expression (the regex input),
 and the inptFile as a FILE.
 ********************************************************************************/
int regex_match(const char* filename, const char* regex, char*** matches)
{
    FILE* inputFile = fopen(filename, "r");
    if(inputFile == NULL)
    {
        fprintf(stderr, "ERROR: Invalid arguments\n");
        fprintf(stderr, "USAGE: a.out <input-file1> [ <input-file2> ... ]\n");
        exit(EXIT_FAILURE);                                 // Terminates the script
    }
    else if(regex == NULL) return 0;
    int size = numLines(inputFile);
    
    //if(expression[strlen(expression)-1] == '\n') expression[strlen(expression)-1] = '\0';
    char *readLine = calloc(1024, sizeof(char));            // Allocates space for each line in the file
    char **matches2 = calloc(size, sizeof(char *) );
    for(int m=0; m< size; m++) {
        matches2[m] = calloc(1024, sizeof(char));
    }
   
    int matched_lines = 0;
    int curWord = 0;
    // MAIN LOOP: Loops through the entire input file
    for(int i=0; i<size; i++)
    {
        bool didPrint = false;                                  // A boolean to exit the loop if the line is printed
        fgets(readLine, 1024, inputFile);                        // Reads the line in
        //if(readLine[strlen(readLine)-1] == '\n') readLine[strlen(readLine)-1] = '\0';
        for(int j=0; j<strlen(readLine); j++)                   // Loops across each line in the input file
        {
            if(didPrint == true) break;                         // Breaks the loop if the line has already been printed
            int offset = 0;                                     // allows the inner loop to continue without loosing j index
            for(int k=0; k<strlen(regex); k++)                  // Loops across the expression
            {
                if((offset+j)>strlen(readLine)) break;
                bool doesItMatch = false;                       // boolean which stores if the current character matches the regex
                
                // Accounting for the Regex Syntax
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
                    temp = oneOrMore(k, offset-1, j, regex, readLine, 0);       // Determines if there are more values!
                    if(temp != 0 )
                    {
                       
                        offset = temp-j;                                                // As temp is o (j) and offset combined, must subtract j
                        if(k+1< strlen(regex) && regex[k+1] == '[') while(regex[k]!=']') k+=1;
                        else if(k+3 < strlen(regex) && regex[k+3] == ('*' | '?') && regex[k+1] == '\\'){k+=3;} //offset-=1;}
                        else if(k+2 < strlen(regex) && regex[k+2] == ('*' | '+' | '?')) { k+=2;} //offset-=1; }
                        else if(k+2<strlen(regex) && regex[k+1] == '\\') k+=2;
                        else if (k+1<strlen(regex) && regex[k+1] != '\0') k+=1;         // If the + was the last character, only add one!

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
                    if(readLine[strlen(readLine)-1] == '\n') readLine[strlen(readLine)-1] = '\0';
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

//#ifdef USE_SUBMITTY_MAIN
int regex_match( const char * filename, const char * regex, char *** matches );

int main( int argc, char * argv[] )
{
    if ( argc != 3 )
    {
        fprintf( stderr, "ERROR: Invalid arguments\n" );
        fprintf( stderr, "USAGE: %s <regex-file> <input-file>\n", argv[0] );
        return EXIT_FAILURE;
    }
    
    FILE * regexfile = fopen( argv[1], "r" );
    
    if ( regexfile == NULL )
    {
        perror( "fopen() failed" );
        return EXIT_FAILURE;
    }
    
    char * regex = calloc(256, sizeof(char));
    fgets(regex, 256, regexfile);
    fclose( regexfile );
    
//#ifdef DEBUG_MODE
    printf( "REGEX: \"%s\"\n", regex );
//#endif
    
    
    /* call regex_match() to find matches of a given regex   */
    /*  in a given file (argv[2]); lines must be dynamically */
    /*   allocated in the regex_match() function call        */
    char ** lines = NULL;
    int matched_lines = regex_match( argv[2], regex, &lines );
    int i;
    
    /* display resulting matched lines */
    for ( i = 0 ; i < matched_lines ; i++ )
    {
        printf( "%s\n", lines[i] );
    }
    
    /* deallocate dynamically allocated memory */
    for ( i = 0 ; i < matched_lines ; i++ )
    {
        free( lines[i] );
    }
    
    free( lines );
    
    return EXIT_SUCCESS;
}

//#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXL 30

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

struct wordEntry
{
   char aWord[MAXL + 1];          // stores a word assuming maximum characters expected in a word
};

/*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

    autoSpellLev - a non-interactive spelling text correction program
    Copyright (C) 2016  Perry T Jennings

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

Any similarity of the name of this program to another program not written by this author is coincidental and unintended and does not indicate or imply an association.

usage: autoSpellLev text_file_to_correct dictionary
purpose: to non-interactively correct some spelling mistakes (such as in a text file produced by optical
character recognition - ocr)
output is to standard output.
In other words this program takes a text file with spelling errors and prints to standard output the same text with some spelling corrections.  It does not interactively ask you to select or approve the changes since a word of seven or more characters with one or two wrong characters most likely is misspelled and has a high probability of being matched to just one correct word.  Beside, with an ocr text you are going to have to manually scan through the whole file anyway for possible ocr mistakes and garbled text.  This program is intended to save time and labor with at least the larger words.

below are the program parameters the user may change, but give careful thought before you do:

MAXL 		is maximum characters for a word.  For utf-8 texts you may need to account
 		  for possibility of one to four bytes per character
minCharWord   	the minimum number characters in a word to search for similar words
		(smaller words are simply printed out without searching for replacement)
maxNumWords   	maximum number of dictionary words this program can read and use.
		This figure must of course be at least as large as the number of words in dictionary.

The dictionary (the second argument to this program) must be a single column of words sorted from shortest
to longest.  It is adviseable to also secondarily sort the words such that words of the same length 
are sorted from most frequent to least. 

The author concurrently wrote two similar programs - autoSpellSim and autoSpellLev. 
Their purpose was to non-interactively and automatically correct a large number of
ocr mistakes in a 4.5M text file.  It was observed that most errors are of the form
decernamtts vs. decernamus (ocr made tt from u) or constarc vs. constare (transposition).
autoSpellSim was written to find and correct these errors.
Then the author discovered the concept of edit distance and various tools and theories of
fuzzy searching and spell checking.  The other program autoSpellLev uses the well-known
Levenshtein algorithm.

future work: write another version to better handle wide characters (non-ascii) - as yet a velleity.

Perry  res_emptito@yahoo.com
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

/**************************** function xtractstr ***************************************/

static _Bool xtractstr ( char *substring, const char *mainstr, const int start, const int charLen )
{
   // function to extract a substring given the starting index in mainstr and character length
   int k;
   int ll = (int) strlen(mainstr);
   if (start + charLen > ll || start < 0 || charLen < 0 || ll == 0 || mainstr[ll] != '\0' )
      return (_Bool) 0;
   substring[charLen] = '\0';
   for ( k = 0; k < charLen; k++)
      substring[k] = mainstr[start + k];
   return (_Bool) 1;
}

/***************************** function lenOfCharSet ******************************/

static int lenOfCharSet ( const int lastPos, const char *dString, const char *aLine )
{
   /* function to return the leftmost substring length in the given character array aLine having one or
      more of the characters in the dString character set (delimiters or punctuation characters)
      start searching from the index lastPos in aLine
      send lastPos = -1 to this function when starting a new line */

   int dStringLength = (int) strlen(dString);
   int lineLength = (int) strlen(aLine);
   int charsLeft = lineLength - lastPos - 1;
   char subString[charsLeft + 1];

   // validate function input
   if ( dStringLength == 0 || dString[dStringLength] != '\0')
   {
      fprintf (stderr, "dString string sent to function lenOfCharSet empty or not null-terminated\n");
      exit (EXIT_FAILURE);
   }
   if ( aLine[lineLength] != '\0')
   {
      fprintf (stderr, "aLine string sent to function lenOfCharSet not null-terminated\n");
      exit (EXIT_FAILURE);
   } 
   if ( lastPos + 1 > lineLength || lastPos < -1)
   {
      fprintf( stderr, "index of line string sent to function lenOfCharSet is beyond array limit.\n");
      exit (EXIT_FAILURE); 
   }

   if ( lineLength == 0 )
      return -1;   // return code for blank line (same as for starting a new line)
   if ( lastPos + 1 == lineLength)
      return -2;   // return code for index at last position

   if ( xtractstr( subString, aLine, lastPos + 1, charsLeft ) )  // extracts substring = right side portion of aLine
      return (int) strspn ( subString, dString );   //  take this return value and in main() add it to lastPos to get new lastPos
      //   strspan returns maximum number of initial characters in subString consisting only of characters in dString
      //  if 0 then next string in aLine isn't a delimiter
   else
      return -3;
}  

/***************************** function negLenOfCharSet ******************************/

static int negLenOfCharSet ( const int lastPos, const char *dString, const char *aLine )
{
   /* function to return the leftmost substring length in the given character array aLine having none
      of the characters in the dString character set (delimiters or punctuation characters)
      start searching from the index lastPos in aLine
      send lastPos = -1 to this function when starting a new line */

   int dStringLength = (int) strlen(dString);
   int lineLength = (int) strlen(aLine);
   int charsLeft = lineLength - lastPos - 1;
   char subString[charsLeft + 1];

   // validate function input
   if ( dStringLength == 0 || dString[dStringLength] != '\0')
   {
      fprintf (stderr, "dString string sent to function lenOfCharSet empty or not null-terminated\n");
      exit (EXIT_FAILURE);
   }
   if ( aLine[lineLength] != '\0')
   {
      fprintf (stderr, "aLine string sent to function lenOfCharSet not null-terminated\n");
      exit (EXIT_FAILURE);
   } 
   if ( lastPos + 1 > lineLength )
   {
      fprintf( stderr, "index of line string sent to function lenOfCharSet is beyond array limit.\n");
      exit (EXIT_FAILURE); 
   }

   if ( lineLength == 0 )
      return -1;   // return code for blank line (same as for starting a new line)
   if ( lastPos + 1 == lineLength)
      return -2;   // return code for index at last position

   if ( xtractstr ( subString, aLine, lastPos + 1, charsLeft ) )
      return (int) strcspn ( subString, dString );
   else
      return -3;
}  

/***************************** function levenshtein ******************************/

static int levenshtein(const char *s1, const char *s2)

/*
Purpose of this function is to return the Levenshtein "distance" between two words which is an indication of their similarity.
Taken from https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C
*/

{
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = (unsigned int) strlen(s1);
    s2len = (unsigned int) strlen(s2);
    unsigned int column[s1len+1];
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x-1; y <= s1len; y++) {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y-1] + 1, lastdiag + (s1[y-1] == s2[x-1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return(column[s1len]);
}

/***************************** function returnWord ******************************/

static int returnWord (char *outputWord, const char *inputWord, struct wordEntry wordArray[], long int *wordLenIndex, long int *correctedWrdCt )

/* This function is custom-made to work with function levenshtein.
   Send dictionary words to levenshtein equal and one less character than inputWord because sometimes ocr programs
   make two letters out of one like h -> li.
   Make copy of inputword to send here since we may change the first character to lower if upper  
   to preserve an initial capital letter. 
*/

{
char givenWord[MAXL + 1];                                    // the inputWord subject to modification
long int i;                                                  // multi-use integers for counters, etc.
long int k;
int maxED;                                                   // maximum permissible edit distance for a word to match
int lastSR;
int levResult;
_Bool capitalized = 0;                                       // true if inputWord has 1st letter capitalized
long int startIndex;                                         // starting index to search in wordArray
long int endIndex;                                           // ending index to search in wordArray
long int nWords = wordLenIndex[0];
int inputWrdLen = (int) strlen( inputWord );

strcpy( outputWord, inputWord );                             // default return word
strcpy( givenWord, inputWord );
if ( inputWrdLen <= 4 )                                      // ridiculous to auto-correct tiny words
   return 0;
if ( wordLenIndex[inputWrdLen] == 0 )
   return 0;                                                 // there are no available dictionary words that could match

if ( inputWrdLen <= 7 )
   maxED = 1;
else if ( 7 < inputWrdLen && inputWrdLen <= 13 )
   maxED = 2;
else
   maxED = 3;

lastSR = maxED + 1;
if ( givenWord[0] == toupper( givenWord[0] ))                // preserve capitalization of inputWord to outputWord
{
   capitalized = (_Bool) 1;
   givenWord[0] = tolower( givenWord[0] );
}
startIndex = wordLenIndex[inputWrdLen];
i = inputWrdLen + 1;
if ( i <= MAXL && wordLenIndex[i] != 0 )
   endIndex = wordLenIndex[i] - 1;                           // the usual case: last (highest) index for words of same length
else
{
   endIndex = nWords - 1;
   for ( k = MAXL; k > i; k-- )
      wordLenIndex[k] && ( endIndex = wordLenIndex[k] - 1 ); 
}

i = startIndex;
while ( i <= endIndex && levResult != 0 )
{
   levResult = levenshtein( givenWord, wordArray[i].aWord );
   if ( levResult < lastSR )
   {
      strcpy( outputWord, wordArray[i].aWord );
      lastSR = levResult;
   }
   i++;
}

if (0 < lastSR && lastSR <= maxED )
   (*correctedWrdCt)++;
if ( capitalized )
   outputWord[0] = toupper( outputWord[0] );
return 1;
}


/***************************************** main **********************************************/

long int main(int argc, char *argv[])      // usage: autoSpellLev text_file_to_correct dictionary
{
   const size_t minCharWord = 7;           // the minimum number characters in a word to search for similar words
   const long int maxNumWords = 200000;    // maximum number of dictionary words this program can read and use
   FILE *ocrTextFile;                      // the text file to auto-correct for ocr mistakes
   FILE *wordList;                         /* one column - the word list is sorted (1st) in ascending length and (2nd)
                                              in descending frequency of use */
   char *line = NULL;                      // used in the getline function; initialize to NULL if not using malloc
   size_t len = 0;                         // used in the getline function; otherwise not needed
   size_t lastWordSize = 0;                   // use to make wordLenIndex[]
   size_t wordSize;                        // temporary variable (for readability) used to make wordLenIndex[]
   ssize_t read;                           // used in getline
   long int numWords = 0;                  // the actual number of entries in WordArray
   long int corrWrdCt = 0;                 /* a running count of the number of corrections made - just for reference
                                              and interest; printed on last line of output */
   long int wordLenIndex[MAXL];            /* the index [0..MAXL] represents word length (strlen) while the value stored
                                              is the index in wordArray where a word of that length first appears */
   // struct wordEntry wordArray[maxNumWords]; 
   int lastPos, dLen, wLen;
   char string[MAXL + 1];                  // string (delimiter or word) read from a line in ocrTextFile
   char simOrSame[MAXL + 1];               // returned word from function returnWord
   long int return_code = 1;               /* default return code for main(); if negative then one or more words or
                                              delimiter strings was greater than MAXL and was printed without processing */
                                           /* char delimiters[] separate words.  Have to escape the quotes to get
                                              them in the string */
   struct wordEntry* wordArray = malloc( maxNumWords * sizeof( *wordArray ));
   char delimiters[] = " .,?!\';\n:-()\"\t";
   


   if (argc != 3)
   {
      fprintf (stderr, "required input of textfile with ocr errors and word list used to automatically correct it\n");
      free( wordArray );
      return -1;
   };

   if ((ocrTextFile = fopen (argv[1], "r+")) == NULL)
   {
      fprintf (stderr, "cannot read the input text file %s\n", argv[1]);
      free( wordArray );
      return -2;
   };  

   if ((wordList = fopen (argv[2], "r")) == NULL)
   {
      fprintf (stderr, "cannot read the input dictionary %s\n", argv[2]);
      free( wordArray );
      return -3;
   }; 

// now the main body......................................

   for (numWords = 0; numWords <= MAXL + 1; numWords++) 
      wordLenIndex[numWords] = 0;    //initialize 
   numWords = lastWordSize = 0; 
   while ( feof( wordList ) == 0 && numWords < maxNumWords )
   {
      (void) fscanf(wordList, "%s", wordArray[numWords].aWord);  // loop to read in the column of dictionary words
      wordSize = strlen(wordArray[numWords].aWord);
      if ( wordSize > lastWordSize )
      {
         lastWordSize = wordSize;
         wordLenIndex[wordSize] = numWords;
         /*wordLenIndex stores the index of wordArray using its own index equal to the first occurance of strlen(x).
           The word length is increasing in the sorted word list we are reading in.  So, if, for example, the first time 
           the program encounters a word of length 7 is at the 116th word read in then wordLenIndex[7] = 116.  If no words
           are of length 7 then wordLenIndex[7] stays initialized at 0 which is a flag for later use.  This wordLenIndex array
           will speed up the search for similiar words since we still know where in the word list to start and stop looking. */
      }
      if ( wordSize < lastWordSize && wordSize > 0)
      {
         fprintf( stderr, "the input dictionary is not properly sorted\n");
         exit (EXIT_FAILURE);
      }
      numWords++;
   }
   wordLenIndex[0] = numWords;   //  convenient place to store size of array
   (void) fclose(wordList); 
   if ( numWords >= maxNumWords )
   {
      fprintf (stderr, "word list greater than program maximum of %ld\n", maxNumWords );
      free( wordArray );
      return -4;
   }

   while ((read = getline(&line, &len, ocrTextFile)) != -1)    //see getline_help_and_notes file
   {
      lastPos = -1;
      if ( (size_t) read < minCharWord )
         printf( "%s", line );
      else
      {  
         while ( 1 )  // loop to parse and process all words on a line
         {
            dLen = lenOfCharSet (lastPos, delimiters, line);  // length of delimiter string
            if ( dLen == -2 ) 
               break;  // at end of line
            if ( dLen == -3 )
            {
               fprintf (stderr, "problem in lenOfCharSet function\n");
               exit (EXIT_FAILURE);
            }
            if ( dLen > 0 )
               if (dLen > MAXL )
               {
                  char *wrdOverMAXL = malloc(4*dLen*sizeof(char)); // utf-8 char requires 1 to 4 bytes
                  if (wrdOverMAXL == NULL || ! xtractstr( wrdOverMAXL, line, lastPos + 1, dLen ))
                  {
                     fprintf(stderr, "problem in malloc wrdOverMAXL or in function xtractstr\n" );
                     if ( wordArray )
                        free( wordArray );
                     if ( line )
                        free( line );
                     exit ( EXIT_FAILURE );
                  }
                  else
                  {
                     printf("%s", wrdOverMAXL);
                     free( wrdOverMAXL ); 
                  }
                  lastPos = lastPos + dLen;
                  return_code = -5;
               }
               else if (xtractstr( string, line, lastPos + 1, dLen ))  // the commonly expected condition, just find the delimiters and print them
               {
                  lastPos = lastPos + dLen;
                  printf( "%s", string );
               }
               else
               {
                  fprintf(stderr, "problem in xtractstr function\n");
                  exit (EXIT_FAILURE);
               }
            wLen = negLenOfCharSet ( lastPos, delimiters, line);  // length of word string (search by whatever is not a delimiter)
            if ( wLen == -2 ) 
               break;  // at end of line
            if ( wLen == -3 )
            {
               fprintf (stderr, "problem in a negLenOfCharSet function\n");
               exit (EXIT_FAILURE);
            }
            if ( wLen > 0 )
               if (wLen > MAXL )
               {
                  char *wrdOverMAXL = malloc(4*wLen*sizeof(char));  // utf-8 char requires 1 to 4 bytes
                  if ( wrdOverMAXL == NULL || ! xtractstr( wrdOverMAXL, line, lastPos + 1, wLen ) )
                  {
                     fprintf(stderr, "problem in malloc wrdOverMAXL or in function xtractstr\n" );
                     if ( wordArray )
                        free( wordArray );
                     if ( line )
                        free( line );
                     exit (EXIT_FAILURE);
                  }
                  else
                  {
                     printf( "%s", wrdOverMAXL );
                     free( wrdOverMAXL ); 
                  }
                  lastPos = lastPos + wLen;
                  return_code = -6;
               }
               else if ( wLen < (int) minCharWord || wLen > (int) lastWordSize )  // just print words too short or too long
                  if ( xtractstr( string, line, lastPos + 1, wLen ) )
                  {
                     lastPos = lastPos + wLen;
                     printf( "%s", string );
                  }
                  else
                  {
                     fprintf(stderr, "problem in xtractstr function\n");
                     exit (EXIT_FAILURE);
                  }
               else if ( xtractstr( string, line, lastPos + 1, wLen ) )  // the common condition, where words get processed for correction
               {
                  lastPos = lastPos + wLen;
                  (void) returnWord( simOrSame, string, wordArray, wordLenIndex, &corrWrdCt );
                  printf( "%s", simOrSame );
               }
               else
               {
                  fprintf(stderr, "problem in xtractstr function\n");
                  exit (EXIT_FAILURE);
               }
         }       // while 1 loop to process a line of text read from the file to be corrected
      }          // else read a minimum length of text to process
   }             // while getline


   fclose(ocrTextFile);
   if ( wordArray )
      free( wordArray );
   if ( line )
      free( line );
   if ( return_code > 0 )
      return_code = corrWrdCt;
   return return_code;   // echo $? to get return value which equals number of corrections made if no errors
}

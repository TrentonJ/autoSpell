# autoSpell
non-interactive spelling correction for plain text file

usage: autoSpellLev text_file_to_correct dictionary

purpose: to non-interactively correct some spelling mistakes (such as in a text file produced by optical
character recognition - ocr)
output is to standard output.

In other words this program takes a text file with spelling errors and prints to standard output the same text with some spelling corrections.  It does not interactively ask you to select or approve the changes since a word of seven or more characters with one or two wrong characters most likely is misspelled and has a high probability of being matched to just one correct word.  Beside, with an ocr text you are going to have to manually scan through the whole file anyway for possible ocr mistakes and garbled text.  This program is intended to save time and labor with at least the larger words.

below are the program parameters the user may change, but give careful thought before you do:

MAXL 		        is maximum characters for a word.  For utf-8 texts you may need to account
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

**update**
Now I realize I should have simply used a function pointer in a single program and with input from the user select which filter to use.  But I'm a slightly better coder now. :)

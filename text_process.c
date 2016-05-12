#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

int remove_non_chars( char * line )
{
	int i;
	int len = strlen(line);
	int err = 0;

	for(i=0; i<len; i++)
	{
		if ( !(line[i]>='a' && line[i]<='z') && 
		     !(line[i]>='A' && line[i]<='Z') &&
		     !(line[i]>='0' && line[i]<='9') )
		{
			switch (line[i])
			{
				
				default:
					line[i] = ' ';
					break;
			}
		}
	}
	return err;
}

int main()
{
	char line[1024];
	char *word, *trav, *processed_line;
	unsigned int len = 0, line_len = 0, mod_line_len = 0;
	int err = 0;


	memset(line, '\0', sizeof(line));
	snprintf(line, sizeof(line), "  \t a 234 53335 abcfd \n 342dsf n/342 k23afa %$ $  ds$afd/234 23 5 3443a    000 as 3asdf asf1 dsf0 \n \r \t");


	line_len = strlen(line);
	mod_line_len = line_len + 32;//Some extra space, includes extra space at the end + newline + teminating '\0' character.
	processed_line = (char *) malloc(mod_line_len);
	memset(processed_line, '\0', sizeof(mod_line_len));


	printf("\nLine is: \"%s\"\n\n", line);
	printf("\nLine length: %u\n\n", line_len);
	printf("\nWords read...\n");

	remove_non_chars(line);

	trav = line;

	while (sscanf(trav, "%ms", &word) != EOF)
	{
		len = strlen(word);
		printf("%s; sizeof(%s)=%d \n\n", word, word, len);

		trav = strstr(trav, word) +  len;

		//Check if word begins with an alphabet
		if ( (word[0]>='a' && word[0]<='z') || 
		     (word[0]>='A' && word[0]<='Z') )
		{
			//printf("%s\n", word);
			strncat(processed_line, word, len);
			//Add space after adding word. For last word, replace
			//trailing space with a newline
			strncat(processed_line, " ", 1);
			
		}

		free(word);
		if ( trav >= (line + line_len) )
		{
			break;
		}
	}
	//processed_line will have a space at the end. Replace it with a 
	//newline as this is line by line operation.
	len = strlen( processed_line );
	if ( len > mod_line_len )
	{
		fprintf(stderr, "%s:%d ERROR! BUFFER OVERFLOW!", __func__, __LINE__);
		goto exit1;

	}
	if ( len )
	{
		//len accounts for the last, trailing space.
		//Replacing it with a newline.
		processed_line[len - 1] = '\n';
	}
	//Copy the processed line into the old line
	//length of processed_line <= length of line
	strncpy(line, processed_line, line_len);
	
	printf("\n\nEnd of processing!\n");
	printf("\nFinally, processed line: \"%s\"\n", line);
exit1:

	free(processed_line);
	return 0;

}

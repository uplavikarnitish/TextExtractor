#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define RECORD_TAG "<RECORD ID="
#define E_RECORD_TAG "</RECORD>"
#define TEXT_TAG "<TEXT>"
#define E_TEXT_TAG "</TEXT>"
#define REPORT_E "report_end"

#define STEM_ALGO_BIN_F_PATH "./stem"

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
int process_line(char *line)
{
	char *word, *trav, *processed_line;
	unsigned int len = 0, line_len = 0, mod_line_len = 0;
	int err = 0;




	line_len = strlen(line);
	mod_line_len = line_len + 32;//Some extra space, includes extra space at the end + newline + teminating '\0' character.
	processed_line = (char *) malloc(mod_line_len);
	memset(processed_line, '\0', sizeof(mod_line_len));


	remove_non_chars(line);
	trav = line;

	while (sscanf(trav, "%ms", &word) != EOF)
	{
		len = strlen(word);
		//printf("%s; sizeof(%s)=%d \n\n", word, word, len);

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
		err = -ENOBUFS;
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
	
exit1:

	free(processed_line);
	return err;

}


char file_name_to_write[512];
char file_name_to_write_temp[512];
FILE *fp_write = NULL;
FILE *file_open_to_write(int id, char *dest_dir)
{
	
	snprintf(file_name_to_write_temp, sizeof(file_name_to_write_temp), "%s//%d_temp.txt", dest_dir, id);
	snprintf(file_name_to_write, sizeof(file_name_to_write_temp), "%s//%d.txt", dest_dir, id);
	if ( (fp_write = fopen(file_name_to_write_temp, "w"))==NULL )
	{
		fp_write = NULL;
		fprintf(stderr, "%s:%d:: Error creating file %s! ENOENT", __func__, __LINE__, file_name_to_write_temp);
		return NULL;
	}
	return fp_write;
}

int file_write(char *str)
{
	if (fp_write == NULL)
	{
		fprintf(stderr, "%s:%d:: ERROR! EBAD file descriptor!", __func__, __LINE__);
		return -EBADF;
	}
	if ( str == NULL )
	{
		fprintf(stderr, "%s:%d:: ERROR! str:NULL!", __func__, __LINE__);
		return -EFAULT;
	}

	return fprintf(fp_write, "%s", str);
}

int file_write_close()
{
	if (fp_write == NULL)
	{
		fprintf(stderr, "%s:%d:: ERROR! EBAD file descriptor!", __func__, __LINE__);
		return -EBADF;
	}

	return fclose(fp_write);
}


int main(int argc, char *args[])
{
	char* text_start, *text_end;
	int err = 0, id;
	FILE *fp = NULL;
	char *line = NULL, *rec_tg_st, *rec_tg_cl_inv;
	size_t size = 0, n;
	int record=0, text=0;
	char system_command[512];

	if (argc != 3)
	{
		
		fprintf(stderr, "%s:%d:: ERROR! arguments:%d;; Expected %s <filename> <destination_dir>\n", __func__, __LINE__, argc, args[0]);
		err = -1;
		goto exit;
	}

	if ( (fp = fopen( args[1], "r"))==NULL )
	{
		err = errno;
		fprintf(stderr, "%s:%d:: ERROR! File %s not found! err: %d\n", __func__, __LINE__, args[1], err);
		goto exit;
	}

	
	while(( n = getline(&line, &size, fp))!=-1)
	{
		//printf("%s", line);
		if ( (rec_tg_st = strstr(line, RECORD_TAG))!=NULL )
		{
			rec_tg_st += sizeof(RECORD_TAG);
			id = atoi(rec_tg_st);
			//printf("Found New Record! ID = %d!\n", id);
			record = 1;
			continue; //If text on this same line then it will be discarded. If you want to use it, then read word by word using scanf("%s", addr), instead of getline
		}
		
		if ( (rec_tg_st = strstr(line, TEXT_TAG))!=NULL )
		{
			rec_tg_st += sizeof(TEXT_TAG);
			text = 1;
			//Validate structure
			if ( text && (record == 0) )
			{
				fprintf(stderr, "%s:%d:: ERROR! %s should be inside %s tag\n", __func__, __LINE__, TEXT_TAG, RECORD_TAG);
			}
			if ( file_open_to_write(id, args[2]) == NULL )
			{
				fprintf(stderr, "%s:%d:: ERROR! Executing file_open_to_write()!\n", __func__, __LINE__);
				err = -3;
				goto exit;
				
			}
			continue; //If text on this same line then it will be discarded. If you want to use it, then read word by word using scanf("%s", addr), instead of getline
		}

		
		if ( (rec_tg_st = strstr(line, E_TEXT_TAG))!=NULL )
		{
			rec_tg_st += sizeof(E_TEXT_TAG);
			text = 0;
			if( file_write_close() )
			{
				err = errno;
				fprintf(stderr, "%s:%d:: ERROR! Closing file, errno:%d!\n", __func__, __LINE__, err);
				goto exit;
			}
			//Now convert the temporary file using the Porter Stemmer http://tartarus.org/martin/PorterStemmer/c_thread_safe.txt 
			//algorithm for all the words to appear in their stemmed form.
			//Create stemming command
			snprintf(system_command, sizeof(system_command), "%s %s > %s", STEM_ALGO_BIN_F_PATH, file_name_to_write_temp, file_name_to_write);
			if ( (err = system(system_command))!=0 )
			{
				fprintf(stderr, "%s:%d:: ERROR! Executing system command %s; errno:%d!\n", __func__, __LINE__, system_command, err);
				goto exit;
				
			}
			//Now remove the intermediate file
			snprintf(system_command, sizeof(system_command), "rm %s", file_name_to_write_temp);
			if ( (err = system(system_command))!=0 )
			{
				fprintf(stderr, "%s:%d:: ERROR! Executing system command %s; errno:%d!\n", __func__, __LINE__, system_command, err);
				goto exit;
				
			}
			//printf("==Text==for=====Record ID = %d completed!=============\n", id);
			continue; //If text on this same line then it will be discarded. If you want to use it, then read word by word using scanf("%s", addr), instead of getline
		}

		if ( (rec_tg_st = strstr(line, E_RECORD_TAG))!=NULL )
		{
			rec_tg_st += sizeof(E_RECORD_TAG);
			//printf("=============Record ID = %d completed!=============\n", id);
			record = 0;
			continue; //If text on this same line then it will be discarded. If you want to use it, then read word by word using scanf("%s", addr), instead of getline
		}

		if ( (rec_tg_st = strstr(line, REPORT_E))!=NULL )
		{
			rec_tg_st += sizeof(REPORT_E);
			continue; //Each document has this 'report_end' key word, hence, removing it
		}

		if (text && record)
		{
			//printf("%s", line);
			if ( (err = process_line(line)) != 0)
			{
				fprintf(stderr, "%s:%d:: Error processing line %s! errno:%d\n", __func__, __LINE__, line, err);
				goto exit;
				
			}
			if (file_write(line) < 0)
			{
				fprintf(stderr, "%s:%d:: Error writing to file %s!\n", __func__, __LINE__, file_name_to_write_temp);
				err = -5;
				goto exit;
			}
		}
		

		//Both lines necessary for getting allocated memory -- see "man strstr" specification for details. This prevents sgementation fault.
		free(line);
		size = 0;
		//Both lines necessary for getting allocated memory --
	}
	if (errno != 0)
	{
		fprintf(stderr, "%s:%d:: ERROR! errno:%d ", __func__, __LINE__, errno);
		err = errno;
		goto exit;
	}



	fclose(fp);
exit:
	return err;

}

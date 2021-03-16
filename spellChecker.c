/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Yi Chang Liao
 * Date: 03/01/2020
 */

#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Help function to calculate Levenshtein distance
 * https://en.wikipedia.org/wiki/Levenshtein_distance
 * @return The value of the distance
 */
int minValue(int x, int y, int z) // help function to get the min
{
    int val = x;
    if(y<val) val=y;
    if(z<val) val=z;

    return val;
}

int levenshtein(char *a, char *b)
{
    // get length of both string
    int len_a = strlen(a), len_b=strlen(b);

    // create matrix
    int m[len_a+1][len_b+1];

    // set first row and column
    for(int i=0; i<len_a+1; i++)
    {m[i][0]=i;}
    for(int j=0; j<len_b+1; j++)
    {m[0][j]=j;}

    // compute the distance
    int cost;
    for(int i=1; i<=len_a; i++)
    {
        for(int j=1; j<=len_b; j++)
        {
            if(a[i-1]==b[j-1]) cost=0;
            else cost=1;

            m[i][j] = minValue(m[i-1][j]+1,m[i][j-1]+1,m[i-1][j-1]+cost);
        }
    }

    return m[len_a][len_b];
}

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    char* word;
    while(word = nextWord(file))
    {
        hashMapPut(map, word, 0);
        free(word);
    }
    free(word);
}

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    inputBuffer[0] = '\0';

    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");

        scanf("%50[^\n]s", inputBuffer);

        // check if press ENTER at the beginning
        while(inputBuffer[0] == 0)
        {

            printf("Please don't press ENTER at the beginning\n"
                   "Enter a word or \"quit\" to quit: ");

            while((getchar()) != '\n'); // flush input buffer

            scanf("%50[^\n]s", inputBuffer);
        }

        printf("\n");

        // Implement the spell checker code here

        // lowercase for every character
        for(int i=0;inputBuffer[i];i++)
        {
            inputBuffer[i] = tolower(inputBuffer[i]);
        }

        if (strcmp(inputBuffer, "quit") == 0) // if enter quit
        {
            quit = 1;
        }
        else if(hashMapContainsKey(map, inputBuffer)) //spell correctly
        {
            printf("The inputted word %s is spelled correctly\n",inputBuffer);
            while((getchar()) != '\n'); // flush input buffer
            inputBuffer[0]='\0';
        }
        else // spell incorrectly
        {
            // use count sort for storing levenshtein value
            // create count table
            HashLink** countSort = malloc(map->size*sizeof(HashLink*));
            assert(countSort!=0);
            for (int i = 0; i < map->size; i++)
            {
                countSort[i] = NULL;
            }

            // store levenshtein value in count table
            for(int i=0; i<map->capacity; i++)
            {
                HashLink *cur = map->table[i];
                while(cur!=0)
                {
                    int idx = levenshtein(inputBuffer,cur->key);
                    HashLink* link = malloc(sizeof(HashLink));
                    link->key = malloc(sizeof(char) * (strlen(cur->key) + 1));
                    strcpy(link->key, cur->key);
                    link->value = idx;
                    link->next = countSort[idx];
                    countSort[idx] = link;
                    cur = cur->next;
                }
            }

            // store first 5 suggested words
            char *arr[5];
            int count = 0;
            for(int i=0; i<map->size; i++)
            {

                HashLink *cur = countSort[i];
                while(cur!=0)
                {
                    arr[count] = cur->key;
                    cur = cur->next;
                    count++;
                    if(count > 4)
                    {
                        i=map->capacity;
                        break;
                    }
                }
            }

            // Print suggested words
            printf("The inputted word %s is spelled incorrectly"
                   ", Did you mean %s, %s, %s, %s or %s?\n",
                   inputBuffer,
                   arr[0],
                   arr[1],
                   arr[2],
                   arr[3],
                   arr[4]);

            // Free count sort

            for(int i=0; i<map->size; i++)
            {
                HashLink *cur, *next;
                cur = countSort[i];
                while(cur != 0)
                {
                    next = cur->next;
                    free(cur->key);
                    free(cur);
                    cur = next;
                }
            }
            free(countSort);
            while((getchar()) != '\n'); // flush input buffer
            inputBuffer[0]='\0';

        }
    }

    hashMapDelete(map);
    return 0;
}

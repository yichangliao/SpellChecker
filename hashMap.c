/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Yi Chang Liao
 * Date: 03/01/2020
 */

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap* map)
{
    assert(map!=0);

    HashLink *cur, *next;

    for(int i=0; i<map->capacity; i++)
    {
        cur = map->table[i];

        while(cur != 0)
        {
            next = cur->next;
            hashLinkDelete(cur);
            cur = next;
        }
    }

    free(map->table);
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key  and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    assert(map != 0 && key != 0);

    int idx = HASH_FUNCTION(key)%map->capacity;
    if(idx<0) idx += map->capacity;

    HashLink *cur = map->table[idx];
    while(cur != 0)
    {
        if (strcmp(cur->key, key)==0)
        {
            return &cur->value;
        }

        cur = cur->next;
    }

    return NULL;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    assert(map!=0 && capacity>0);

    //Create new table
    HashLink **newTable = malloc(capacity*sizeof(HashLink*));
    assert(newTable != 0);

    for(int i=0; i<capacity; i++)
    {
        newTable[i] = NULL;
    }

    //Pointer to old table for copy purpose, then assign newTable, capacity & size to map.
    HashLink **oldTable = map->table;
    int oldCap = map->capacity;
    map->table = newTable;
    map->capacity = capacity;
    map->size = 0;

    for(int i=0; i<oldCap; i++)
    {
        HashLink *cur= oldTable[i];
        while(cur != 0)
        {
            HashLink *temp = cur;
            hashMapPut(map, cur->key, cur->value);
            cur = cur->next;
            hashLinkDelete(temp);
        }
    }

    free(oldTable);
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    assert(map!=0 && key!=0);

    int idx = HASH_FUNCTION(key)%map->capacity;
    if(idx<0) idx += map->capacity;

    // check whether the map has the key, if so, delete the key
    if(hashMapContainsKey(map, key)) hashMapRemove(map, key);

    // put the key
    map->table[idx] = hashLinkNew(key, value, map->table[idx]);
    map->size++;

    // check the load
    if(hashMapTableLoad(map)>MAX_TABLE_LOAD) resizeTable(map, 2*map->capacity);
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap* map, const char* key)
{
    assert(map!=0 && key!=0);

    int idx = HASH_FUNCTION(key)%map->capacity;
    if(idx<0) idx += map->capacity;

    HashLink *cur = map->table[idx], *prev = 0;

    // if map contain key
    if(hashMapContainsKey(map, key))
    {
        // get location of the key
        while(strcmp(cur->key, key))
        {
            prev = cur;
            cur = cur->next;
        }

        if(prev == 0) // found the key at the first link
        {
            map->table[idx] = cur->next;
            hashLinkDelete(cur);
        }
        else
        {
            prev->next = cur->next;
            hashLinkDelete(cur);
        }
        map->size--;
    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    assert(map!=0 && key!=0);

    int idx = HASH_FUNCTION(key)%map->capacity;
    if(idx<0) idx += map->capacity;

    HashLink *cur = map->table[idx];

    while(cur != 0)
    {
        if(strcmp(cur->key, key)==0)
        {
            return 1;
        }
        else
        {
            cur = cur->next;
        }
    }
    return 0;
}

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    assert(map!=0);
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    assert(map!=0);
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    assert(map!=0);

    int count=0;

    for(int i=0; i<map->capacity; i++)
    {
        if(map->table[i] == 0)
        {
            count++;
        }
    }
    return count;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    assert(map!=0);

    return (float)map->size/map->capacity;
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap* map)
{
    assert(map!=0);

    printf("\n");

    for(int i=0; i<map->capacity; i++)
    {
        printf("table[%d]: ",i);
        HashLink *cur = map->table[i];
        while(cur!=0)
        {
            printf("(key: %s, value: %d), ",cur->key, cur->value);
            cur = cur->next;
        }
        printf("\n");
    }
   
}

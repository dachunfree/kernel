/*
LRUCacheImpl.c
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"LRUCache.h"
#include"LRUCacheImpl.h"

typedef struct cacheEntryS
{
    char key;
    char data;

    struct cacheEntryS *hashListPrev;   /* 缓存哈希表指针，指向哈希链表的前一个元素 */
    struct cacheEntryS *hashListNext;   /* 缓存哈希表指针，指向哈希链表的后一个元素 */

    struct cacheEntryS *lruListPrev;    /* 缓存双向链表，指向双向链表的前一个元素 */
    struct cacheEntryS *lruListNext;    /* 缓存双向链表，指向双向链表的后一个元素 */
}cacheEntryS;

typedef struct LRUCacheS
{
    int cacheCapacity;
    cacheEntryS **hashMap;  //缓存的hash表

    cacheEntryS *lruListHead;   //缓存双向链表的表头
    cacheEntryS *lruListTail;   //缓存双向链表表位
    int lruListSize;            //缓存双向链表节点个数
}LRUCacheS;


static void freeList(LRUCacheS *cache);

/****************************************************
*LRU缓存及缓存单位相关接口及实现
*****************************************************/

//创建一个缓存单位
static cacheEntryS *newCacheEntry(char key, char data)
{
    cacheEntryS* entry = NULL;
    if (NULL == (entry = malloc(sizeof(*entry))))
    {
        perror("malloc");
        return NULL;
    }

    memset(entry, 0, sizeof(*entry));
    entry->key = key;
    entry->data = data;
    return entry;
}

//释放一个缓存单元
static void freeCacheEntry(cacheEntryS* entry)
{
    if (NULL == entry)
    {
        return ;
    }
    free(entry);
}

//创建一个LRU缓存
int LRUCacheCreate(int capacity, void **lruCache)
{
    LRUCacheS* cache = NULL;
    if (NULL == (cache = malloc(sizeof(*cache))))
    {
        perror("malloc");
        return -1;
    }

    memset(cache, 0, sizeof(*cache));
    cache->cacheCapacity = capacity;
    cache->hashMap = (cacheEntryS**)malloc(sizeof(cacheEntryS)*capacity);
    if (NULL == cache->hashMap)
    {
        free(cache);
        perror("malloc");
        return -1;
    }

    memset(cache->hashMap, 0, sizeof(cacheEntryS)*capacity);
    *lruCache = cache;
    return 0;
}

//释放一个LRU缓存
int LRUCacheDestroy(void *lruCache)
{
    LRUCacheS* cache = (LRUCacheS*)lruCache;
    if (NULL == cache)
    {
        return 0;
    }

    if (cache->hashMap)
    {
        free(cache->hashMap);
    }

    freeList(cache);
    free(cache);
    return 0;
}

/****************************************************
* 双向链表相关接口及实现
*****************************************************/

//从双向链表中删除指定节点
static void removeFromList(LRUCacheS *cache, cacheEntryS* entry)
{
    //链表为空
    if (cache->lruListSize == 0)
    {
        return;
    }

    if (entry == cache->lruListHead && entry == cache->lruListTail)
    {
        //当链表仅剩当前一个节点
        cache->lruListTail = cache->lruListHead = NULL;
    }
    else if (entry == cache->lruListHead)
    {
        //删除节点位于表头
        cache->lruListHead = entry->lruListNext;
        cache->lruListHead->lruListPrev = NULL;
    }
    else if (entry == cache->lruListTail)
    {
        //删除节点位于表尾
        cache->lruListTail = entry->lruListPrev;
        cache->lruListTail->lruListNext = NULL;
    }
    else
    {
        //非头非为情况，直接摘抄节点
        entry->lruListPrev->lruListNext = entry->lruListNext;
        entry->lruListNext->lruListPrev = entry->lruListPrev;
    }

    //
    cache->lruListSize--;
}

//见节点插入链表表头
static cacheEntryS *insertToListHead(LRUCacheS *cache, cacheEntryS* entry)
{
    cacheEntryS *removeEntry = NULL;
    if (++cache->lruListSize > cache->cacheCapacity)
    {
        /* 如果缓存满了，即链表当前节点数已等于缓存容量，那么需要先删除链表表尾节点，即淘汰最久没有被访问到的缓存数据单元*/
        removeEntry = cache->lruListTail;
        removeFromList(cache, cache->lruListTail);
    }

    if (cache->lruListHead == NULL && cache->lruListTail == NULL)
    {
        //如果当前俩目标为空链表
        cache->lruListHead = cache->lruListTail = entry;
    }
    else
    {
        //当前链表非空，插入表头
        entry->lruListNext = cache->lruListHead;
        entry->lruListPrev = NULL;
        cache->lruListHead->lruListPrev = entry;
        cache->lruListHead = entry;
    }
    return removeEntry;
}

//释放整个链表
static void freeList(LRUCacheS *cache)
{
    //链表为空
    if (0 == cache->lruListSize)
    {
        return;
    }
    cacheEntryS* entry = cache->lruListHead;
    while(entry)
    {
        cacheEntryS *temp = entry->lruListNext;
        freeCacheEntry(entry);
        entry = temp;
    }

    cache->lruListSize = 0;
}


//辅助性接口，用于保证最近访问的节点总是位于链表表头
static void updateLRUList(LRUCacheS *cache, cacheEntryS *entry)
{
    //摘抄节点
    removeFromList(cache, entry);
    //将节点插入到链表表头
    insertToListHead(cache, entry);
}

/****************************************************
* hash表相关接口及实现
*****************************************************/

//哈希函数
static int hashKey(LRUCacheS *cache, char key)
{
    return (int)key%cache->cacheCapacity;
}

//从哈希表中获取缓存单元
static cacheEntryS *getValueFromHashMap(LRUCacheS *cache, char key)
{
    //使用函数定位数据存放在哪个槽中
    cacheEntryS *entry = cache->hashMap[hashKey(cache,key)];

    //遍历槽内链表，找到准确的数据项
    while(entry)
    {
        if (entry->key == key)
        {
            break;
        }
        entry = entry->hashListNext;
    }

    return entry;
}


//向hash表中插入缓存单元
static void insertentryToHashMap(LRUCacheS *cache, cacheEntryS *entry)
{
    //使用函数定位数据存放在哪个槽中
    cacheEntryS *n = cache->hashMap[hashKey(cache, entry->key)];
    if (n != NULL)
    {
        //如果槽内已经有其他数据项，将巢内数据项与当前预加入数据项组成链表
        //当前预加入数据项为表头
        entry->hashListNext = n;
        n->hashListPrev = entry;
    }

    //将数据项放到到哈希槽内
    cache->hashMap[hashKey(cache, entry->key)] = entry;
}


//从哈希表中删除缓存单元
static void removeEntryFromHashMap(LRUCacheS *cache, cacheEntryS *entry)
{
    //无需做任何删除操作的情况
    if (NULL == entry || NULL == cache || NULL == cache->hashMap)
    {
        return ;
    }

    //定位数据位于哪个槽内
    cacheEntryS *n = cache->hashMap[hashKey(cache, entry->key)];
    //遍历槽内链表，找到与删除节点，将节点从哈希表摘除
    while(n)
    {
        //找到预删除节点，将节点从hash表摘除
        if (n->key == entry->key)
        {
            if (n->hashListPrev)
            {
                n->hashListPrev->hashListNext = n->hashListNext;
            }
            else
            {
                cache->hashMap[hashKey(cache, entry->key)] = n->hashListNext;
            }

            if (n->hashListNext)
            {
                n->hashListNext->hashListPrev = n->hashListPrev;
            }
        }

        n = n->hashListNext;
    }
}


/****************************************************
* hash表相关接口及实现
*****************************************************/

//将数据放入LRU缓存中
int LRUCacheSet(void *lrucache, char key, char data)
{
    LRUCacheS *cache = (LRUCacheS *)lrucache;
    //从hash表查找数据是否已经在缓存中
    cacheEntryS* entry = getValueFromHashMap(cache, key);
    if (NULL != entry)
    {
        //更新数据，将数据项更新至链表表头
        entry->data = data;
        updateLRUList(cache, entry);
    }
    else    //数据没在缓存中，新建缓存插入链表表头
    {
        entry = newCacheEntry(key, data);
        cacheEntryS *removeEntry = insertToListHead(cache, entry);
        if (NULL != removeEntry)
        {
            //缓存满，淘汰最近最久没有被访问到的数据单元
            removeEntryFromHashMap(cache, removeEntry);
            freeCacheEntry(removeEntry);
        }
        insertentryToHashMap(cache, entry);
    }
    return 0;
}

char LRUCacheGet(void *lruCache, char key)
{
    LRUCacheS *cache = (LRUCacheS *)lruCache;
    cacheEntryS *entry = getValueFromHashMap(cache, key);
    //检查hash缓存是否已经存在数据
    if (NULL != entry)
    {
        //缓存中存在数据，更新至表头
        updateLRUList(cache, entry);
        return entry->data;
    }
    else
    {
        return '\0';
    }

}

void LRUCachePrint(void *lruCache)
{
    LRUCacheS *cache = (LRUCacheS *)lruCache;
    if (NULL == cache || 0 == cache->lruListSize)
    {
        return;
    }

    fprintf(stdout, "\n>>>>>>>>>>>>>>\n");
    fprintf(stdout, "cache (key data):\n");
    cacheEntryS *entry = cache->lruListHead;

    while(entry)
    {
        fprintf(stdout, "(%c, %c) ", entry->key, entry->data);
        entry = entry->lruListNext;
    }
    fprintf(stdout, "\n<<<<<<<<<<<<<<<<<<\n\n");
}




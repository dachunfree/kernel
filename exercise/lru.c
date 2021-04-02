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

    struct cacheEntryS *hashListPrev;   /* �����ϣ��ָ�룬ָ���ϣ�����ǰһ��Ԫ�� */
    struct cacheEntryS *hashListNext;   /* �����ϣ��ָ�룬ָ���ϣ����ĺ�һ��Ԫ�� */

    struct cacheEntryS *lruListPrev;    /* ����˫������ָ��˫�������ǰһ��Ԫ�� */
    struct cacheEntryS *lruListNext;    /* ����˫������ָ��˫������ĺ�һ��Ԫ�� */
}cacheEntryS;

typedef struct LRUCacheS
{
    int cacheCapacity;
    cacheEntryS **hashMap;  //�����hash��

    cacheEntryS *lruListHead;   //����˫������ı�ͷ
    cacheEntryS *lruListTail;   //����˫�������λ
    int lruListSize;            //����˫������ڵ����
}LRUCacheS;


static void freeList(LRUCacheS *cache);

/****************************************************
*LRU���漰���浥λ��ؽӿڼ�ʵ��
*****************************************************/

//����һ�����浥λ
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

//�ͷ�һ�����浥Ԫ
static void freeCacheEntry(cacheEntryS* entry)
{
    if (NULL == entry)
    {
        return ;
    }
    free(entry);
}

//����һ��LRU����
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

//�ͷ�һ��LRU����
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
* ˫��������ؽӿڼ�ʵ��
*****************************************************/

//��˫��������ɾ��ָ���ڵ�
static void removeFromList(LRUCacheS *cache, cacheEntryS* entry)
{
    //����Ϊ��
    if (cache->lruListSize == 0)
    {
        return;
    }

    if (entry == cache->lruListHead && entry == cache->lruListTail)
    {
        //�������ʣ��ǰһ���ڵ�
        cache->lruListTail = cache->lruListHead = NULL;
    }
    else if (entry == cache->lruListHead)
    {
        //ɾ���ڵ�λ�ڱ�ͷ
        cache->lruListHead = entry->lruListNext;
        cache->lruListHead->lruListPrev = NULL;
    }
    else if (entry == cache->lruListTail)
    {
        //ɾ���ڵ�λ�ڱ�β
        cache->lruListTail = entry->lruListPrev;
        cache->lruListTail->lruListNext = NULL;
    }
    else
    {
        //��ͷ��Ϊ�����ֱ��ժ���ڵ�
        entry->lruListPrev->lruListNext = entry->lruListNext;
        entry->lruListNext->lruListPrev = entry->lruListPrev;
    }

    //
    cache->lruListSize--;
}

//���ڵ���������ͷ
static cacheEntryS *insertToListHead(LRUCacheS *cache, cacheEntryS* entry)
{
    cacheEntryS *removeEntry = NULL;
    if (++cache->lruListSize > cache->cacheCapacity)
    {
        /* ����������ˣ�������ǰ�ڵ����ѵ��ڻ�����������ô��Ҫ��ɾ�������β�ڵ㣬����̭���û�б����ʵ��Ļ������ݵ�Ԫ*/
        removeEntry = cache->lruListTail;
        removeFromList(cache, cache->lruListTail);
    }

    if (cache->lruListHead == NULL && cache->lruListTail == NULL)
    {
        //�����ǰ��Ŀ��Ϊ������
        cache->lruListHead = cache->lruListTail = entry;
    }
    else
    {
        //��ǰ����ǿգ������ͷ
        entry->lruListNext = cache->lruListHead;
        entry->lruListPrev = NULL;
        cache->lruListHead->lruListPrev = entry;
        cache->lruListHead = entry;
    }
    return removeEntry;
}

//�ͷ���������
static void freeList(LRUCacheS *cache)
{
    //����Ϊ��
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


//�����Խӿڣ����ڱ�֤������ʵĽڵ�����λ�������ͷ
static void updateLRUList(LRUCacheS *cache, cacheEntryS *entry)
{
    //ժ���ڵ�
    removeFromList(cache, entry);
    //���ڵ���뵽�����ͷ
    insertToListHead(cache, entry);
}

/****************************************************
* hash����ؽӿڼ�ʵ��
*****************************************************/

//��ϣ����
static int hashKey(LRUCacheS *cache, char key)
{
    return (int)key%cache->cacheCapacity;
}

//�ӹ�ϣ���л�ȡ���浥Ԫ
static cacheEntryS *getValueFromHashMap(LRUCacheS *cache, char key)
{
    //ʹ�ú�����λ���ݴ�����ĸ�����
    cacheEntryS *entry = cache->hashMap[hashKey(cache,key)];

    //�������������ҵ�׼ȷ��������
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


//��hash���в��뻺�浥Ԫ
static void insertentryToHashMap(LRUCacheS *cache, cacheEntryS *entry)
{
    //ʹ�ú�����λ���ݴ�����ĸ�����
    cacheEntryS *n = cache->hashMap[hashKey(cache, entry->key)];
    if (n != NULL)
    {
        //��������Ѿ�������������������������뵱ǰԤ�����������������
        //��ǰԤ����������Ϊ��ͷ
        entry->hashListNext = n;
        n->hashListPrev = entry;
    }

    //��������ŵ�����ϣ����
    cache->hashMap[hashKey(cache, entry->key)] = entry;
}


//�ӹ�ϣ����ɾ�����浥Ԫ
static void removeEntryFromHashMap(LRUCacheS *cache, cacheEntryS *entry)
{
    //�������κ�ɾ�����������
    if (NULL == entry || NULL == cache || NULL == cache->hashMap)
    {
        return ;
    }

    //��λ����λ���ĸ�����
    cacheEntryS *n = cache->hashMap[hashKey(cache, entry->key)];
    //�������������ҵ���ɾ���ڵ㣬���ڵ�ӹ�ϣ��ժ��
    while(n)
    {
        //�ҵ�Ԥɾ���ڵ㣬���ڵ��hash��ժ��
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
* hash����ؽӿڼ�ʵ��
*****************************************************/

//�����ݷ���LRU������
int LRUCacheSet(void *lrucache, char key, char data)
{
    LRUCacheS *cache = (LRUCacheS *)lrucache;
    //��hash����������Ƿ��Ѿ��ڻ�����
    cacheEntryS* entry = getValueFromHashMap(cache, key);
    if (NULL != entry)
    {
        //�������ݣ�������������������ͷ
        entry->data = data;
        updateLRUList(cache, entry);
    }
    else    //����û�ڻ����У��½�������������ͷ
    {
        entry = newCacheEntry(key, data);
        cacheEntryS *removeEntry = insertToListHead(cache, entry);
        if (NULL != removeEntry)
        {
            //����������̭������û�б����ʵ������ݵ�Ԫ
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
    //���hash�����Ƿ��Ѿ���������
    if (NULL != entry)
    {
        //�����д������ݣ���������ͷ
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




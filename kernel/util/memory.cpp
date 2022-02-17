#include "debug.h"
#include "memory.h"

constexpr uint32_t MEMNODE_COUNT = 4096;
constexpr uint32_t TAIL_MAGIC = 0x317839FE;

struct MemNode {
    uint32_t addr, size;
    MemNode *next;
} __attribute__((packed));

static MemNode nodes[MEMNODE_COUNT];
static MemNode *pUse, *pFree;

static MemNode *get_free_node() {
    MemNode *r = pFree;
    pFree = pFree->next;
    return r;
}

static void push_free_node(MemNode *node) {
    node->next = pFree;
    pFree = node;
}

static void insert(uint32_t addr, uint32_t size) {
    if (pUse == 0) { // no node
        pUse = get_free_node();
        pUse->next = 0;
        pUse->addr = addr;
        pUse->size = size;
        return;
    }
    if (addr < pUse->addr) { // before first node
        if (addr + size == pUse->addr) {
            pUse->addr = addr;
            pUse->size += size;
        } else {
            MemNode *p = get_free_node();
            p->next = pUse;
            p->addr = addr;
            p->size = size;
            pUse = p;
        }
        return;
    }
    if (pUse->next == 0) { // only 1 node
        if (pUse->addr + pUse->size == addr) {
            pUse->size += size;
        } else {
            MemNode *p = get_free_node();
            p->next = 0;
            p->addr = addr;
            p->size = size;
            pUse->next = p;
        }
        return;
    }
    MemNode *pre = pUse, *cur = pUse->next;
    while (cur != 0) {
        if (pre->addr < addr && addr < cur->addr) {
            if (pre->addr + pre->size == addr) {
                if (addr + size == cur->addr) {
                    pre->size += size + cur->size;
                    pre->next = cur->next;
                    push_free_node(cur);
                } else {
                    pre->size += size;
                }
                return;
            }
            if (addr + size == cur->addr) {
                cur->addr = addr;
                cur->size += size;
                return;
            }
            MemNode *n = get_free_node();
            pre->next = n;
            n->next = cur;
            n->addr = addr;
            n->size = size;
            return;
        }
        pre = cur;
        cur = cur->next;
    }
    if (pre->addr + pre->size == addr) {
        pre->size += size;
    } else {
        MemNode *n = get_free_node();
        pre->next = n;
        n->next = 0;
        n->addr = addr;
        n->size = size;
    }
}

static uint32_t find_alloc(uint32_t size) {
    if (pUse->next == 0) {
        if (pUse->size < size) {
            debug() << "ERR: cannot alloc memory!\n";
            return 0;
        }
        uint32_t ret = pUse->addr;
        pUse->addr += size;
        pUse->size -= size;
        return ret;
    }
    MemNode **pnext = &pUse, *node = pUse;
    MemNode *find = 0;
    uint32_t fsize = 0xFFFFFFFF;
    while (node != 0) {
        if (node->size == size) {
            uint32_t ret = node->addr;
            *pnext = node->next;
            push_free_node(node);
            return ret;
        } else if (node->size > size && node->size < fsize) {
            find = node;
            fsize = node->size;
        }
    }
    if (find == 0) {
        debug() << "ERR: cannot alloc memory!\n";
        return 0;
    }
    uint32_t ret = find->addr;
    find->addr += size;
    return ret;
}

void Memory::init() {
    pFree = nodes;
    pUse = 0;
    for (int i = 1; i < MEMNODE_COUNT; i++) {
        nodes[i - 1].next = nodes + i;
    }
}

void Memory::add(uint32_t start, uint32_t size) {
    size &= ~3;
    start = (start + 3) & ~3;
    insert(start, size);
}

void *Memory::alloc(uint32_t size) {
    uint32_t rsize = (size + 8 + 3) & (~3);
    uint32_t addr = find_alloc(rsize);
    uint32_t *ptr = (uint32_t *)addr;
    *ptr = rsize;
    ptr[rsize / 4 - 1] = TAIL_MAGIC ^ rsize;
    return ptr + 1;
}

void Memory::free(void *ptr) {
    uint32_t *p = (uint32_t *)ptr;
    --p;
    uint32_t size = *p;
    if ((p[size / 4 - 1] ^ size) != TAIL_MAGIC) {
        debug() << "ERR: free memory tail check failed!";
        return;
    }
    insert((uint32_t)p, size);
}

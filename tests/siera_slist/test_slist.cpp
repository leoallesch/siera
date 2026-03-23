extern "C" {
#include "siera/slist.h"
}
#include <CppUTest/TestHarness.h>

struct Item { int value; siera_slist_node_t node; };

TEST_GROUP(SieraSlist) {};

TEST(SieraSlist, PushPop) {
    siera_slist_head_t list = SIERA_SLIST_HEAD_INIT;
    Item a{1}, b{2};
    siera_slist_node_init(&a.node);
    siera_slist_node_init(&b.node);
    siera_slist_push(&list, &a.node);
    siera_slist_push(&list, &b.node);
    CHECK_EQUAL(2, SIERA_CONTAINER_OF(siera_slist_pop(&list), Item, node)->value);
    CHECK_EQUAL(1, SIERA_CONTAINER_OF(siera_slist_pop(&list), Item, node)->value);
    CHECK_TRUE(siera_slist_is_empty(&list));
}

TEST(SieraSlist, RemoveMiddle) {
    siera_slist_head_t list = SIERA_SLIST_HEAD_INIT;
    Item a{1}, b{2}, c{3};
    siera_slist_node_init(&a.node);
    siera_slist_node_init(&b.node);
    siera_slist_node_init(&c.node);
    siera_slist_push(&list, &a.node);
    siera_slist_push(&list, &b.node);
    siera_slist_push(&list, &c.node);
    CHECK_TRUE(siera_slist_remove(&list, &b.node));
    CHECK_FALSE(siera_slist_remove(&list, &b.node));
}

TEST(SieraSlist, SafeIterationRemoval) {
    siera_slist_head_t list = SIERA_SLIST_HEAD_INIT;
    Item items[5];
    for (int i = 0; i < 5; i++) {
        items[i].value = i;
        siera_slist_node_init(&items[i].node);
        siera_slist_push(&list, &items[i].node);
    }
    // Remove even values (0, 2, 4) — leaves odd values (1, 3) = 2 items
    siera_slist_node_t *cur, *tmp;
    SIERA_SLIST_FOR_EACH_SAFE(&list, cur, tmp) {
        if (SIERA_CONTAINER_OF(cur, Item, node)->value % 2 == 0)
            siera_slist_remove(&list, cur);
    }
    int count = 0;
    SIERA_SLIST_FOR_EACH_SAFE(&list, cur, tmp) { count++; }
    CHECK_EQUAL(2, count);
}

TEST(SieraSlist, EmptyList) {
    siera_slist_head_t list = SIERA_SLIST_HEAD_INIT;
    CHECK_TRUE(siera_slist_is_empty(&list));
    CHECK(siera_slist_pop(&list) == nullptr);
}

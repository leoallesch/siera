#include "CppUTest/TestHarness.h"

extern "C" {
#include "list.h"
}

TEST_GROUP(ListTests)
{
  list_t list;

  void setup()
  {
    list_init(&list);
  }

  void teardown()
  {
  }
};

TEST(ListTests, InitializesEmpty)
{
  CHECK(list.head == NULL);
}

TEST(ListTests, AddFrontSingleElement)
{
  list_node_t node;
  list_add_front(&list, &node);

  CHECK(list.head == &node);
  CHECK(node.next == NULL);
}

TEST(ListTests, AddFrontMultipleElements)
{
  list_node_t node1, node2, node3;

  list_add_front(&list, &node1);
  list_add_front(&list, &node2);
  list_add_front(&list, &node3);

  CHECK(list.head == &node3);
  CHECK(node3.next == &node2);
  CHECK(node2.next == &node1);
  CHECK(node1.next == NULL);
}

TEST(ListTests, PushToEmptyList)
{
  list_node_t node;

  list_push(&list, &node);

  CHECK(list.head == &node);
  CHECK(node.next == NULL);
}

TEST(ListTests, PushToNonEmptyList)
{
  list_node_t node1, node2, node3;

  list_add_front(&list, &node1);
  list_push(&list, &node2);
  list_push(&list, &node3);

  CHECK(list.head == &node1);
  CHECK(node1.next == &node2);
  CHECK(node2.next == &node3);
  CHECK(node3.next == NULL);
}

TEST(ListTests, DeleteHead)
{
  list_node_t node1, node2;

  list_add_front(&list, &node1);
  list_add_front(&list, &node2);

  list_delete(&list, &node2);

  CHECK(list.head == &node1);
  CHECK(node1.next == NULL);
}

TEST(ListTests, DeleteMiddle)
{
  list_node_t node1, node2, node3;

  list_add_front(&list, &node1);
  list_add_front(&list, &node2);
  list_add_front(&list, &node3);

  list_delete(&list, &node2);

  CHECK(list.head == &node3);
  CHECK(node3.next == &node1);
  CHECK(node1.next == NULL);
}

TEST(ListTests, DeleteTail)
{
  list_node_t node1, node2, node3;

  list_add_front(&list, &node1);
  list_add_front(&list, &node2);
  list_add_front(&list, &node3);

  list_delete(&list, &node1);

  CHECK(list.head == &node3);
  CHECK(node3.next == &node2);
  CHECK(node2.next == NULL);
}

TEST(ListTests, DeleteOnlyElement)
{
  list_node_t node;

  list_add_front(&list, &node);
  list_delete(&list, &node);

  CHECK(list.head == NULL);
}

TEST(ListTests, DeleteNonExistentDoesNothing)
{
  list_node_t node1, node2;

  list_add_front(&list, &node1);
  list_delete(&list, &node2);

  CHECK(list.head == &node1);
}

TEST(ListTests, DeleteFromEmptyListDoesNothing)
{
  list_node_t node;
  list_delete(&list, &node);
  CHECK(list.head == NULL);
}

TEST(ListTests, IsEmptyReturnsTrueWhenEmpty)
{
  CHECK_TRUE(list_is_empty(&list));
}

TEST(ListTests, IsEmptyReturnsFalseWhenNotEmpty)
{
  list_node_t node;
  list_add_front(&list, &node);
  CHECK_FALSE(list_is_empty(&list));
}

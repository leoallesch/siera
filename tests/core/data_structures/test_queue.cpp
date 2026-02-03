#include "CppUTest/TestHarness.h"

extern "C" {
#include "queue.h"
}

TEST_GROUP(QueueTests)
{
  queue_t queue;

  void setup()
  {
    queue_init(&queue);
  }

  void teardown()
  {
  }
};

TEST(QueueTests, InitializesEmpty)
{
  CHECK(queue_is_empty(&queue));
  CHECK_EQUAL(0, queue_size(&queue));
  CHECK(queue_peek(&queue) == NULL);
}

TEST(QueueTests, EnqueueSingleElement)
{
  queue_node_t node;
  queue_enqueue(&queue, &node);

  CHECK_FALSE(queue_is_empty(&queue));
  CHECK_EQUAL(1, queue_size(&queue));
  CHECK(queue_peek(&queue) == &node);
}

TEST(QueueTests, EnqueueMultipleElements)
{
  queue_node_t node1, node2, node3;

  queue_enqueue(&queue, &node1);
  queue_enqueue(&queue, &node2);
  queue_enqueue(&queue, &node3);

  CHECK_EQUAL(3, queue_size(&queue));
  CHECK(queue_peek(&queue) == &node1);
}

TEST(QueueTests, DequeueSingleElement)
{
  queue_node_t node;
  queue_enqueue(&queue, &node);

  queue_node_t* dequeued = queue_dequeue(&queue);

  CHECK(dequeued == &node);
  CHECK(queue_is_empty(&queue));
  CHECK_EQUAL(0, queue_size(&queue));
}

TEST(QueueTests, DequeueFIFOOrder)
{
  queue_node_t node1, node2, node3;

  queue_enqueue(&queue, &node1);
  queue_enqueue(&queue, &node2);
  queue_enqueue(&queue, &node3);

  CHECK(queue_dequeue(&queue) == &node1);
  CHECK(queue_dequeue(&queue) == &node2);
  CHECK(queue_dequeue(&queue) == &node3);
  CHECK(queue_is_empty(&queue));
}

TEST(QueueTests, DequeueFromEmptyReturnsNull)
{
  queue_node_t* dequeued = queue_dequeue(&queue);

  CHECK(dequeued == NULL);
  CHECK(queue_is_empty(&queue));
}

TEST(QueueTests, PeekDoesNotRemoveElement)
{
  queue_node_t node;
  queue_enqueue(&queue, &node);

  queue_node_t* peeked1 = queue_peek(&queue);
  queue_node_t* peeked2 = queue_peek(&queue);

  CHECK(peeked1 == &node);
  CHECK(peeked2 == &node);
  CHECK_EQUAL(1, queue_size(&queue));
}

TEST(QueueTests, PeekEmptyReturnsNull)
{
  CHECK(queue_peek(&queue) == NULL);
}

TEST(QueueTests, EnqueueAfterDequeue)
{
  queue_node_t node1, node2;

  queue_enqueue(&queue, &node1);
  queue_dequeue(&queue);
  queue_enqueue(&queue, &node2);

  CHECK_EQUAL(1, queue_size(&queue));
  CHECK(queue_peek(&queue) == &node2);
}

TEST(QueueTests, SizeTracksCorrectly)
{
  queue_node_t node1, node2, node3;

  CHECK_EQUAL(0, queue_size(&queue));

  queue_enqueue(&queue, &node1);
  CHECK_EQUAL(1, queue_size(&queue));

  queue_enqueue(&queue, &node2);
  CHECK_EQUAL(2, queue_size(&queue));

  queue_enqueue(&queue, &node3);
  CHECK_EQUAL(3, queue_size(&queue));

  queue_dequeue(&queue);
  CHECK_EQUAL(2, queue_size(&queue));

  queue_dequeue(&queue);
  CHECK_EQUAL(1, queue_size(&queue));

  queue_dequeue(&queue);
  CHECK_EQUAL(0, queue_size(&queue));
}

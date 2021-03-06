#ifndef RC_SRC_HEAP_HPP
#define RC_SRC_HEAP_HPP

#include "types.hpp"
#include <config.h>

namespace runtime
{

// Element in the free list.
struct Chunk
{
  Chunk* next;
  size_t size;
};

struct Block
{
  ~Block ();

  static Block* make (size_t size);
  static void insert (Block** ptr, Block* block);
  static Block* find (Block* block, void* address);
  static void scan_worklist (Block* work_list, Heap* heap);
  static size_t sweep (Block** b, Chunk** head);
  static void merge (Block** root, Block* block);

  void allocate (void* address, size_t size);
  void mark (void* address, Block** work_list);
  void* begin () const;
  bool is_object (void* address) const;
  bool is_allocated (void* address) const;
  void set_mark ()
  {
    marked_ = true;
  }

#ifndef COVERAGE
  void dump () const;
#endif

private:
  Block (size_t bits_bytes);
  static Block* remove_right_most (Block** b);
  size_t slot (void* address) const;
  void set_bits (size_t slot, unsigned char mask);
  void reset_bits (size_t slot, unsigned char mask);
  void clear_bits (size_t slot);
  unsigned char get_bits (size_t slot) const;
  void scan (Heap* heap, Block** work_list);

  // Blocks are organized into a tree sorted by begin.
  Block* left_;
  Block* right_;
  // Blocks are also organized into a set/list when collecting garbage.
  Block* next_;
  // Beginning and end of the storage for this block.
  void* begin_;
  void* end_;
  // Indicates that at least one slot is marked.
  bool marked_;
  // Status bits.
  unsigned char bits_[];
};

struct Heap
{
  // Constructor for external root of the given size.
  // Used for statically allocated components.
  Heap (void* begin, size_t size);
  // Constructor for internal root of the given size.
  // Used for dynamically allocated heaps.
  Heap (size_t size_of_root);
  ~Heap ();
  // Return the root of the heap.
  void* root () const;
  // Allocate size bytes.
  void* allocate (size_t size);
  bool collect_garbage (bool force = false);
  // Merge x into this heap.
  void merge (Heap* x);
  void insert_child (Heap* child);
  void remove_from_parent ();
#ifndef COVERAGE
  void dump ();
#endif
  bool contains (void* ptr);
  bool is_object (void* ptr);
  bool is_allocated (void* ptr);
  bool is_child (Heap* child);

private:
  // The root block of this heap.
  Block* block_;

  // Lock for this heap.
  pthread_mutex_t mutex_;

  // List of free chunks.
  Chunk* free_list_head_;

  // Number of bytes allocated in this heap.
  size_t allocated_size_;
  // Size of the next block to be allocated (bytes).
  size_t next_block_size_;
  // Size when next collection will be triggered (bytes).
  size_t next_collection_size_;

  // The beginning and end of the root of the heap.  For a statically
  // allocated component, they refer to a chunk in the parent
  // component.  For a child heap, they refer to a chunk in the
  // heap.
  char* begin_;
  char* end_;

  // The pointers for a tree of heaps.
  Heap* child_;
  Heap* next_;
  Heap* parent_;
  // Flag indicating heap was reachable during garbage collection.
  bool reachable_;

  void scan (void* begin, void* end, Block** work_list);
  void mark_slot_for_address (void* p, Block** work_list);

#ifndef COVERAGE
  void dump_i () const;
#endif
  friend class Block;
};

}

#endif // RC_SRC_HEAP_HPP

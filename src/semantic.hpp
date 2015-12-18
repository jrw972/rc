#ifndef RC_SRC_SEMANTIC_HPP
#define RC_SRC_SEMANTIC_HPP

#include <error.h>

#include <utility>

#include "types.hpp"
#include "ast.hpp"
#include "symbol_visitor.hpp"

namespace semantic
{

using namespace std::rel_ops;

struct Multiplier
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x * y;
  }
};

struct Divider
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x / y;
  }
};

struct Modulizer
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x % y;
  }
};

struct LeftShifter
{
  template <typename T, typename U>
  T operator() (const T& x, const U& y) const
  {
    return x << y;
  }
};

struct RightShifter
{
  template <typename T, typename U>
  T operator() (const T& x, const U& y) const
  {
    return x >> y;
  }
};

struct BitAnder
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x & y;
  }
};

struct BitAndNotter
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x & (~y);
  }
};

struct Adder
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x + y;
  }
};

struct Subtracter
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x - y;
  }
};

struct BitOrer
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x | y;
  }
};

struct BitXorer
{
  template <typename T>
  T operator() (const T& x, const T& y) const
  {
    return x ^ y;
  }
};

struct Equalizer
{
  template <typename T>
  bool operator() (const T& x, const T& y) const
  {
    return x == y;
  }
};

struct NotEqualizer
{
  template <typename T>
  bool operator() (const T& x, const T& y) const
  {
    return x != y;
  }
};

struct LessThaner
{
  template <typename T>
  bool operator() (const T& x, const T& y) const
  {
    return x < y;
  }
};

struct LessEqualizer
{
  template <typename T>
  bool operator() (const T& x, const T& y) const
  {
    return x <= y;
  }
};

struct MoreThaner
{
  template <typename T>
  bool operator() (const T& x, const T& y) const
  {
    return x > y;
  }
};

struct MoreEqualizer
{
  template <typename T>
  bool operator() (const T& x, const T& y) const
  {
    return x >= y;
  }
};

template <typename T>
struct LogicNotter
{
  typedef T ValueType;
  bool operator() (const T& x) const
  {
    return !x;
  }
};

template <typename T>
struct Negater
{
  typedef T ValueType;
  T operator() (const T& x) const
  {
    return -x;
  }
};

/* Enter all symbols except vars and parameters. */
void enter_symbols (ast::Node* node);

// Enter a symbol.
template <typename T>
T*
enter_symbol (ast::Node& node, T* symbol)
{
  // Check if the symbol is defined locally.
  const std::string& identifier = symbol->identifier;
  decl::Symbol *s = node.FindLocalSymbol (identifier);
  if (s == NULL)
    {
      node.EnterSymbol (symbol);
    }
  else
    {
      const ast::Node* node = symbol->definingNode;
      error_at_line (-1, 0, node->location.File.c_str (), node->location.Line,
                     "%s is already defined in this scope (E113)", identifier.c_str ());
    }
  return symbol;
}

// Enter a signature.
void enter_signature (ast::Node& node, const type::Signature * type);

// Look up a symbol.  If it is not defined, process its definition.
template<typename T>
T* processAndLookup (ast::Node * node, const std::string& identifier)
{
  decl::Symbol *symbol = node->FindGlobalSymbol (identifier);
  if (symbol == NULL)
    {
      error_at_line (-1, 0, node->location.File.c_str (), node->location.Line,
                     "%s was not declared in this scope (E114)", identifier.c_str ());
    }
  if (symbol->inProgress)
    {
      error_at_line (-1, 0, node->location.File.c_str (), node->location.Line,
                     "%s is defined recursively (E115)", identifier.c_str ());
    }
  if (!symbol->defined ())
    {
      // Process the definition.
      UNIMPLEMENTED;
    }

  return decl::SymbolCast<T> (symbol);
}

// Extract an array dimension or error.
type::Int::ValueType process_array_dimension (ast::Node* ptr);

// Check that a signature has +foreign where needed.
void CheckForForeignSafe (const type::Signature* signature, const decl::ParameterSymbol* return_parameter);

// Process a type specification.
const type::Type * process_type (ast::Node* node, bool force);

// Type check the expression, insert an implicit dereference if necessary, and convert to the given type if necessary.
typed_value_t
CheckAndImplicitlyDereferenceAndConvert (ast::Node*& expr, const type::Type* type);

void
check_assignment (typed_value_t left_tv,
                  typed_value_t right_tv,
                  const ast::Node& node,
                  const char* conversion_message,
                  const char* leak_message);

void
allocate_stack_variables (ast::Node* node);

decl::Method*
get_current_method (const ast::Node * node);

ReceiverAccess ComputeReceiverAccess (const ast::Node* node);

void
allocate_symbol (runtime::MemoryModel& memory_model,
                 decl::Symbol* symbol);

template <typename Iterator>
void
allocate_parameter (runtime::MemoryModel& memory_model,
                    Iterator pos,
                    Iterator limit)
{
  if (pos != limit)
    {
      allocate_parameter (memory_model, pos + 1, limit);
      allocate_symbol (memory_model, *pos);
    }
}

}

#endif // RC_SRC_SEMANTIC_HPP

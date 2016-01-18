#include "type.hpp"

#include "action.hpp"
#include "reaction.hpp"
#include "field.hpp"
#include "callable.hpp"
#include "bind.hpp"
#include "arch.hpp"

namespace type
{
using namespace decl;
using namespace semantic;

std::ostream&
operator<< (std::ostream& o, const Type& type)
{
  return o << type.to_string ();
}

const Pointer*
Type::get_pointer () const
{
  if (pointer_ == NULL)
    {
      const_cast<Type*> (this)->pointer_ = new Pointer (this);
    }
  return pointer_;
}

const Slice*
Type::get_slice () const
{
  if (slice_ == NULL)
    {
      const_cast<Type*> (this)->slice_ = new Slice (this);
    }
  return slice_;
}

const Array*
Type::get_array (IntValueType dimension) const
{
  assert (dimension >= 0);
  const Array* a;
  ArraysType::const_iterator pos = arrays_.find (dimension);
  if (pos != arrays_.end ())
    {
      a = pos->second;
    }
  else
    {
      a = new Array (dimension, this);
      const_cast<Type*> (this)->arrays_[dimension] = a;
    }
  return a;
}

const Heap*
Type::get_heap () const
{
  if (heap_ == NULL)
    {
      const_cast<Type*> (this)->heap_ = new Heap (this);
    }
  return heap_;
}

std::string
Array::to_string () const
{
  std::stringstream str;
  str << '[' << dimension << ']' << *base_;
  return str.str ();
}


Reaction *
NamedType::get_reaction (const std::string& identifier) const
{
  for (std::vector<Reaction*>::const_iterator pos = reactions_.begin (),
       limit = reactions_.end ();
       pos != limit;
       ++pos)
    {
      Reaction *a = *pos;
      if (a->name == identifier)
        {
          return a;
        }
    }

  return NULL;
}

decl::Method*
NamedType::get_method (const std::string& identifier) const
{
  for (std::vector< decl::Method*>::const_iterator pos = this->methods_.begin (),
       limit = this->methods_.end ();
       pos != limit;
       ++pos)
    {
      decl::Method* a = *pos;
      if (a->name == identifier)
        {
          return a;
        }
    }

  return NULL;
}

Initializer*
NamedType::get_initializer (const std::string& identifier) const
{
  for (std::vector<Initializer*>::const_iterator pos = this->initializers_.begin (),
       limit = this->initializers_.end ();
       pos != limit;
       ++pos)
    {
      Initializer* a = *pos;
      if (a->name == identifier)
        {
          return a;
        }
    }

  return NULL;
}

Getter*
NamedType::get_getter (const std::string& identifier) const
{
  for (std::vector<Getter*>::const_iterator pos = this->getters_.begin (),
       limit = this->getters_.end ();
       pos != limit;
       ++pos)
    {
      Getter* a = *pos;
      if (a->name == identifier)
        {
          return a;
        }
    }

  return NULL;
}

Action*
NamedType::get_action (const std::string& identifier) const
{
  for (ActionsType::const_iterator pos = this->actions_.begin (),
       limit = this->actions_.end ();
       pos != limit;
       ++pos)
    {
      Action* a = *pos;
      if (a->name == identifier)
        {
          return a;
        }
    }

  return NULL;
}

Bind*
NamedType::get_bind (const std::string& identifier) const
{
  for (BindsType::const_iterator pos = this->binds_.begin (),
       limit = this->binds_.end ();
       pos != limit;
       ++pos)
    {
      Bind* a = *pos;
      if (a->name == identifier)
        {
          return a;
        }
    }

  return NULL;
}

Struct*
Struct::append_field (const std::string& field_name, const Type* field_type)
{
  size_t alignment = field_type->Alignment ();
  offset_ = util::align_up (offset_, alignment);

  Field *field = new Field (field_name, field_type, offset_);
  fields_.push_back (field);

  offset_ += field_type->Size ();
  if (alignment > alignment_)
    {
      alignment_ = alignment;
    }

  return this;
}

Field *
Struct::get_field_i (const std::string& name) const
{
  for (std::vector<Field*>::const_iterator field = fields_.begin (),
       limit = fields_.end ();
       field != limit;
       ++field)
    {
      if (name == (*field)->name)
        {
          return (*field);
        }
    }
  return NULL;
}

const Type*
Type::select (const std::string& identifier) const
{
  Field* f = this->get_field (identifier);
  if (f)
    {
      return f->type;
    }

  decl::Method* m = this->get_method (identifier);
  if (m)
    {
      return m->methodType;
    }

  Initializer* i = this->get_initializer (identifier);
  if (i)
    {
      return i->initializerType;
    }

  Getter* g = this->get_getter (identifier);
  if (g)
    {
      return g->getterType;
    }

  Reaction* r = this->get_reaction (identifier);
  if (r)
    {
      return r->reaction_type;
    }

  Action* a = this->get_action (identifier);
  if (a)
    {
      return Void::Instance ();
    }

  Bind* b = this->get_bind (identifier);
  if (b)
    {
      return Void::Instance ();
    }

  return NULL;
}

ParameterSymbol*
Signature::Find (const std::string& name) const
{
  for (ParametersType::const_iterator ptr = parameters_.begin (),
       limit = parameters_.end ();
       ptr != limit;
       ++ptr)
    {
      if ((*ptr)->identifier == name)
        {
          return *ptr;
        }
    }
  return NULL;
}

const Type*
type_move (const Type* type)
{
  const Pointer* ptf = type_cast<Pointer> (type);
  if (ptf)
    {
      const Heap* h = type_cast<Heap> (ptf->Base ());
      if (h)
        {
          return ptf->Base ()->get_pointer ();
        }
    }

  return NULL;
}

const Type* type_merge (const Type* type)
{
  const Pointer* ptf = type_cast<Pointer> (type);

  if (ptf)
    {
      const Heap* h = type_cast<Heap> (ptf->Base ());
      if (h)
        {
          return h->Base ()->get_pointer ();
        }
    }

  return NULL;
}

const Type*
type_change (const Type* type)
{
  const Pointer* ptf = type_cast<Pointer> (type);

  if (ptf)
    {
      const Heap* h = type_cast<Heap> (ptf->Base ());
      if (h)
        {
          return h->Base ()->get_pointer ();
        }
    }

  return NULL;
}

#define ACCEPT(type) void \
type::Accept (Visitor& visitor) const \
{ \
  visitor.visit (*this); \
}

#define T_ACCEPT(type) template<> void type::Accept (Visitor& visitor) const { visitor.visit (*this); }

ACCEPT(NamedType)
ACCEPT(Component)
ACCEPT(Struct)
T_ACCEPT(Bool)
ACCEPT(Function)
ACCEPT(Method)
ACCEPT(Heap)
ACCEPT(FileDescriptor)
ACCEPT(Pointer)
ACCEPT(Signature)
T_ACCEPT(Int)
T_ACCEPT(Int8)
T_ACCEPT(Int16)
T_ACCEPT(Int32)
T_ACCEPT(Int64)
T_ACCEPT(Uint)
T_ACCEPT(Uint8)
T_ACCEPT(Uint16)
T_ACCEPT(Uint32)
T_ACCEPT(Uint64)
T_ACCEPT(Float32)
T_ACCEPT(Float64)
T_ACCEPT(Complex64)
T_ACCEPT(Complex128)
T_ACCEPT(StringU)
T_ACCEPT(Uintptr)
ACCEPT(Void)
ACCEPT(Nil)
ACCEPT(Boolean)
ACCEPT(Rune)
ACCEPT(Integer)
ACCEPT(Float)
ACCEPT(Complex)
ACCEPT(String)
ACCEPT(Array)
ACCEPT(Slice)
ACCEPT(Template)

struct IdenticalImpl
{
  bool retval;

  IdenticalImpl () : retval (false) { }

  void operator() (const Array& type1, const Array& type2)
  {
    UNIMPLEMENTED;
  }

  void operator() (const Slice& type1, const Slice& type2)
  {
    retval = identical (type1.Base (), type2.Base ());
  }

  void operator() (const Struct& type1, const Struct& type2)
  {
    UNIMPLEMENTED;
  }

  void operator() (const Pointer& type1, const Pointer& type2)
  {
    UNIMPLEMENTED;
  }

  void operator() (const Signature& type1, const Signature& type2)
  {
    retval = true;
    size_t x_arity = type1.Arity ();
    size_t y_arity = type2.Arity ();
    if (x_arity != y_arity)
      {
        return;
      }

    for (size_t idx = 0; idx != x_arity; ++idx)
      {
        const ParameterSymbol* x_parameter = type1.At (idx);
        const Type* x_parameter_type = x_parameter->type;
        const ParameterSymbol* y_parameter = type2.At (idx);
        const Type* y_parameter_type = y_parameter->type;

        if (!identical (x_parameter_type, y_parameter_type))
          {
            return;
          }
      }

    retval = true;
  }

  void operator() (const Function& type1, const Function& type2)
  {
    retval =
      identical (type1.GetSignature (), type2.GetSignature ()) &&
      identical (type1.GetReturnParameter ()->type, type2.GetReturnParameter ()->type);
  }

  // TODO:  Interfaces
  // void operator() (const Function& type1, const Function& type2) {
  //   UNIMPLEMENTED;
  // }

  // TODO:  Maps
  // void operator() (const Function& type1, const Function& type2) {
  //   UNIMPLEMENTED;
  // }

  template <typename T1, typename T2>
  void operator() (const T1& type1, const T2& type2)
  {
    retval = static_cast<const Type*> (&type1) == static_cast<const Type*> (&type2);
  }
};

bool
identical (const Type* x, const Type* y)
{
  if ((x == &named_uint8 && y == &named_byte) ||
      (x == &named_byte &&  y == &named_uint8))
    {
      return true;
    }

  if (x == y)
    {
      return true;
    }

  if (x->Level () == Type::NAMED ||
      y->Level () == Type::NAMED)
    {
      return false;
    }

  IdenticalImpl i;
  DoubleDispatch (x, y, i);
  return i.retval;
}

std::string
Signature::to_string () const
{
  std::stringstream str;
  str << '(';
  bool flag = false;
  for (ParametersType::const_iterator ptr = parameters_.begin (),
       limit = parameters_.end ();
       ptr != limit;
       ++ptr)
    {
      if (flag)
        {
          str << ", ";
        }

      str << (*ptr)->type->to_string ();
      flag = true;
    }
  str << ')';
  return str.str ();
}

#define INSTANCE(type) const type* \
type::Instance () \
{ \
  static type* i = new type ();                 \
  return i; \
}

INSTANCE(Error)
INSTANCE(Void)
INSTANCE(FileDescriptor)
INSTANCE(Nil)
INSTANCE(Boolean)
INSTANCE(Rune)
INSTANCE(Integer)
INSTANCE(Float)
INSTANCE(Complex)
INSTANCE(String)

Struct::Struct (bool insert_runtime) : offset_ (0), alignment_ (0)
{
  if (insert_runtime)
    {
      /* Prepend the field list with a pointer for the runtime. */
      append_field ("0runtime", Void::Instance ()->get_pointer ());
    }
}

const Type* type_dereference (const Type* type)
{
  struct visitor : public DefaultVisitor
  {
    const Type* retval;

    visitor () : retval (NULL) { }

    void visit (const Pointer& type)
    {
      retval = type.Base ();
    }
  };
  visitor v;
  type->Accept (v);
  return v.retval;
}

bool
type_contains_pointer (const Type* type)
{
  struct visitor : public DefaultVisitor
  {
    bool flag;

    visitor () : flag (false) { }

    void default_action (const Type& type)
    {
      TYPE_NOT_REACHED (type);
    }

    void visit (const NamedType& type)
    {
      type.UnderlyingType ()->Accept (*this);
    }

    void visit (const Void& type)
    { }

    void visit (const Bool& type)
    { }

    void visit (const Int& type)
    { }

    void visit (const Int8& type)
    { }

    void visit (const Int16& type)
    { }

    void visit (const Int32& type)
    { }

    void visit (const Int64& type)
    { }

    void visit (const Uint& type)
    { }

    void visit (const Uint8& type)
    { }

    void visit (const Uint16& type)
    { }

    void visit (const Uint32& type)
    { }

    void visit (const Uint64& type)
    { }

    void visit (const Float32& type)
    { }

    void visit (const Float64& type)
    { }

    void visit (const Complex64& type)
    { }

    void visit (const Complex128& type)
    { }

    void visit (const Array& type)
    {
      type.Base ()->Accept (*this);
    }

    void visit (const Pointer& type)
    {
      flag = true;
    }

    void visit (const Heap& type)
    {
      type.Base ()->Accept (*this);
    }

    void visit (const Slice& type)
    {
      flag = true;
    }

    void visit (const StringU& type)
    {
      flag = true;
    }

    void visit (const FileDescriptor& type)
    {
      flag = true;
    }

    void visit (const Function& type)
    {
      // Do nothing.
    }

    void visit (const Struct& type)
    {
      for (Struct::const_iterator pos = type.Begin (), limit = type.End ();
           pos != limit;
           ++pos)
        {
          (*pos)->type->Accept (*this);
        }
    }

    void visit (const Boolean& type) { }
    void visit (const Integer& type) { }
    void visit (const Float& type) { }
    void visit (const Complex& type) { }
    void visit (const String& type)
    {
      flag = true;
    }
  };
  visitor v;
  type->Accept (v);
  return v.flag;
}

NamedType::NamedType (const std::string& name,
                      const Type* subtype)
  : name_ (name)
  , underlyingType_ (subtype->UnderlyingType ())
{
  assert (underlyingType_->Level () == UNNAMED);
}

bool
type_is_integral (const Type* type)
{
  struct visitor : public DefaultVisitor
  {
    bool flag;
    visitor () : flag (false) { }

    void visit (const NamedType& type)
    {
      type.UnderlyingType ()->Accept (*this);
    }

    void visit (const Int& type)
    {
      flag = true;
    }

    void visit (const Int8& type)
    {
      flag = true;
    }

    void visit (const Int64& type)
    {
      flag = true;
    }

    void visit (const Uint& type)
    {
      flag = true;
    }

    void visit (const Uint8& type)
    {
      flag = true;
    }

    void visit (const Uint32& type)
    {
      flag = true;
    }

    void visit (const Uint64& type)
    {
      flag = true;
    }

    void visit (const Integer& type)
    {
      flag = true;
    }
  };
  visitor v;
  type->Accept (v);
  return v.flag;
}

bool
type_is_unsigned_integral (const Type* type)
{
  struct visitor : public DefaultVisitor
  {
    bool flag;
    visitor () : flag (false) { }

    void visit (const NamedType& type)
    {
      type.UnderlyingType ()->Accept (*this);
    }

    void visit (const Uint& type)
    {
      flag = true;
    }

    void visit (const Uint8& type)
    {
      flag = true;
    }
  };
  visitor v;
  type->Accept (v);
  return v.flag;
}

bool
type_is_floating (const Type* type)
{
  struct visitor : public DefaultVisitor
  {
    bool flag;
    visitor () : flag (false) { }

    void visit (const NamedType& type)
    {
      type.UnderlyingType ()->Accept (*this);
    }

    void visit (const Float64& type)
    {
      flag = true;
    }
  };
  visitor v;
  type->Accept (v);
  return v.flag;
}

bool
type_is_orderable (const Type* type)
{
  struct visitor : public DefaultVisitor
  {
    bool flag;
    visitor () : flag (false) { }

    void visit (const Pointer& type)
    {
      flag = true;
    }

    void visit (const NamedType& type)
    {
      type.UnderlyingType ()->Accept (*this);
    }

    void visit (const Int& type)
    {
      flag = true;
    }

    void visit (const Int8& type)
    {
      flag = true;
    }

    void visit (const Uint& type)
    {
      flag = true;
    }
  };
  visitor v;
  type->Accept (v);
  return v.flag;
}

bool
type_is_pointer_compare (const Type* left, const Type* right)
{
  const Type* l = type_strip (left);
  const Type* r = type_strip (right);

  return
    (type_cast<Pointer> (l) && type_cast<Nil>(r)) ||
    (type_cast<Nil> (l) && type_cast<Pointer> (r));

}

const Type*
type_strip (const Type* type)
{
  struct visitor : public DefaultVisitor
  {
    const Type* retval;
    visitor (const Type* t) : retval (t) { }

    void visit (const NamedType& type)
    {
      retval = type.UnderlyingType ();
    }
  };
  visitor v (type);
  type->Accept (v);
  if (type == v.retval)
    {
      return type;
    }
  else
    {
      return type_strip (v.retval);
    }
}

const Type*
type_index (const Type* base, const Type* index)
{
  struct visitor : public DefaultVisitor
  {
    const Type* index;
    const Type* result;

    visitor (const Type* i) : index (i), result (NULL) { }

    void visit (const Array& type)
    {
      struct visitor : public DefaultVisitor
      {
        const Array& array_type;
        const Type* result;

        visitor (const Array& at) : array_type (at), result (NULL) { }

        void visit (const NamedType& type)
        {
          type.UnderlyingType ()->Accept (*this);
        }

        void visit (const Uint& type)
        {
          result = array_type.Base ();
        }

        void visit (const Int& type)
        {
          result = array_type.Base ();
        }
      };
      visitor v (type);
      index->Accept (v);
      result = v.result;
    }

  };
  visitor v (index);
  base->Accept (v);
  return v.result;
}

bool
type_is_index (const Type* type, Int::ValueType index)
{
  struct visitor : public DefaultVisitor
  {
    Int::ValueType index;
    bool flag;

    visitor (Int::ValueType i) : index (i), flag (false) { }

    void visit (const Array& type)
    {
      flag = index >= 0 && index < type.dimension;
    }
  };
  visitor v (index);
  type->Accept (v);
  return v.flag;
}

bool
type_is_castable (const Type* x, const Type* y)
{
  return type_is_numeric (x) && type_is_numeric (y);
}

std::string
Function::to_string () const
{
  std::stringstream str;
  switch (function_kind)
    {
    case FUNCTION:
      str << "func " << *GetSignature () << ' ' << *GetReturnParameter ()->type;
      break;
    case PUSH_PORT:
      str << "push " << *GetSignature ();
      break;
    case PULL_PORT:
      str << "pull " << *GetSignature () << ' ' << *GetReturnParameter ()->type;
      break;
    }
  return str.str ();
}

std::string
Method::to_string () const
{
  std::stringstream str;
  switch (method_kind)
    {
    case METHOD:
      str << '(' << *receiver_type () << ')' << " func " << *signature << ' ' << *return_type ();
      break;
    case INITIALIZER:
      str << '(' << *receiver_type () << ')' << " init " << *signature << ' ' << *return_type ();
      break;
    case GETTER:
      str << '(' << *receiver_type () << ')' << " getter " << *signature << ' ' << *return_type ();
      break;
    case REACTION:
      str << '(' << *receiver_type () << ')' << " reaction " << *signature;
      break;
    }
  return str.str ();
}

Function*
Method::make_function_type (ParameterSymbol* this_parameter,
                            const Signature* signature,
                            ParameterSymbol* return_parameter)
{
  Signature* sig = new Signature ();

  sig->Append (this_parameter);
  for (Signature::const_iterator pos = signature->begin (),
       limit = signature->end ();
       pos != limit;
       ++pos)
    {
      sig->Append (*pos);
    }
  return new Function (Function::FUNCTION, sig, return_parameter);
}

const Type*
Function::GetReturnType () const
{
  return return_parameter_->type;
}

const Type*
Method::receiver_type () const
{
  return receiver_parameter->type;
}

const Type*
Method::return_type () const
{
  return return_parameter->type;
}

Method::Method (MethodKind k, const NamedType* named_type_,
                ParameterSymbol* receiver_parameter_,
                const Signature * signature_,
                ParameterSymbol* return_parameter_)
  : method_kind (k), named_type (named_type_)
  , receiver_parameter (receiver_parameter_)
  , function_type (make_function_type (receiver_parameter_, signature_, return_parameter_))
  , signature (signature_)
  , return_parameter (return_parameter_)
{
}

const Type*
Boolean::DefaultType () const
{
  return &named_bool;
}

const Type*
Rune::DefaultType () const
{
  return &named_rune;
}

const Type*
Integer::DefaultType () const
{
  return &named_int;
}

const Type*
Float::DefaultType () const
{
  return &named_float64;
}

const Type*
Complex::DefaultType () const
{
  return &named_complex128;
}

const Type*
String::DefaultType () const
{
  return &named_string;
}

const Type*
Choose (const Type* x, const Type* y)
{
  return (x->kind () > y->kind ()) ? x : y;
}

bool
assignable (const Type* from, const Value& from_value, const Type* to)
{
  if (to->IsUntyped ())
    {
      return false;
    }

  if (identical (from, to))
    {
      return true;
    }

  if (identical (from->UnderlyingType (), to->UnderlyingType ()) &&
      (from->Level () != Type::NAMED || to->Level () != Type::NAMED))
    {
      return true;
    }

  // TODO:  T is an interface type and x implements T.

  if (type_cast<Nil> (from) &&
      (type_cast<Pointer> (to->UnderlyingType ()) ||
       type_cast<Function> (to->UnderlyingType ()) ||
       type_cast<Slice> (to->UnderlyingType ())
       //type_cast<Map> (to->UnderlyingType ()) ||
       //type_cast<Interface> (to->UnderlyingType ())
      ))
    {
      return true;
    }

  if (from->IsUntyped () && from_value.representable (from, to))
    {
      return true;
    }

  return false;
}

Signature*
Signature::Append (ParameterSymbol* p)
{
  parameters_.push_back (p);
  size_ += util::align_up (p->type->Size (), arch::stack_alignment ());
  return this;
}

void
Signature::check_foreign_safe () const
{
  for (const_iterator pos = begin (), limit = end ();
       pos != limit;
       ++pos)
    {
      (*pos)->check_foreign_safe ();
    }
}

Field*
Struct::select_field (const std::string& name) const
{
  return get_field (name);
}

Callable*
NamedType::select_callable (const std::string& name) const
{
  decl::Method* m = this->get_method (name);
  if (m)
    {
      return m;
    }

  Initializer* i = this->get_initializer (name);
  if (i)
    {
      return i;
    }

  Getter* g = this->get_getter (name);
  if (g)
    {
      return g;
    }

  Reaction* r = this->get_reaction (name);
  if (r)
    {
      return r;
    }

  return NULL;
}

bool is_typed_integer (const Type* type)
{
  struct visitor : DefaultVisitor
  {
    bool flag;
    visitor () : flag (false) { }
    void visit (const Uint8& type)
    {
      flag = true;
    }
    void visit (const Uint16& type)
    {
      flag = true;
    }
    void visit (const Uint32& type)
    {
      flag = true;
    }
    void visit (const Uint64& type)
    {
      flag = true;
    }
    void visit (const Int8& type)
    {
      flag = true;
    }
    void visit (const Int16& type)
    {
      flag = true;
    }
    void visit (const Int32& type)
    {
      flag = true;
    }
    void visit (const Int64& type)
    {
      flag = true;
    }
    void visit (const Uint& type)
    {
      flag = true;
    }
    void visit (const Int& type)
    {
      flag = true;
    }
    void visit (const Uintptr& type)
    {
      flag = true;
    }
  };
  visitor v;
  type->UnderlyingType ()->Accept (v);
  return v.flag;
}

bool is_typed_unsigned_integer (const Type* type)
{
  switch (type->underlying_kind ())
    {
    case kUint8:
    case kUint16:
    case kUint32:
    case kUint64:
      return true;
    default:
      return false;
    }
}

bool is_typed_float (const Type* type)
{
  switch (type->underlying_kind ())
    {
    case kFloat32:
    case kFloat64:
      return true;
    default:
      return false;
    }
}

bool is_typed_complex (const Type* type)
{
  switch (type->underlying_kind ())
    {
    case kComplex64:
    case kComplex128:
      return true;
    default:
      return false;
    }
}

bool is_typed_numeric (const Type* type)
{
  switch (type->underlying_kind ())
    {
    case kUint8:
    case kUint16:
    case kUint32:
    case kUint64:
    case kInt8:
    case kInt16:
    case kInt32:
    case kInt64:
    case kFloat32:
    case kFloat64:
    case kComplex64:
    case kComplex128:
    case kUint:
    case kInt:
    case kUintptr:
      return true;
    default:
      return false;
    }
}

bool is_untyped_numeric (const Type* type)
{
  struct visitor : DefaultVisitor
  {
    bool flag;
    visitor () : flag (false) { }
    void visit (const Rune& type)
    {
      flag = true;
    }
    void visit (const Integer& type)
    {
      flag = true;
    }
    void visit (const Float& type)
    {
      flag = true;
    }
    void visit (const Complex& type)
    {
      flag = true;
    }
  };
  visitor v;
  type->UnderlyingType ()->Accept (v);
  return v.flag;
}

bool is_typed_string (const Type* type)
{
  return type->underlying_kind () == kStringU;
}

bool is_untyped_string (const Type* type)
{
  return type->underlying_kind () == kString;
}

bool is_any_string (const Type* type)
{
  return is_typed_string (type) || is_untyped_string (type);
}

bool is_slice_of_bytes (const Type* type)
{
  const Slice* slice_type = type_cast<Slice> (type->UnderlyingType ());
  if (slice_type == NULL)
    {
      return false;
    }
  return type_cast<Uint8> (slice_type->Base ()->UnderlyingType ()) != NULL;
}

bool is_slice_of_runes (const Type* type)
{
  const Slice* slice_type = type_cast<Slice> (type->UnderlyingType ());
  if (slice_type == NULL)
    {
      return false;
    }
  return type_cast<Int32> (slice_type->Base ()->UnderlyingType ()) != NULL;
}

bool orderable (const Type* type)
{
  switch (type->underlying_kind ())
    {
    case kUint8:
    case kUint16:
    case kUint32:
    case kUint64:
    case kInt8:
    case kInt16:
    case kInt32:
    case kInt64:
    case kFloat32:
    case kFloat64:
    case kUint:
    case kInt:
    case kUintptr:
    case kStringU:
    case kRune:
    case kInteger:
    case kFloat:
    case kString:
      return true;
    default:
      return false;
    }
}

bool arithmetic (const Type* type)
{
  switch (type->underlying_kind ())
    {
    case kUint8:
    case kUint16:
    case kUint32:
    case kUint64:
    case kInt8:
    case kInt16:
    case kInt32:
    case kInt64:
    case kFloat32:
    case kFloat64:
    case kComplex64:
    case kComplex128:
    case kUint:
    case kInt:
    case kUintptr:
    case kRune:
    case kInteger:
    case kFloat:
    case kComplex:
      return true;
    default:
      return false;
    }
}

bool integral (const Type* type)
{
  switch (type->underlying_kind ())
    {
    case kUint8:
    case kUint16:
    case kUint32:
    case kUint64:
    case kInt8:
    case kInt16:
    case kInt32:
    case kInt64:
    case kUint:
    case kInt:
    case kUintptr:
    case kRune:
    case kInteger:
      return true;
    default:
      return false;
    }
}

bool is_typed_boolean (const Type* type)
{
  return type->underlying_kind () == kBool;
}

bool is_untyped_boolean (const Type* type)
{
  return type->underlying_kind () == kBoolean;
}

bool is_any_boolean (const Type* type)
{
  return is_typed_boolean (type) || is_untyped_boolean (type);
}

const Pointer*
pointer_to_array (const Type* type)
{
  const Pointer* p = type_cast<Pointer> (type->UnderlyingType ());
  if (p != NULL && p->Base ()->underlying_kind () == kArray)
    {
      return p;
    }
  return NULL;
}

NamedType named_bool ("bool", Bool::Instance ());

NamedType named_uint8 ("uint8", Uint8::Instance ());
NamedType named_uint16 ("uint16", Uint16::Instance ());
NamedType named_uint32 ("uint32", Uint32::Instance ());
NamedType named_uint64 ("uint64", Uint64::Instance ());

NamedType named_int8 ("int8", Int8::Instance ());
NamedType named_int16 ("int16", Int16::Instance ());
NamedType named_int32 ("int32", Int32::Instance ());
NamedType named_int64 ("int64", Int64::Instance ());

NamedType named_float32 ("float32", Float32::Instance ());
NamedType named_float64 ("float64", Float64::Instance ());

NamedType named_complex64 ("complex64", Complex64::Instance ());
NamedType named_complex128 ("complex128", Complex128::Instance ());

NamedType named_byte ("byte", Uint8::Instance ());
NamedType named_rune ("rune", Int::Instance ());

NamedType named_uint ("uint", Uint::Instance ());
NamedType named_int ("int", Int::Instance ());
NamedType named_uintptr ("uintptr", Uintptr::Instance ());

NamedType named_string ("string", StringU::Instance ());

NamedType named_file_descriptor ("FileDescriptor", FileDescriptor::Instance ());
NamedType named_timespec ("timespec", (new Struct ())->append_field ("tv_sec", &named_uint64)->append_field ("tv_nsec", &named_uint64));

}

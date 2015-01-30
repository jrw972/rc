#include "instance.h"
#include "util.h"
#include "type.h"
#include "debug.h"
#include "binding.h"
#include "instance_set.h"
#include "instance_trigger_set.h"
#include <error.h>
#include "action.h"
#include "trigger.h"

struct instance_t
{
  const type_t *type;
  bool is_top_level;
  // Pointer to the run-time instance.
  instance_record_t* ptr;
  VECTOR_DECL (instance_sets, instance_set_t*);
};

typedef struct
{
  instance_t *instance;
  field_t *field;
  instance_t *subinstance;
} subinstance_t;

struct concrete_binding_t
{
  instance_t *output_instance;
  const field_t *output_port;
  instance_t *input_instance;
  const action_t *input_reaction;
};

struct instance_table_t
{
  VECTOR_DECL (instances, instance_t *);
  VECTOR_DECL (subinstances, subinstance_t);
  VECTOR_DECL (bindings, concrete_binding_t);
};

static instance_t *
instance_make (const type_t * type,
               bool is_top_level)
{
  instance_t *i = xmalloc (sizeof (instance_t));
  i->type = type;
  i->is_top_level = is_top_level;
  VECTOR_INIT (i->instance_sets, instance_set_t*, type_action_count (type), NULL);
  return i;
}

instance_table_t *
instance_table_make (void)
{
  instance_table_t *t = xmalloc (sizeof (instance_table_t));
  VECTOR_INIT (t->instances, instance_t *, 0, NULL);
  subinstance_t s;
  VECTOR_INIT (t->subinstances, subinstance_t, 0, s);
  concrete_binding_t b;
  VECTOR_INIT (t->bindings, concrete_binding_t, 0, b);
  return t;
}

instance_t *
instance_table_insert (instance_table_t * table, const type_t * type, bool is_top_level)
{
  instance_t *i = instance_make (type, is_top_level);
  VECTOR_PUSH (table->instances, instance_t *, i);
  return i;
}

void
instance_table_insert_subinstance (instance_table_t * table,
				   instance_t * instance,
				   field_t * field, instance_t * subinstance)
{
subinstance_t s = { instance: instance, field: field, subinstance:subinstance
    };
  VECTOR_PUSH (table->subinstances, subinstance_t, s);
}

instance_t* instance_table_get_subinstance (const instance_table_t * table,
                                            instance_t * instance,
                                            const field_t * field)
{
  VECTOR_FOREACH (pos, limit, table->subinstances, subinstance_t)
  {
    if (pos->instance == instance && pos->field == field)
      {
	return pos->subinstance;
      }
  }

  bug ("no subinstance\n");
}

static void
add_binding (instance_table_t * table,
	     instance_t * output_instance, const field_t * output_port,
	     instance_t * input_instance, const action_t * input_reaction)
{
concrete_binding_t v = { output_instance:output_instance,
  output_port:output_port,
  input_instance:input_instance,
  input_reaction:input_reaction
  };
  VECTOR_PUSH (table->bindings, concrete_binding_t, v);
}

void
instance_table_enumerate_bindings (instance_table_t * table)
{
  // For each instance.
  VECTOR_FOREACH (instance_pos, instance_limit, table->instances,
		  instance_t *)
  {
    instance_t *instance = *instance_pos;
    const type_t *type = instance->type;
    // Enumerate the bindings.
    binding_t **binding_pos;
    binding_t **binding_limit;
    for (binding_pos = type_component_bindings_begin (type), binding_limit =
	 type_component_bindings_end (type); binding_pos != binding_limit;
	 binding_pos = type_component_bindings_next (binding_pos))
      {
	binding_t *binding = *binding_pos;
	field_t **result_pos;
	field_t **result_limit;

	instance_t *output_instance = instance;
	result_pos = binding_output_begin (binding);
	result_limit = binding_output_end (binding);
	field_t *output_port = *result_pos;
	result_pos = binding_output_next (result_pos);
	while (result_pos != result_limit)
	  {
	    output_instance =
	      instance_table_get_subinstance (table, output_instance, output_port);
	    output_port = *result_pos;
	    result_pos = binding_output_next (result_pos);
	  }

	instance_t *input_instance = instance;
	result_pos = binding_input_begin (binding);
	result_limit = binding_input_end (binding);
        while (result_pos != result_limit)
          {
            field_t *input_reaction = *result_pos;
	    input_instance =
	      instance_table_get_subinstance (table, input_instance, input_reaction);
	    result_pos = binding_input_next (result_pos);
	  }

	add_binding (table, output_instance, output_port, input_instance,
		     binding_get_reaction (binding));
      }
  }
}

static void
transitive_closure (const instance_table_t * table,
		    instance_trigger_set_t * set,
                    instance_set_t* local_set,
		    instance_t * instance, const action_t * action)
{
  trigger_t **trigger_pos;
  trigger_t **trigger_limit;
  for (trigger_pos = action_trigger_begin (action), trigger_limit =
       action_trigger_end (action); trigger_pos != trigger_limit;
       trigger_pos = action_trigger_next (trigger_pos))
    {
      trigger_t *tg = *trigger_pos;

      if (instance_trigger_set_contains (set, instance, tg))
	{
	  error (-1, 0, "system is non-deterministic");
	}
      instance_trigger_set_push (set, instance, tg);
      instance_set_insert (local_set, instance, trigger_get_action (tg));

      field_t **field_pos;
      field_t **field_limit;
      for (field_pos = trigger_begin (tg), field_limit = trigger_end (tg);
	   field_pos != field_limit; field_pos = trigger_next (field_pos))
	{
	  const field_t *field = *field_pos;
	  VECTOR_FOREACH (binding_pos, binding_limit, table->bindings,
			  concrete_binding_t)
	  {
	    if (binding_pos->output_instance == instance &&
		binding_pos->output_port == field)
	      {
		transitive_closure (table, set, local_set,
				    binding_pos->input_instance,
				    binding_pos->input_reaction);
	      }
	  }
	}

      instance_trigger_set_pop (set);
    }
}

void
instance_table_analyze_composition (const instance_table_t * table)
{
  {
    // Check that no reaction is bound more than once.
    VECTOR_FOREACH (pos1, limit, table->bindings, concrete_binding_t)
    {
      concrete_binding_t *pos2;
      for (pos2 = VECTOR_NEXT (pos1); pos2 != limit;
	   pos2 = VECTOR_NEXT (pos2))
	{
	  if (pos1->input_instance == pos2->input_instance &&
	      pos1->input_reaction == pos2->input_reaction)
	    {
	      error (-1, 0, "reaction bound more than once");
	    }
	}
    }
  }

  instance_trigger_set_t *set = instance_trigger_set_make ();

  // For each instance.
  VECTOR_FOREACH (instance_pos, instance_limit, table->instances,
		  instance_t *)
  {
    instance_t *instance = *instance_pos;
    const type_t *type = instance->type;
    // For each action in the type.
    action_t **action_pos;
    action_t **action_end;
    for (action_pos = type_component_actions_begin (type), action_end =
	 type_component_actions_end (type); action_pos != action_end;
	 action_pos = type_component_actions_next (action_pos))
      {
        assert (instance_trigger_set_empty (set));
        instance_set_t* local_set = instance_set_make ();
	transitive_closure (table, set, local_set, instance, *action_pos);
        instance_set_sort (local_set);
        instance_set_set (instance, *action_pos, local_set);
      }
  }
}

void
instance_table_dump (const instance_table_t * table)
{
  {
    printf ("%s\t%s\n", "instance", "type");
    VECTOR_FOREACH (ptr, limit, table->instances, instance_t *)
    {
      printf ("%p\t%s\n", (*ptr), type_to_string ((*ptr)->type));
    }
    printf ("\n");
  }

  {
    printf ("%s\t%s\t\t%s\n", "instance", "field", "subinstance");
    VECTOR_FOREACH (ptr, limit, table->subinstances, subinstance_t)
    {
      printf ("%p\t%p\t%p\n", ptr->instance, ptr->field, ptr->subinstance);
    }
    printf ("\n");
  }

  {
    printf ("%s\t\t%s\t\t%s\t\t%s\n", "output", "port", "input", "reaction");
    VECTOR_FOREACH (ptr, limit, table->bindings, concrete_binding_t)
    {
      printf ("%p\t%p\t%p\t%p\n",
	      ptr->output_instance, ptr->output_port,
	      ptr->input_instance, ptr->input_reaction);
    }
    printf ("\n");
  }
}

instance_t** instance_table_begin (instance_table_t* table)
{
  return VECTOR_BEGIN (table->instances);
}

instance_t** instance_table_end (instance_table_t* table)
{
  return VECTOR_END (table->instances);
}

instance_t** instance_table_next (instance_t** pos)
{
  return VECTOR_NEXT (pos);
}

concrete_binding_t* instance_table_binding_begin (instance_table_t* table)
{
  return VECTOR_BEGIN (table->bindings);
}

concrete_binding_t* instance_table_binding_end (instance_table_t* table)
{
  return VECTOR_END (table->bindings);
}

concrete_binding_t* instance_table_binding_next (concrete_binding_t* pos)
{
  return VECTOR_NEXT (pos);
}

const type_t* instance_type (const instance_t* instance)
{
  return instance->type;
}

void instance_set_record (instance_t* instance, instance_record_t* ptr)
{
  instance->ptr = ptr;
}

instance_record_t* instance_get_record (const instance_t* instance)
{
  return instance->ptr;
}

bool instance_is_top_level (const instance_t* instance)
{
  return instance->is_top_level;
}

void instance_set_set (instance_t* instance,
                       const action_t* action,
                       instance_set_t* set)
{
  VECTOR_SET(instance->instance_sets, action_number (action), set);
}

const instance_set_t* instance_get_set (const instance_t* instance,
                                        const action_t* action)
{
  return VECTOR_AT(instance->instance_sets, action_number (action));
}

instance_t* concrete_binding_output_instance (const concrete_binding_t* binding)
{
  return binding->output_instance;
}

const field_t* concrete_binding_output_port (const concrete_binding_t* binding)
{
  return binding->output_port;
}

instance_t* concrete_binding_input_instance (const concrete_binding_t* binding)
{
  return binding->input_instance;
}

const action_t* concrete_binding_input_reaction (const concrete_binding_t* binding)
{
  return binding->input_reaction;
}

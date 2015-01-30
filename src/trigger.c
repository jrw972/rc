#include "trigger.h"
#include "util.h"

struct trigger_t
{
  VECTOR_DECL (fields, field_t *);
  TriggerAction action;
};

trigger_t *
trigger_make (void)
{
  trigger_t *tg = xmalloc (sizeof (trigger_t));
  VECTOR_INIT (tg->fields, field_t *, 0, NULL);
  tg->action = TRIGGER_READ;
  return tg;
}

void
trigger_add_field (trigger_t * group, field_t * field)
{
  VECTOR_PUSH (group->fields, field_t *, field);
}

field_t **
trigger_begin (const trigger_t * trigger)
{
  return VECTOR_BEGIN (trigger->fields);
}

field_t **
trigger_end (const trigger_t * trigger)
{
  return VECTOR_END (trigger->fields);
}

field_t **
trigger_next (field_t ** pos)
{
  return pos + 1;
}

void
trigger_set_action (trigger_t * trigger, TriggerAction action)
{
  trigger->action = action;
}

TriggerAction
trigger_get_action (const trigger_t * trigger)
{
  return trigger->action;
}

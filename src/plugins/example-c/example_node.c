/*
 * Copyright (c) 2015 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/pg/pg.h>
#include <vnet/ip/ip4_packet.h>
#include <vnet/ip/format.h>
#include <vppinfra/error.h>
#include <example-c/example.h>

typedef struct
{
  ip4_header_t header;
} example_trace_t;

/* packet trace format function */
static u8 *
format_example_trace (u8 *s, va_list *args)
{
  CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
  CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
  example_trace_t *t = va_arg (*args, example_trace_t *);

  return format (s, "header: %U\n", format_ip4_header, &t->header,
		 sizeof (t->header));
}

#define foreach_example_error _ (DROP, "Drops")

typedef enum
{
#define _(sym, str) EXAMPLE_ERROR_##sym,
  foreach_example_error
#undef _
    EXAMPLE_N_ERROR,
} example_error_t;

static char *example_error_strings[] = {
#define _(sym, string) string,
  foreach_example_error
#undef _
};

typedef enum
{
  EXAMPLE_DROP,
  EXAMPLE_N_NEXT,
} example_next_t;

VLIB_NODE_FN (example_node)
(vlib_main_t *vm, vlib_node_runtime_t *node, vlib_frame_t *frame)
{
  vlib_buffer_t *bufs[VLIB_FRAME_SIZE], **b = bufs;
  u16 nexts[VLIB_FRAME_SIZE], *next;
  u32 n_left, *from;

  from = vlib_frame_vector_args (frame);
  n_left = frame->n_vectors;
  next = nexts;

  vlib_get_buffers (vm, from, bufs, n_left);

  while (n_left > 0)
    {
      ip4_header_t *ip = vlib_buffer_get_current (b[0]);
      if (ip->protocol == IP_PROTOCOL_ICMP)
	{
	  b[0]->error = node->errors[EXAMPLE_ERROR_DROP];
	  next[0] = EXAMPLE_DROP;
	  if (PREDICT_FALSE (b[0]->flags & VLIB_BUFFER_IS_TRACED))
	    {
	      example_trace_t *t;

	      t = vlib_add_trace (vm, node, b[0], sizeof (*t));
	      t->header = *ip;
	    }
	}
      else
	{
	  vnet_feature_next_u16 (&next[0], b[0]);
	}

      b += 1;
      next += 1;
      n_left -= 1;
    }

  vlib_buffer_enqueue_to_next (vm, node, from, nexts, frame->n_vectors);

  return frame->n_vectors;
}

VLIB_REGISTER_NODE (example_node) =
{
  .name = "example-c",
  .vector_size = sizeof (u32),
  .format_trace = format_example_trace,
  .type = VLIB_NODE_TYPE_INTERNAL,

  .n_errors = ARRAY_LEN(example_error_strings),
  .error_strings = example_error_strings,

  .n_next_nodes = EXAMPLE_N_NEXT,

  .next_nodes = {
    [EXAMPLE_DROP] = "drop",
  },
};

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */

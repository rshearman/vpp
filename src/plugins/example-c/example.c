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

#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include <example-c/example.h>

#include <vlibapi/api.h>
#include <vlibmemory/api.h>

#include <example-c/example.api_enum.h>
#include <example-c/example.api_types.h>

#define REPLY_MSG_ID_BASE sm->msg_id_base
#include <vlibapi/api_helper_macros.h>

VLIB_PLUGIN_REGISTER () = {
  .version = "1.0",
  .description = "Example (C)",
};

sample_main_t sample_main;

int
example_enable_disable (u32 sw_if_index, bool enable_disable)
{
  return vnet_feature_enable_disable ("ip4-unicast", "example-c", sw_if_index,
				      enable_disable, 0, 0);
}

static clib_error_t *
example_enable_disable_command_fn (vlib_main_t *vm, unformat_input_t *input,
				   vlib_cli_command_t *cmd)
{
  vnet_main_t *vnet_main = vnet_get_main ();
  u32 sw_if_index = ~0;
  int enable_disable = 1;

  int rv;

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "disable"))
	enable_disable = 0;
      else if (unformat (input, "%U", unformat_vnet_sw_interface, vnet_main,
			 &sw_if_index))
	;
      else
	break;
    }

  if (sw_if_index == ~0)
    return clib_error_return (0, "Missing interface name");

  rv = example_enable_disable (sw_if_index, enable_disable);
  if (rv)
    return clib_error_return (0, "Failed: %d = %U", rv, format_vnet_api_errno,
			      rv);

  return NULL;
}

VLIB_CLI_COMMAND (example_command, static) = {
  .path = "c-example",
  .short_help = "c-example <interface-name> [disable]",
  .function = example_enable_disable_command_fn,
};

static void
vl_api_example_enable_disable_t_handler (vl_api_example_enable_disable_t *mp)
{
  vl_api_example_enable_disable_reply_t *rmp;
  sample_main_t *sm = &sample_main;
  int rv;

  rv = example_enable_disable (mp->sw_if_index, mp->enable);
  REPLY_MACRO (VL_API_EXAMPLE_ENABLE_DISABLE_REPLY);
}

#include <example-c/example.api.c>

static clib_error_t *
example_init (vlib_main_t *vm)
{
  sample_main_t *sm = &sample_main;

  /* Add our API messages to the global name_crc hash table */
  sm->msg_id_base = setup_message_id_table ();

  return 0;
}

VLIB_INIT_FUNCTION (example_init);

VNET_FEATURE_INIT (example, static) = {
  .arc_name = "ip4-unicast",
  .node_name = "example-c",
};

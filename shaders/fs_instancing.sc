$input v_texcoord0

/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "bgfx_shader.sh"

layout(location = 0) out vec4 f_fragcolor0; // gl_FragColor is deprecated, so this will be our output color
SAMPLER2D(s_texColor, 0);

void main()
{
	f_fragcolor0 = texture2D(s_texColor, v_texcoord0);
}
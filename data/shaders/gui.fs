varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform float u_time;
//uniform float u_tex_tiling;
uniform vec4 u_tex_range; // [startx, starty, width, height]

void main()
{
	vec2 uv = v_uv;
	uv.x = (u_tex_range.z)*uv.x + u_tex_range.x; 
	uv.y = (u_tex_range.w)*uv.y + u_tex_range.y;
	gl_FragColor = u_color * texture2D( u_texture, uv ); //* u_tex_tiling);
}
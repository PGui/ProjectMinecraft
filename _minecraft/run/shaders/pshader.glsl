uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float screen_width;
uniform float screen_height;

void main (void)
{
	vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ) );
	gl_FragColor = color;
}
uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float screen_width;
uniform float screen_height;
uniform float elapsed;

float LinearizeDepth(float z)
{
	float n = 0.5; // camera z near
  	float f = 10000.0; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ));
	float depth = texture2D( Texture1 , vec2( gl_TexCoord[0] )).r;	
	
	float var = 6.0f;
	vec4 sum;
	float variation = 0.0f;
	float depthAround = 0.0f;
	
	for(int i = -var/2.0f; i <=  var/2.0f; ++i )
	{
		for(int j = -var/2.0f; j <=  var/2.0f; ++j )
		{
			sum += texture2D( Texture0 , vec2( gl_TexCoord[0])+vec2(xstep*i, ystep*j));
			variation += abs(texture2D( Texture1 , vec2( gl_TexCoord[0] ) +  vec2(xstep*i, ystep*j)).r - depth);
			depthAround += texture2D( Texture1 , vec2( gl_TexCoord[0] ) +  vec2(xstep*i, ystep*j)).r;
		}
	}
	sum /= var*var;
	/*depthAround/= var*var;
	depthAround = (1.0f - depthAround);
	color += vec4(0,0,0,depthAround);*/
	
	//Outline
	float threshold = (1.0f - depth)/0.25f;//0.0001f;
	if(variation > threshold)
		color += vec4(1,0,0,0.8);
	
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);
	
	color = color * (1.0f-depth) + sum*depth;
	
	//Fog
	gl_FragColor = (1.0f- depth*0.85f)*color;
	
}
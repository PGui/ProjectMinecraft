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

	vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ) );
	float depth = texture2D( Texture1 , vec2( gl_TexCoord[0] ) ).r;	
	
	vec4 sum;
	float variation = 0.0f;
	for(int i = -2; i <= 2; ++i )
	{
		for(int j = -2; j <= 2; ++j )
		{
			sum += texture2D( Texture0 , vec2( gl_TexCoord[0])+vec2(xstep*i, ystep*j));
			variation += abs(texture2D( Texture1 , vec2( gl_TexCoord[0] ) +  vec2(xstep*i, ystep*j)).r - depth);
		}
	}
	sum /= 25;
	
	//Outline
	float threshold = (1.0f - depth)/0.8f;//0.0001f;
	if(variation > threshold)
		color = vec4(1,0,0,1);
	
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);
	
	color = color * (1.0f-depth) + sum*depth ;
	
	//Fog
	gl_FragColor = (1.0f- depth*0.85f)*color;
	
}
varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec4 color;

uniform float elapsed;
uniform mat4 invertView;

void main()
{
	if(gl_Color.r == 0.001 )
	{
		gl_Vertex.z += sin(elapsed*1.0) / 5.0;
	}
	gl_Vertex.z += (sin(elapsed*1.0 + gl_Vertex.x) + sin(elapsed*1.0 + gl_Vertex.y))/6.0f;
	
	// Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// Transforming The Normal To ModelView-Space
	normal = gl_NormalMatrix * gl_Normal; 

	//Direction lumiere
	vertex_to_light_vector = vec3(gl_LightSource[0].position);

	//Couleur
	color = gl_Color;
}
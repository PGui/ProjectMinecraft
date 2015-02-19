varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec4 color;

uniform float ambientLevel;

void main()
{
	// Scaling The Input Vector To Length 1
	vec3 normalized_normal = normalize(normal);
	vec3 normalized_vertex_to_light_vector = normalize(vertex_to_light_vector);

	// Calculating The Diffuse Term And Clamping It To [0;1]
	float DiffuseTerm = clamp(dot(normal, vertex_to_light_vector), 0.0, 1.0);

	// Calculating The Final Color
	gl_FragColor = color * (DiffuseTerm*(1-ambientLevel) + ambientLevel);
	gl_FragColor.a = color.a;
}
#version 330 core

in vec2 UV;
in vec3 Normal;

in vec3 VertexPosition_Worldspace;
in vec3 LightDirection_Cameraspace;
in vec3 Normal_Cameraspace;
in vec3 EyeDirection_cameraspace;

out vec4 outColor;

uniform sampler2D tex;
uniform vec3 lightposition_worldspace;
uniform vec3 lightcolor;
uniform float lightpower;

void main() {

	//normal of computed fragment (camera space)
	vec3 n = normalize(Normal_Cameraspace);
	//direction of light (from fragment to light)
	vec3 l = normalize(LightDirection_Cameraspace);
	//eye vector, towards the camera
	vec3 E = normalize(EyeDirection_cameraspace);
	//triangle light reflection
	vec3 R = reflect(-l, n);

	vec4 materialdiffuse = texture(tex, UV);
	vec3 materialambient = vec3(0.5, 0.5, 0.5) * materialdiffuse.xyz;
	vec3 materialspecular = lightcolor;
	float distance = length(lightposition_worldspace - VertexPosition_Worldspace);

	float cosAlpha = clamp(dot(E, R), 0, 1);

	float cosTheta = clamp(dot(n, l), 0, 1); //take the dot product between incoming light and vertex normal, then clamp it to 0-1 in case the light is behind (resulting in neg value)
	vec3 color = materialdiffuse.xyz * lightcolor * lightpower * cosTheta / (distance * distance)
		+ materialspecular * lightcolor * lightpower * pow(cosAlpha, 5) / (distance * distance)
		+ materialambient;

	outColor = vec4(color, materialdiffuse.a);
};
#version 400

in vec2 textureCoord; 
in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ambientStrength;
uniform float specularStrength;

void main()
{
	// Ambient lighting
	float ambientStrength = 0.2;
	vec3 ambient = ambientStrength * lightColor;
	
	// Diffuse lighting
	vec3 norm_normalized = normalize(norm);
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(norm_normalized, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	// Specular lighting
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm_normalized);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;
	
	// Combine lighting with texture
	vec3 result = (ambient + diffuse + specular) * texture(texture1, textureCoord).rgb;
	fragColor = vec4(result, 1.0);
}
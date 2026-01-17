#version 400

out vec4 fragColor;

uniform vec3 bulletColor;

void main()
{
    // Simple bright glowing bullet
    fragColor = vec4(bulletColor, 1.0);
}

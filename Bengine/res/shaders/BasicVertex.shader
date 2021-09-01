#version 330 core

in vec3 vertexposition_local;
in vec2 uv;
in vec3 normal;

out vec2 UV;
out vec3 Normal;

out vec3 VertexPosition_Worldspace;
out vec3 LightDirection_Cameraspace;
out vec3 Normal_Cameraspace;
out vec3 EyeDirection_cameraspace;

out vec3 LightPosition_worldspace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 lightposition_worldspace;

void main()
{
    UV = uv;
    Normal = normal;

    gl_Position = proj * view * model * vec4(vertexposition_local, 1); // THE ORDER MATTERS, PLEASE DONT FORGET, FOR THE LOVE OF GOD. P * V * M

    //worldspace position
    VertexPosition_Worldspace = (model * vec4(vertexposition_local, 1)).xyz;

    //vector from vertex to camera
    vec3 vertexPosition_cameraspace = (view * model * vec4(vertexposition_local, 1)).xyz;
    EyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;

    //vector from vertex to light
    vec3 lightposition_cameraspace = (view * vec4(lightposition_worldspace, 1)).xyz;
    LightDirection_Cameraspace = lightposition_cameraspace + EyeDirection_cameraspace;

    //normal of vertex
    Normal_Cameraspace = (view * model * vec4(normal, 0)).xyz;
}
#version 330 core
layout (location = 0) in vec3 attribute_vertex;
layout (location = 1) in vec2 attribute_textureVertex;
layout (location = 2) in vec3 attribute_normalVertex;

out vec4 fragmentPosition;
out vec2 texturePosition;
out vec4 fragmentNormal;

uniform mat4 modelMatrix;
uniform mat4 modelRotationMatrix;
uniform mat4 cameraMatrix;
uniform mat4 perspectiveMatrix;

void main(){

	vec4 vertexPosition = vec4(attribute_vertex.xyz, 1.0);
	vec4 vertexNormal = vec4(attribute_normalVertex.xyz, 1.0);

	fragmentPosition = vertexPosition;
	texturePosition = attribute_textureVertex;
	fragmentNormal = vertexNormal;

	vec4 projectedPosition = vertexPosition * modelRotationMatrix * modelMatrix * cameraMatrix * perspectiveMatrix;

	gl_Position = projectedPosition;

}


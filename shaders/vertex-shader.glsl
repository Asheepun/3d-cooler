#version 330 core
layout (location = 0) in vec3 attribute_vertex;
layout (location = 1) in vec2 attribute_textureVertex;
layout (location = 2) in vec3 attribute_normalVertex;

out vec4 fragmentPosition;
out vec2 texturePosition;
out vec4 fragmentNormal;
out mat4 modelMatrix;
out mat4 modelRotationMatrix;
out vec4 inputColor;

uniform samplerBuffer modelMatrixTextureBuffer;
uniform samplerBuffer modelRotationMatrixTextureBuffer;
uniform samplerBuffer inputColorTextureBuffer;

uniform mat4 cameraMatrix;
uniform mat4 perspectiveMatrix;

void main(){

	//get instanced variables from texture buffers
	modelMatrix = mat4(
		texelFetch(modelMatrixTextureBuffer, gl_InstanceID * 4 + 0),
		texelFetch(modelMatrixTextureBuffer, gl_InstanceID * 4 + 1),
		texelFetch(modelMatrixTextureBuffer, gl_InstanceID * 4 + 2),
		texelFetch(modelMatrixTextureBuffer, gl_InstanceID * 4 + 3)
	);

	modelRotationMatrix = mat4(
		texelFetch(modelRotationMatrixTextureBuffer, gl_InstanceID * 4 + 0),
		texelFetch(modelRotationMatrixTextureBuffer, gl_InstanceID * 4 + 1),
		texelFetch(modelRotationMatrixTextureBuffer, gl_InstanceID * 4 + 2),
		texelFetch(modelRotationMatrixTextureBuffer, gl_InstanceID * 4 + 3)
	);

	inputColor = texelFetch(inputColorTextureBuffer, gl_InstanceID);

	//calculate positions
	vec4 vertexPosition = vec4(attribute_vertex.xyz, 1.0);
	vec4 vertexNormal = vec4(attribute_normalVertex.xyz, 1.0);

	//vertexPosition.x += gl_InstanceID * 10;

	fragmentPosition = vertexPosition;
	texturePosition = attribute_textureVertex;
	fragmentNormal = vertexNormal;

	vec4 projectedPosition = vertexPosition * modelRotationMatrix * modelMatrix * cameraMatrix * perspectiveMatrix;

	gl_Position = projectedPosition;

}

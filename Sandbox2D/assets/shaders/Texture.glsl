#type vertex
#version 450 core
		
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
	
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 u_TexCoord;

void main ()
{
	u_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core
		
layout(location = 0) out vec4 color;
			
uniform sampler2D u_Texture;

in vec2 u_TexCoord;
			
void main ()
{
	color = texture(u_Texture, u_TexCoord);
}
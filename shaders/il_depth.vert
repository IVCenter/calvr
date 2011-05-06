uniform float frontAlio;
uniform float frontWorld;
uniform float backAlio;
uniform float backWorld;

void main()
{
  vec4 npos = gl_ModelViewMatrix * gl_Vertex;

  vec3 normal = normalize(gl_NormalMatrix * gl_Normal);

  vec3 ptol = normalize(gl_LightSource[0].position.xyz - npos.xyz);
  float diffuse = max(0.0, dot(ptol, normal));

  vec4 color;
  vec3 gAmb = gl_LightModel.ambient.xyz * gl_FrontMaterial.ambient.xyz;
  vec3 amb = gl_FrontMaterial.ambient.xyz * gl_LightSource[0].ambient.xyz;
  color.rgb = diffuse * gl_Color.rgb * gl_FrontMaterial.diffuse.rgb * gl_LightSource[0].diffuse.rgb + gAmb + amb;
  color.a = gl_Color.a * gl_FrontMaterial.diffuse.a * gl_LightSource[0].diffuse.a;
  clamp(color, 0.0, 1.0);

  float newy = npos.z + 2956.56;

  if(newy > 0)
  {
    float x = newy * 20.0 / frontWorld;
    newy = x * frontAlio / (x + 1.0);
  }
  else
  {
    float x = -newy * 20.0 / backWorld;
    newy = x * -backAlio / (x + 1.0);
  }

  //npos.y = newy + 2956.56;
  //npos.y = 2956.56;
  //npos.x = npos.x / 2.0;
  //npos.y = npos.y - 100.0;
  npos.z = newy - 2956.56;
  //npos.z = npos.z + 100.0;
  gl_Position = gl_ProjectionMatrix * npos;
  //gl_Position = ftransform();
  //gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

  gl_TexCoord[0]  = gl_MultiTexCoord0;


  //gl_FrontColor = gl_Color;
  //gl_BackColor = gl_Color;
  gl_FrontColor = color;
  gl_BackColor = color;
}

R"(

	// Get the texture coordinates (between 0 and 1 relative to the texture
	// width and height). The 0 matches the GL_TEXTURE0 in sprite.cpp etc.
	vec2 pos = gl_TexCoord[0].st;

	// Get the color that the texture gives for this position.
	vec4 color = texture2D(texture, pos);

	// We do not have to draw anything if it is transparent.
	if (color.a == 0)
	{
		// We have determined the color.
		gl_FragColor = vec4(0, 0, 0, 0);
		return;
	}

)"

R"(

	// Is there a mask set?
	if (masked)
	{
		// Get the mask coordinates (between 0 and 1 relative to the mask
		// width and height). The 2 matches the GL_TEXTURE2 in sprite.cpp etc.
		vec2 maskpos = gl_TexCoord[2].st;

		// Get the mask value that the mask gives for this position.
		float maskindex = texture2D(mask, maskpos).r;

		// We do not have to draw anything if it is not included in the mask.
		if (maskindex == 0)
		{
			// We have determined the color.
			gl_FragColor = vec4(0, 0, 0, 0);
			return;
		}
	}

)"

R"(

	// Get the texture coordinates (between 0 and 1 relative to the texture
	// width and height). We use 0 here because we did not use MultiTex.
	vec2 pos = gl_TexCoord[0].st;

	// Get the color index that the texture gives for this position.
	// We have palettes of size 2^X, but the indices are 0, 1/256, 2/256, etc.
	// Therefore we multiply the indices with 256/2^X, the "palettescale".
	float index = palettescale * texture2D(texture, pos).r;

	// Get the color that the palette gives us for this index.
	vec4 color = texture1D(palette, index);

	// We do not have to draw anything if it is transparent.
	if (color.a == 0)
	{
		// We have determined the color.
		gl_FragColor = vec4(0, 0, 0, 0);
		return;
	}

)"

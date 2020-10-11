R"(

	// Check if we are part of an obscured sprite or part of a shine.
	if (border)
	{
		// Change to dimmed down sepia.
		vec3 sepia;
		sepia.r = .812 * (color.r * .492 + color.g * .468 + color.b * .040);
		sepia.g = .721 * (color.r * .292 + color.g * .568 + color.b * .140);
		sepia.b = .562 * (color.r * .192 + color.g * .468 + color.b * .340);

		// Get the alpha channel.
		color = vec4(sepia, color.a);
	}
	else if (obscured > 0)
	{
		// We want diagonal stripes over the sprite, use the screen position
		// to determine whether we are part of the stripe.
		if (mod((gl_FragCoord.x + gl_FragCoord.y) / 8, 2) < 1)
		{
			// Mix the color with some black to make the stripe darker.
			color = mix(color, vec4(0, 0, 0, color.a), 0.2 * obscured);
		}
	}
	else if (grayed > 0)
	{
		// Change to grayscale (using values based on CCIR 601).
		float value = (0.299 * color.r + 0.587 * color.g + 0.114 * color.b);
		vec4 gray = vec4(value, value, value, color.a);
		color = mix(color, gray, grayed);
	}
	else if (shine > 0)
	{
		// We want diagonal stripes over the sprite, use the texture position
		// to determine whether we are part of the stripe.
		if (mod(gl_TexCoord[0].s + gl_TexCoord[0].t + 8 * theta, 40) < 0.5)
		{
			// Mix the color with some shine to make the stripe shinier.
			color = mix(color, vec4(shinecolor, color.a), 0.5 * shine);
		}
	}

)"

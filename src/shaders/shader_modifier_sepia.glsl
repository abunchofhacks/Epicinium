R"(

	// Change to sepia.
	vec3 sepia;
	sepia.r = color.r * .393 + color.g * .769 + color.b * .189;
	sepia.g = color.r * .349 + color.g * .686 + color.b * .168;
	sepia.b = color.r * .272 + color.g * .534 + color.b * .131;

	// We want some color back.
	sepia = mix(sepia, color.rgb, 1 / 3.0f);
	color = vec4(sepia, color.a);

)"

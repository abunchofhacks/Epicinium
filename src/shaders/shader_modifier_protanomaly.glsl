R"(

	// Change to decolored.
	vec3 decolored;
	decolored.r = color.r * 0.20 + color.g * 1.00 + color.b * -0.20;
	decolored.g = color.r * 0.20 + color.g * 1.00 + color.b * -0.20;
	decolored.b = color.r * 0.00 + color.g * 0.00 + color.b *  1.00;
	color = vec4(mix(decolored, color.rgb, 0.5), color.a);

)"

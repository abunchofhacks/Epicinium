R"(

	// Change to decolored.
	vec3 decolored;
	decolored.r = color.r * 0.30 + color.g * 0.70 + color.b * 0.10;
	decolored.g = color.r * 0.30 + color.g * 0.70 + color.b * 0.10;
	decolored.b = color.r * 0.00 + color.g * 0.00 + color.b * 1.00;
	color = vec4(decolored, color.a);

)"

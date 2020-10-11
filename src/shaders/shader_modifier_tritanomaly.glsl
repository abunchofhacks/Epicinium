R"(

	// Change to decolored.
	vec3 decolored;
	decolored.r = color.r * 0.90 + color.g * 0.70 + color.b * 0.00;
	decolored.g = color.r * 0.00 + color.g * 0.80 + color.b * 0.30;
	decolored.b = color.r * 0.20 + color.g * 0.70 + color.b * 0.50;
	color = vec4(mix(decolored, color.rgb, 0.5), color.a);

)"

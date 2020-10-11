R"(

	// Change to gameboy.
	vec3 gameboy;
	gameboy.r = 0.00 + color.r * 0.10 + color.g * 0.30 + color.b * 0.40;
	gameboy.g = 0.20 + color.r * 0.20 + color.g * 0.20 + color.b * 0.40;
	gameboy.b = 0.00 + color.r * 0.00 + color.g * 0.00 + color.b * 0.30;
	color = vec4(gameboy, color.a);

)"

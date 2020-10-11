R"(

	// Change to grayscale (using values based on CCIR 601).
	float value = (0.299 * color.r + 0.587 * color.g + 0.114 * color.b);
	color = vec4(value, value, value, color.a);

)"

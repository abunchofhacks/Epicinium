R"(

	// Build up over time.
	color.r = mod(color.r + theta / 30.0, 1.0);
	color.g = mod(color.g + theta / 60.0, 1.0);
	color.b = mod(color.b + theta / 90.0, 1.0);

)"

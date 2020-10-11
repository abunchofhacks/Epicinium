R"(

#version 120

uniform sampler2D texture;
uniform sampler2D mask;
uniform sampler1D palette;
uniform float palettescale;
uniform bool masked;
uniform bool border;
uniform float obscured;
uniform float grayed;
uniform float shine;
uniform vec3 shinecolor;
uniform float theta;

)"

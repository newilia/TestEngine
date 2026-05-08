uniform sampler2D texture;
uniform float aspect_ratio;
uniform int warp_count;
uniform vec2 warp_center_uv[64];
uniform float warp_strength[64];
uniform float warp_influence_radius[64];

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec2 radial = vec2(0.0);
    for (int i = 0; i < 64; i++)
    {
        if (i >= warp_count)
            break;
        vec2 d_uv = uv - warp_center_uv[i];
        vec2 d_screen = vec2(d_uv.x * aspect_ratio, d_uv.y);
        float dist = length(d_screen);
        float R = max(warp_influence_radius[i], 1e-5);
        float t = dist / R;
        float w = exp(-t * t);
        float f = 1.0 + warp_strength[i] * w;
        f = clamp(f, 0.08, 8.0);
        radial += (f - 1.0) * d_uv;
    }
    vec2 uv_sample = clamp(uv + radial, vec2(0.0), vec2(1.0));
    gl_FragColor = texture2D(texture, uv_sample);
}

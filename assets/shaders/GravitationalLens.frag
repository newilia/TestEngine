uniform sampler2D texture;
uniform float aspect_ratio;
uniform int lens_count;
uniform vec2 lens_uv[8];
uniform float lens_amplitude[8];
uniform float lens_falloff[8];

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec2 disp = vec2(0.0);
    for (int i = 0; i < 32; i++)
    {
        if (i >= lens_count)
            break;
        vec2 delta_uv = uv - lens_uv[i];
        vec2 d_screen = vec2(delta_uv.x * aspect_ratio, delta_uv.y);
        float dist = length(d_screen);
        vec2 dir_screen = dist > 1e-5 ? d_screen / dist : vec2(0.0);
        float inv = lens_falloff[i] / (dist * dist + 0.00015);
        vec2 disp_screen = dir_screen * lens_amplitude[i] * inv;
        disp += vec2(disp_screen.x / aspect_ratio, disp_screen.y);
    }
    vec2 uv2 = clamp(uv + disp, vec2(0.0), vec2(1.0));
    gl_FragColor = texture2D(texture, uv2);
}

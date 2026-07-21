uniform sampler2D texture;
uniform vec2 u_bounds_min;
uniform vec2 u_bounds_size;
uniform vec2 u_tiling;
uniform vec2 u_uv_offset;
uniform vec4 u_tint;
uniform int u_sphere_projection_active;
uniform float u_sphere_rotation;
uniform vec2 u_sphere_uv_offset;
uniform vec2 u_world_origin;
uniform vec2 u_world_dx;
uniform vec2 u_world_dy;
uniform float u_target_height;

const float kInvTwoPi = 0.159154943;
const float kInvPi = 0.318309886;

vec2 world_pos_from_frag()
{
    float px = gl_FragCoord.x - 0.5;
    float py = u_target_height - gl_FragCoord.y - 0.5;
    return u_world_origin + px * u_world_dx + py * u_world_dy;
}

void main()
{
    vec2 world = world_pos_from_frag();
    vec2 uv;

    if (u_sphere_projection_active != 0)
    {
        vec2 norm = (world - u_bounds_min) / u_bounds_size;
        vec2 p = norm * 2.0 - 1.0;
        float aspect = u_bounds_size.x / max(u_bounds_size.y, 1e-5);
        p.x *= aspect;
        float r2 = dot(p, p);
        if (r2 > 1.0)
        {
            gl_FragColor = vec4(0.0);
            return;
        }
        float z = sqrt(max(1.0 - r2, 0.0));
        float lon = atan(p.x, z) + u_sphere_rotation;
        float lat = asin(clamp(p.y, -1.0, 1.0));
        uv = vec2(lon * kInvTwoPi + 0.5, lat * kInvPi + 0.5);
        uv = fract(uv * u_tiling + u_uv_offset + u_sphere_uv_offset);
    }
    else
    {
        uv = fract((world - u_bounds_min) / u_bounds_size * u_tiling + u_uv_offset);
    }

    gl_FragColor = texture2D(texture, uv) * u_tint;
}

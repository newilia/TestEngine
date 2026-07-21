uniform sampler2D texture;
uniform vec2 u_bounds_size;
uniform mat3 u_local_from_world;
uniform vec2 u_tiling;
uniform vec2 u_uv_offset;
uniform vec4 u_tint;
uniform int u_sphere_projection_active;
uniform vec2 u_sphere_uv_offset;
uniform int u_sphere_unwrap;
uniform vec4 u_sphere_orientation;
uniform vec2 u_world_origin;
uniform vec2 u_world_dx;
uniform vec2 u_world_dy;
uniform float u_target_height;

const float kInvTwoPi = 0.159154943;
const float kInvPi = 0.318309886;

const int kSphereUnwrapHorizontal = 0;
const int kSphereUnwrapVertical = 1;

vec2 world_pos_from_frag()
{
    float px = gl_FragCoord.x - 0.5;
    float py = u_target_height - gl_FragCoord.y - 0.5;
    return u_world_origin + px * u_world_dx + py * u_world_dy;
}

vec2 local_pos_from_frag()
{
    vec2 world = world_pos_from_frag();
    return (u_local_from_world * vec3(world, 1.0)).xy;
}

vec3 quat_rotate(vec4 q, vec3 v)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

vec2 equirect_uv_horizontal(vec3 dir)
{
    float lon = atan(dir.x, dir.z);
    float lat = asin(clamp(dir.y, -1.0, 1.0));
    return vec2(lon * kInvTwoPi + 0.5, lat * kInvPi + 0.5);
}

vec2 equirect_uv_vertical(vec3 dir)
{
    float lon = atan(dir.y, dir.z);
    float lat = asin(clamp(dir.x, -1.0, 1.0));
    return vec2(lat * kInvPi + 0.5, lon * kInvTwoPi + 0.5);
}

void main()
{
    vec2 local = local_pos_from_frag();
    vec2 uv;

    if (u_sphere_projection_active != 0)
    {
        vec2 norm = local / u_bounds_size;
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
        vec3 dir = normalize(vec3(p.x, p.y, z));
        dir = quat_rotate(normalize(u_sphere_orientation), dir);

        if (u_sphere_unwrap == kSphereUnwrapVertical)
        {
            uv = equirect_uv_vertical(dir);
        }
        else
        {
            uv = equirect_uv_horizontal(dir);
        }
        uv = fract(uv * u_tiling + u_uv_offset + u_sphere_uv_offset);
    }
    else
    {
        uv = fract(local / u_bounds_size * u_tiling + u_uv_offset);
    }

    gl_FragColor = texture2D(texture, uv) * u_tint;
}

const int kShapeLightingMax = 64;
uniform vec4 u_fill_color;

uniform vec2 u_world_origin;
uniform vec2 u_world_dx;
uniform vec2 u_world_dy;
uniform float u_target_height;

uniform mat3 u_local_from_world;

uniform int u_shape_kind;
uniform int u_vertex_count;
uniform vec2 u_vertices[64];

uniform vec2 u_circle_center;
uniform float u_circle_radius;

uniform vec2 u_rect_min;
uniform vec2 u_rect_max;

uniform int u_light_count;
uniform vec2 u_light_pos[64];
uniform vec3 u_light_color[64];
uniform float u_light_radius[64];
uniform float u_light_height[64];

uniform int u_mode_bevel;
uniform float u_bevel_width;
uniform int u_ease_circ;
uniform float u_diffusion;
uniform float u_lighting_strength;
uniform float u_height_world_scale;

vec2 world_pos_from_frag()
{
    float px = gl_FragCoord.x - 0.5;
    float py = u_target_height - gl_FragCoord.y - 0.5;
    return u_world_origin + px * u_world_dx + py * u_world_dy;
}

vec2 local_pos_from_world(vec2 worldPos)
{
    return (u_local_from_world * vec3(worldPos, 1.0)).xy;
}

float dist_seg(vec2 p, vec2 a, vec2 b)
{
    vec2 ab = b - a;
    float ab2 = dot(ab, ab);
    if (ab2 < 1e-8)
        return distance(p, a);
    float t = clamp(dot(p - a, ab) / ab2, 0.0, 1.0);
    vec2 proj = a + t * ab;
    return distance(p, proj);
}

float convex_edge_distance(vec2 p)
{
    float d = 1e9;
    for (int i = 0; i < kShapeLightingMax; i++)
    {
        if (i >= u_vertex_count)
            break;
        int j = i + 1;
        if (j >= u_vertex_count)
            j = 0;
        d = min(d, dist_seg(p, u_vertices[i], u_vertices[j]));
    }
    return d;
}

float circle_edge_distance(vec2 p)
{
    return u_circle_radius - distance(p, u_circle_center);
}

float rect_edge_distance(vec2 p)
{
    float dx = min(p.x - u_rect_min.x, u_rect_max.x - p.x);
    float dy = min(p.y - u_rect_min.y, u_rect_max.y - p.y);
    return min(dx, dy);
}

vec2 rect_edge_distance_grad(vec2 p)
{
    float dxLeft = p.x - u_rect_min.x;
    float dxRight = u_rect_max.x - p.x;
    float dyBottom = p.y - u_rect_min.y;
    float dyTop = u_rect_max.y - p.y;
    float dx = min(dxLeft, dxRight);
    float dy = min(dyBottom, dyTop);

    if (dx < dy)
        return (dxLeft < dxRight) ? vec2(1.0, 0.0) : vec2(-1.0, 0.0);
    if (dy < dx)
        return (dyBottom < dyTop) ? vec2(0.0, 1.0) : vec2(0.0, -1.0);

    float sx = (dxLeft < dxRight) ? 1.0 : -1.0;
    float sy = (dyBottom < dyTop) ? 1.0 : -1.0;
    return normalize(vec2(sx, sy));
}

float edge_distance_field(vec2 localPos)
{
    if (u_shape_kind == 1)
        return circle_edge_distance(localPos);
    if (u_shape_kind == 2)
        return rect_edge_distance(localPos);
    return convex_edge_distance(localPos);
}

float surface_height_local(vec2 localPos)
{
    float bw = max(u_bevel_width, 1e-3);
    float h = edge_distance_field(localPos);
    float t = clamp(h / bw, 0.0, 1.0);
    if (u_ease_circ != 0)
        return bw * sqrt(max(2.0 * t - t * t, 0.0));
    return h;
}

float surface_height_world(vec2 worldPos)
{
    float zLocal = surface_height_local(local_pos_from_world(worldPos));
    return zLocal * u_height_world_scale;
}

vec2 surface_height_gradient_local(vec2 localPos)
{
    if (u_shape_kind == 1)
    {
        vec2 offset = localPos - u_circle_center;
        float r = length(offset);
        if (r < 1e-5)
            return vec2(0.0);

        float bw = max(u_bevel_width, 1e-3);
        if (u_ease_circ != 0)
        {
            float z = sqrt(max(bw * bw - r * r, 0.0));
            if (z < 1e-5)
                return vec2(0.0);
            return -offset / z;
        }

        float h = circle_edge_distance(localPos);
        return -offset / r;
    }

    if (u_shape_kind == 2)
        return rect_edge_distance_grad(localPos);

    float e = 0.6;
    float dpx = surface_height_local(localPos + vec2(e, 0.0)) - surface_height_local(localPos - vec2(e, 0.0));
    float dpy = surface_height_local(localPos + vec2(0.0, e)) - surface_height_local(localPos - vec2(0.0, e));
    return vec2(dpx, dpy) / (2.0 * e);
}

vec3 surface_normal_world(vec2 worldPos)
{
    vec2 localPos = local_pos_from_world(worldPos);
    vec2 gradLocal = surface_height_gradient_local(localPos);

    float dzdwx = (gradLocal.x * u_local_from_world[0][0] + gradLocal.y * u_local_from_world[1][0]) * u_height_world_scale;
    float dzdwy = (gradLocal.x * u_local_from_world[0][1] + gradLocal.y * u_local_from_world[1][1]) * u_height_world_scale;

    return normalize(vec3(-dzdwx, -dzdwy, 1.0));
}

void main()
{
    vec2 worldPos = world_pos_from_frag();

    vec3 lit = vec3(0.0);

    if (u_mode_bevel != 0)
    {
        float z = surface_height_world(worldPos);
        vec3 N = surface_normal_world(worldPos);
        float specPow = mix(64.0, 4.0, u_diffusion);

        for (int i = 0; i < kShapeLightingMax; i++)
        {
            if (i >= u_light_count)
                break;

            vec3 lc = u_light_color[i];
            float R = max(u_light_radius[i], 1.0);
            vec2 toLight2d = u_light_pos[i] - worldPos;
            float distSq = dot(toLight2d, toLight2d);
            float radiusSq = max(R * R, 1.0);
            float distanceAttenuation = 1.0 / (1.0 + distSq / radiusSq);

            vec3 toLight = vec3(toLight2d, u_light_height[i] - z);
            vec3 L = toLight / max(length(toLight), 1e-5);
            float ndotl = max(dot(N, L), 0.0);
            float glossy = pow(ndotl, specPow);
            float matte = ndotl;
            float surfaceResponse = mix(glossy, matte, u_diffusion);
            lit += lc * (surfaceResponse * distanceAttenuation);
        }
    }
    else
    {
        for (int i = 0; i < kShapeLightingMax; i++)
        {
            if (i >= u_light_count)
                break;

            vec3 lc = u_light_color[i];
            float R = max(u_light_radius[i], 1.0);
            vec2 toLight = u_light_pos[i] - worldPos;
            float distSq = dot(toLight, toLight);
            float radiusSq = max(R * R, 1.0);
            float distanceAttenuation = 1.0 / (1.0 + distSq / radiusSq);
            lit += lc * distanceAttenuation;
        }
    }

    lit *= clamp(u_lighting_strength, 0.0, 1.0);

    // Pass 2 only: RGB is lighting contribution; blend mode combines with pass-1 base colors.
    gl_FragColor = vec4(clamp(lit, 0.0, 1.0), u_fill_color.a);
}

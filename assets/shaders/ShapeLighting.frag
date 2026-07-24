const int kShapeLightingMax = 64;
uniform vec4 u_fill_color;

uniform vec2 u_world_origin;
uniform vec2 u_world_dx;
uniform vec2 u_world_dy;
uniform float u_target_height;

uniform int u_shape_kind;
uniform int u_vertex_count;
uniform vec2 u_vertices[64];

uniform vec2 u_circle_center;
uniform float u_circle_radius;

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

vec2 world_pos_from_frag()
{
    float px = gl_FragCoord.x - 0.5;
    float py = u_target_height - gl_FragCoord.y - 0.5;
    return u_world_origin + px * u_world_dx + py * u_world_dy;
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

float edge_distance_field(vec2 worldPos)
{
    if (u_shape_kind == 1)
        return circle_edge_distance(worldPos);
    return convex_edge_distance(worldPos);
}

float surface_height_world(vec2 worldPos)
{
    float bw = max(u_bevel_width, 1e-3);
    float h = edge_distance_field(worldPos);
    float t = clamp(h / bw, 0.0, 1.0);
    if (u_ease_circ != 0)
        return bw * sqrt(max(2.0 * t - t * t, 0.0));
    return h;
}

vec2 surface_height_gradient_world(vec2 worldPos)
{
    if (u_shape_kind == 1)
    {
        vec2 offset = worldPos - u_circle_center;
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

        return -offset / r;
    }

    float e = 0.6;
    float dpx = surface_height_world(worldPos + vec2(e, 0.0)) - surface_height_world(worldPos - vec2(e, 0.0));
    float dpy = surface_height_world(worldPos + vec2(0.0, e)) - surface_height_world(worldPos - vec2(0.0, e));
    return vec2(dpx, dpy) / (2.0 * e);
}

vec3 surface_normal_world(vec2 worldPos)
{
    vec2 grad = surface_height_gradient_world(worldPos);
    return normalize(vec3(-grad.x, -grad.y, 1.0));
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

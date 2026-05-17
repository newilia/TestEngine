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

uniform int u_mode_bevel;
uniform float u_bevel_width;
uniform int u_ease_circ;
uniform float u_diffusion;
uniform float u_lighting_strength;
uniform int u_blend_mode;

float ease_curve(float t)
{
    if (u_ease_circ != 0)
    {
        float u = clamp(t, 0.0, 1.0);
        return 1.0 - sqrt(max(1.0 - u * u, 0.0));
    }
    return clamp(t, 0.0, 1.0);
}

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

float rect_edge_distance(vec2 p)
{
    float dx = min(p.x - u_rect_min.x, u_rect_max.x - p.x);
    float dy = min(p.y - u_rect_min.y, u_rect_max.y - p.y);
    return min(dx, dy);
}

float edge_distance_field(vec2 localPos)
{
    if (u_shape_kind == 1)
        return circle_edge_distance(localPos);
    if (u_shape_kind == 2)
        return rect_edge_distance(localPos);
    return convex_edge_distance(localPos);
}

vec2 edge_distance_grad(vec2 localPos)
{
    if (u_shape_kind == 1)
    {
        vec2 d = localPos - u_circle_center;
        float len = length(d);
        if (len < 1e-5)
            return vec2(0.0);
        return d / len;
    }
    float e = 0.6;
    float dpx = edge_distance_field(localPos + vec2(e, 0.0)) - edge_distance_field(localPos - vec2(e, 0.0));
    float dpy = edge_distance_field(localPos + vec2(0.0, e)) - edge_distance_field(localPos - vec2(0.0, e));
    return vec2(-dpx, -dpy);
}

vec2 direction_local_from_world(vec2 worldDir)
{
    return (u_local_from_world * vec3(worldDir, 0.0)).xy;
}

void main()
{
    vec2 worldPos = world_pos_from_frag();
    vec3 lp = u_local_from_world * vec3(worldPos, 1.0);
    vec2 localPos = lp.xy;

    vec3 lit = vec3(0.0);
    float bw = max(u_bevel_width, 1e-3);

    for (int i = 0; i < kShapeLightingMax; i++)
    {
        if (i >= u_light_count)
            break;

        vec2 Lp = u_light_pos[i];
        vec3 lc = u_light_color[i];
        float R = max(u_light_radius[i], 1.0);
        vec2 toLight = Lp - worldPos;
        float distSq = dot(toLight, toLight);
        float radiusSq = max(R * R, 1.0);
        float distanceAttenuation = 1.0 / (1.0 + distSq / radiusSq);

        if (u_mode_bevel != 0)
        {
            float h = edge_distance_field(localPos);
            float traw = clamp(1.0 - h / bw, 0.0, 1.0);
            float profile = ease_curve(traw);
            float spread = mix(2.4, 0.35, u_diffusion);
            profile = pow(profile, spread);

            vec2 N = normalize(edge_distance_grad(localPos) + vec2(1e-5, 1e-5));
            vec2 Ldir = normalize(direction_local_from_world(toLight) + vec2(1e-5, 1e-5));
            float nd = max(dot(N, Ldir), 0.0);
            float specPow = mix(14.0, 2.0, u_diffusion);
            float glossySpec = pow(nd, specPow);
            float matteDiffuse = nd;
            float surfaceResponse = mix(glossySpec, matteDiffuse, u_diffusion);
            lit += lc * (surfaceResponse * profile * distanceAttenuation);
        }
        else
        {
            lit += lc * distanceAttenuation;
        }
    }

    lit *= clamp(u_lighting_strength, 0.0, 1.0);

    vec3 base = u_fill_color.rgb;
    float a = u_fill_color.a;
    vec3 rgb = clamp(base + lit, 0.0, 1.0);
    if (u_blend_mode == 1)
    {
        vec3 clampedLit = clamp(lit, 0.0, 1.0);
        rgb = 1.0 - (1.0 - base) * (1.0 - clampedLit);
    }
    gl_FragColor = vec4(clamp(rgb, 0.0, 1.0), a);
}

#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 world;
    vec3 view;
    vec3 normal;
} fsin;

#define TYPE_DIRECTIONAL 0
#define TYPE_POINT 1
#define TYPE_SPOT 2

struct Light {
    int type;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 position, direction;
    vec3 attenuation;
    vec2 cone;
};

struct Material {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 emissive;
    float shininess;
};

struct TexturedMaterial {
    sampler2D albedo;
    vec3 albedoTint;
    sampler2D specular;
    vec3 specularTint;
    sampler2D ambientOcclusion;
    sampler2D roughness;
    vec2 roughnessRange;
    sampler2D emissive;
    vec3 emissiveTint;
};

Material sample_material(TexturedMaterial tex_mat, vec2 tex_coord){
    Material mat;

    mat.diffuse = tex_mat.albedoTint * texture(tex_mat.albedo, tex_coord).rgb;
    mat.specular = tex_mat.specularTint * texture(tex_mat.specular, tex_coord).rgb;
    mat.emissive = tex_mat.emissiveTint * texture(tex_mat.emissive, tex_coord).rgb;
    mat.ambient = mat.diffuse * texture(tex_mat.ambientOcclusion, tex_coord).r;

    float roughness = mix(tex_mat.roughnessRange.x, tex_mat.roughnessRange.y, texture(tex_mat.roughness, tex_coord).r);

    mat.shininess = 2.0f / pow(clamp(roughness, 0.001f, 0.999f), 4.0f) - 2.0f;

    return mat;
}

uniform TexturedMaterial tex_mat;
#define MAX_LIGHT_COUNT 69
uniform Light lights[MAX_LIGHT_COUNT];
uniform int light_count;
out vec4 frag_color;

void main() {
    vec3 normal = normalize(fsin.normal);
    vec3 view = normalize(fsin.view);
    int count = min(light_count, MAX_LIGHT_COUNT);
    vec3 accumulated_light = vec3(0.0);

    Material material = sample_material(tex_mat, fsin.tex_coord);

    for(int index = 0; index < count; index++){
        Light light = lights[index];
        vec3 light_direction;
        float attenuation = 1;
        if(light.type == TYPE_DIRECTIONAL)
            light_direction = light.direction;
        else {
            light_direction = fsin.world - light.position;
            float distance = length(light_direction);
            light_direction /= distance;
            attenuation *= 1.0f / dot(light.attenuation, vec3(distance * distance, distance, 1));
            if(light.type == TYPE_SPOT){
                float angle = acos(dot(light.direction, light_direction));
                attenuation *= smoothstep(light.cone.x, light.cone.y, angle);
            }
        }

        vec3 reflected = reflect(light_direction, normal);
        float lambert = max(0.0f, dot(normal, -light_direction));
        float phong = pow(max(0.0f, dot(view, reflected)), material.shininess);

        vec3 diffuse = material.diffuse * light.diffuse * lambert;
        vec3 specular = material.specular * light.specular * phong;
        vec3 ambient = material.ambient * light.ambient;
        
        accumulated_light += (diffuse + specular + ambient) * attenuation + material.emissive;
    }

    frag_color = fsin.color * vec4(accumulated_light, 1.0f);
}
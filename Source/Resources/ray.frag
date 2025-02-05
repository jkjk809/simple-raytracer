#version 330 core



in vec2 TexCoord;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightAreas;

uniform samplerCube envMap;
uniform sampler2D accumulatedTexture;
uniform int frameCount;

uniform float time;
uniform vec3 lightDir;
uniform vec3 rayOrigin;
uniform mat4 invView;
uniform mat4 invProj;
uniform vec3 sphereColor;
uniform vec3 backgroundColor;
float radius;
uniform vec3 position;
uniform int sphereCount;
bool hit;

vec3 originRay;
vec3 secondHit;

struct Sphere
{
vec3 position;
float padding;
float radius;
float roughness;
float metalic;
float emissivePower; 
vec3 emissiveColor;
float padding2;
vec3 albedo;
float padding4;
//float ior; 
//float padding3; 
};

struct HitRecord
{
    float hitDistance;
    vec3 position;
    vec3 normal;
    vec3 color;
    float roughness;
    float metalic;
    float emissivePower; 
    vec3 emissiveColor;
   // float ior;
};

layout(std140) uniform SpheresBuffer {
    Sphere spheres[10]; 
};


//UTILITY RANDOM FUNCTION

float hash(float n) { return fract(sin(n) * 43758.5453123); }


vec3 hash3(vec3 p) {
    return vec3(hash(p.x + p.y * 57.0 + p.z * 113.0),
                hash(p.x * 157.0 + p.y * 213.0 + p.z * 37.0),
                hash(p.x * 47.0 + p.y * 93.0 + p.z * 129.0));
}


vec3 randomVec3(vec3 seed) {
    return hash3(seed) - 0.5;
}

vec3 sampleCubemap(vec3 direction) {
    return texture(envMap, direction).rgb;
}

float rand(vec3 seed) {
    return fract(sin(dot(seed.xy, vec2(12.9898, 78.233))) * 43758.5453);
}
///////////////

bool checkSphereHit(vec3 rayDirection, vec3 origin, float radius, out float hitDistance)
{
    float a = dot(rayDirection, rayDirection);
    float b = 2.0f * dot(origin, rayDirection);
    float c = dot(origin, origin) - radius * radius; 

    float discriminant = b * b - 4.0f * a * c;

    if(discriminant < 0)
    {
    hitDistance = 0.0f;
    return false;
    }

    float t0 = (-b + sqrt(discriminant)) / (2.0f * a);
    float t1 = (-b - sqrt(discriminant)) / (2.0f * a);

    if(t1 < 0.0)
    {
        hitDistance = -1.0f;
        return false;
    }
    else
    {
        hitDistance = t1;
        return true;
    }

}

float traceRay(vec3 originRay, vec3 rayDirection, inout HitRecord closestRecord)
{
    float closestHit = 1e30;

    hit = false;

    for (int i = 0; i < sphereCount; i++) 
    {
        vec3 origin = originRay - spheres[i].position;
        float radius = spheres[i].radius;
        float hitDistance;

        if (checkSphereHit(rayDirection, origin, radius, hitDistance)) 
        {
            if (hitDistance < closestHit)
            {
                closestHit = hitDistance;
                closestRecord.hitDistance = hitDistance;
                closestRecord.position = spheres[i].position + (origin + rayDirection * hitDistance);
                closestRecord.normal = normalize(closestRecord.position - spheres[i].position);
                closestRecord.color = spheres[i].albedo;
                closestRecord.roughness = spheres[i].roughness;
                closestRecord.metalic = spheres[i].metalic; 
                closestRecord.emissiveColor = spheres[i].emissiveColor;
                closestRecord.emissivePower = spheres[i].emissivePower;
               // closestRecord.ior = spheres[i].ior;
                hit = true;
            }
        }
    }

    if(hit)
    {
        return closestRecord.hitDistance;
    }

    return -1.0f;
}

void main()
{
    vec2 ndc = (TexCoord * 2.0 - 1.0);
    vec4 target = invProj * vec4(ndc.x, ndc.y, 1.0, 1.0);
    target /= target.w;

    vec3 seed = vec3(ndc.xy, time); 

    vec3 rayDirection = normalize((invView * vec4(target.xyz, 0.0)).xyz);
    originRay = rayOrigin;

    vec3 finalColor = backgroundColor;
    vec3 accumulatedColor = vec3(0.0);

    HitRecord closestRecord;

    float multiplier = 1.0f;
    float bounces = 20; 
    vec3 contribution = vec3(1.0f);

    hit = false;

    bool firstHit = true;

    for(int i = 0; i < bounces; i++)
    {
        vec3 color = vec3(0.0);

        float hitDistance = traceRay(originRay, rayDirection, closestRecord);
        
        if(hitDistance < 0.0)
        {
                vec3 skyColor = vec3(1.0f, 1.0f, 1.0f);
                skyColor = sampleCubemap(rayDirection);
                accumulatedColor += skyColor * contribution;
        break;
        }
        

        //float theta = max(dot(closestRecord.normal, -lightDir), 0.0f);
        color = closestRecord.color; // * theta;
        //color *= multiplier;
        //accumulatedColor += color * multiplier;
       // multiplier *= 0.5f;
        contribution *= color;
        accumulatedColor += closestRecord.emissiveColor * closestRecord.emissivePower;
       
       if(closestRecord.metalic == 1.0)
        {
         vec3 reflectedDirection = reflect(rayDirection, closestRecord.normal);
         vec3 refractedDirection = refract(rayDirection, closestRecord.normal, 1.0 / 1.5);
         float fresnelReflectance = pow((1.0 - 1.5) / (1.0 + 1.5), 2.0);
         fresnelReflectance += (1.0 - fresnelReflectance) * pow(1.0 - dot(-rayDirection, closestRecord.normal), 5.0);
     
         if (rand(seed) < fresnelReflectance) 
         {
         rayDirection = reflectedDirection;  // Reflect the ray
         } 
         else
         {
         rayDirection = refractedDirection;  // Refract the ray
         }
     
         originRay = closestRecord.position + rayDirection * 0.001f;
        }
     else
        {
        // not glass
            vec3 randomDirection = randomVec3(seed);
       
            rayDirection = reflect(rayDirection, closestRecord.normal + closestRecord.roughness * randomDirection);
            originRay = closestRecord.position + rayDirection * 0.001f;
       // }
        //finalColor = accumulatedColor;
        }
        
    }
    const vec3 bloomWeights = vec3(0.2126, 0.7152, 0.0722);

    if(frameCount == 1)
    {
        FragColor = vec4(accumulatedColor, 1.0);
     
        float brightness = dot(FragColor.rgb, bloomWeights);
        //if (brightness > 0.9f) 
        //{
        //BrightAreas = FragColor;
        //}
        //else
        //{
        //BrightAreas = vec4(0.0);
       // }
    }
    else
    {
    vec3 previousAccumulatedColor = texture(accumulatedTexture, TexCoord).rgb;
    //previousAccumulatedColor = vec3(0.0, 0.1, 0.1);
    float blendFactor = 1.0 / (float(frameCount) + 1);

    // Blend the current frame color with the accumulated color
    vec3 blendedColor = mix(previousAccumulatedColor, accumulatedColor, blendFactor);

    
    //FragColor0 = vec4(fra, 1.0);// Write the current frame color to the first color attachment (final image)
    FragColor = vec4(blendedColor, 1.0);




    
    
    float brightness = dot(FragColor.rgb, bloomWeights * 1.1);
    if (brightness > 0.89f) 
    {
    BrightAreas = FragColor;
    }
    else
    {
    BrightAreas = vec4(0.0);
    }
    }
}
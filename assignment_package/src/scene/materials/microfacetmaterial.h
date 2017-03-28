#ifndef MICROFACETMATERIAL_H
#define MICROFACETMATERIAL_H
#include "scene/materials/material.h"

class MicrofacetMaterial : public Material
{
public:
    MicrofacetMaterial(const Color3f &Kd, const Color3f &Ks,const Color3f &Kt,
                       float roughness, float indexOfRefraction,
                       const std::shared_ptr<QImage> &roughnessMap,
                       const std::shared_ptr<QImage> &diffuseColorMap,
                       const std::shared_ptr<QImage> &specularColorMap,
                       const std::shared_ptr<QImage> &normalMap)
             : Kd(Kd), Ks(Ks), Kt(Kt),roughness(roughness),indexOfRefraction(indexOfRefraction),
               roughnessMap(roughnessMap), diffuseColorMap(diffuseColorMap), normalMap(normalMap)
         {}

         void ProduceBSDF(Intersection *isect) const;


     private:
         Color3f Kd;                    // The diffuse spectral reflection of this material.
                                        // This is just the base color of the material

         Color3f Ks;                     // The color of the specular highlights

         Color3f Kt;

         float roughness;               // The overall roughness of the material. Will be multiplied
                                        // with the roughness map's values.

         float indexOfRefraction;
         std::shared_ptr<QImage> roughnessMap;
         std::shared_ptr<QImage> diffuseColorMap;
         std::shared_ptr<QImage> specularColorMap;
         std::shared_ptr<QImage> normalMap;
};

#endif // MICROFACETMATERIAL_H

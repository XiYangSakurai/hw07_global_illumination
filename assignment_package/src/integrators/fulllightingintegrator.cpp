#include "fulllightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth,Color3f beta1) const
{
    Color3f L(0.f), beta(1.f);
    Ray currentRay(ray);
    bool specularBounce = false;
    int bounces=recursionLimit-depth;
    while(bounces<recursionLimit) {
        //int bounces=recursionLimit-depth;
        //Intersect ray with scene
        Intersection isect;
        Vector3f wo = -currentRay.direction;
        bool foundIntersection = scene.Intersect(currentRay,&isect);
        //Possibly add emitted light at intersection
        if (bounces == 0 || specularBounce) {
        //Add emitted light
            if (foundIntersection)
            L += beta * isect.Le(wo);
        }
        //Terminate path if ray escaped or maxDepth was reached
        if (!foundIntersection || bounces >= recursionLimit)
        break;
        if(isect.objectHit->areaLight)
            break;
        //Compute scattering functions and skip over medium boundaries
        if(!isect.objectHit->GetMaterial())
                break;
        if (!isect.bsdf) {
        currentRay = isect.SpawnRay(currentRay.direction);
        bounces--;
        continue;
        }
        //Sample illumination from lights to find path contribution
        Point2f uLight = sampler->Get2D();
            Point2f uScattering = sampler->Get2D();
            int nLights = int(scene.lights.size());

            int lightNum = std::min((int)(sampler->Get1D()*nLights),nLights-1);
            const std::shared_ptr<Light> &light = scene.lights[lightNum];
            Color3f Ld(0.f);
            if(nLights>0){
                BxDFType bsdfFlags = specularBounce ? BSDF_ALL :
                BxDFType(BSDF_ALL & ~BSDF_SPECULAR);
        //Sample light source with multiple importance sampling
            Vector3f wiL;
            Float lightPdf = 0, scatteringPdf = 0;
            Color3f Li = light->Sample_Li(isect, uLight, &wiL, &lightPdf);
            //lightPdf/=nLights;
            wiL=glm::normalize(wiL);
            if (lightPdf > 0 && !IsBlack(Li)) {
            //Compute BSDF  for light sample
                Color3f f(0.0f);
                f=isect.bsdf->f(wo,wiL,bsdfFlags);
                scatteringPdf=isect.bsdf->Pdf(wo,wiL,bsdfFlags);
            if (!IsBlack(f)) {
            //Compute visibility
                Ray backRay=isect.SpawnRay(wiL);
                Intersection backIsect;
                bool back_inter=scene.Intersect(backRay,&backIsect);
                if(back_inter)
                 {
                const std::shared_ptr<Light> &b=backIsect.objectHit->areaLight;
                if(!( light.get()==b.get()))
                Li=Color3f(0.0f);
                }
            //Add light’s contribution to reflected radiance
                if (!IsBlack(Li)) {
                Float weight = PowerHeuristic(1, lightPdf, 1, scatteringPdf);
                Ld += f * Li * weight*AbsDot(isect.normalGeometric,wiL) / lightPdf;
                }
            }
            }

            //Sample BSDF with multiple importance sampling

            Color3f f(0.0f);

            //Sample scattered direction
            BxDFType sampledType;
            bool sampledSpecular = false;
            Vector3f wiS;
            Float lightPdf2 = 0, scatteringPdf2 = 0;
            f = isect.bsdf->Sample_f(wo, &wiS, uScattering, &scatteringPdf2,
            bsdfFlags, &sampledType);
            wiS=glm::normalize(wiS);
            //f *= AbsDot(wiS, isect.normalGeometric);
            sampledSpecular =(sampledType&BSDF_SPECULAR) != 0;
            if(!sampledSpecular){
            if (!IsBlack(f) && scatteringPdf2 > 0) {
            //Account for light contributions along sampled direction wi
                Float weight = 1;
                if (!sampledSpecular) {
                lightPdf2 = light->Pdf_Li(isect, wiS);
                //lightPdf2/=nLights;
                weight = PowerHeuristic(1, scatteringPdf2, 1, lightPdf2);
                if (lightPdf2 == 0.0f)
                    weight=0.0f;
                }
                //Add light contribution from material sampling
                Color3f Li(0.0f);

                Ray backRay2=isect.SpawnRay(wiS);
                Intersection backIsect2;
                bool back_inter2=scene.Intersect(backRay2,&backIsect2);
                if(back_inter2)
                {
                    const std::shared_ptr<Light> &b2=backIsect2.objectHit->areaLight;
                    if(!(light.get()==b2.get()))
                    Li=Color3f(0.0f);
                    else
                    Li=isect.Le(-wiS);}

                if (!IsBlack(Li))
                    Ld=Color3f(0.0f);
                Ld += f * Li *  weight*AbsDot(isect.normalGeometric,wiS) / scatteringPdf2;
            }
            }}
            L+=beta*Ld*(float)nLights;;

        //Sample BSDF to get new path direction
        Vector3f  wi;
        Float pdf;
        BxDFType flags;
        Color3f f = isect.bsdf->Sample_f(wo, &wi, sampler->Get2D(),
        &pdf, BSDF_ALL, &flags);
        if (IsBlack(f) || pdf == 0.f)
        break;
//        Ray backRay3=isect.SpawnRay(wi);
//            Intersection backIsect3;
//            bool back_inter3=scene.Intersect(backRay3,&backIsect3);
//            if(back_inter3)
// {
//            const std::shared_ptr<Light> &b3=backIsect3.objectHit->areaLight;
//            //if(light.get()==b3.get())
//              if(b3!=nullptr)
//                break;
//            }
        beta *= f * AbsDot(wi, isect.normalGeometric) / pdf;
        specularBounce = (flags & BSDF_SPECULAR) != 0;
        currentRay = isect.SpawnRay(wi);


        //Account for subsurface scattering, if applicable

        //Possibly terminate the path with Russian roulette
        if (bounces > 3) {
        Float q = std::max((Float).05, 1 - beta.y);
        if (sampler->Get1D() < q)
        break;
        beta /= 1 - q;
        }
        bounces++;


    }
    return L;

//    Color3f L(0.0f);
//    Color3f beta_loop(1.0f);
//    Ray r=ray;
//    Vector3f woW;
//    Vector3f wiW;
//    Point2f xi;
//    BxDFType sampletype;
//    bool specular_check=false;
//    while(depth>0)
//    {
//        sampletype=BxDFType(0);
//        woW=-r.direction;
//        Intersection intersect;
//        bool intersection_test=scene.Intersect(r,&intersect);
//        if(!intersection_test)
//            break;
//        if(depth==recursionLimit||specular_check)
//            L+=beta_loop*intersect.Le(woW);
//        if(intersect.objectHit->areaLight)
//            break;
//        //MIS
//        Color3f Ld(0.0f);
//        xi=sampler->Get2D();
//        float lightPdf=0,scatteringPdf=0;
//        float lightweight=0, scatterWeight=0;
//        Color3f f_light,f_scattering;
//        int light_num=scene.lights.size();
//        int select_index=std::min(int(sampler->Get1D()*light_num),light_num-1);
//        std::shared_ptr<Light> selected_light=scene.lights[select_index];

//        Color3f L_i=selected_light->Sample_Li(intersect,xi,&wiW,&lightPdf);

//        //lightPdf/=light_num;
//        if(fabs(lightPdf)==0.0f)
//            L_i=Color3f(0.0f);
//        wiW=glm::normalize(wiW);
//        r=intersect.SpawnRay(wiW);
//        Intersection occlusion;
//        if(scene.Intersect(r,&occlusion))
//        {
//            //judge if the Trace ray is occluded
//            if(occlusion.objectHit->areaLight!=scene.lights[select_index])
//                L_i=Color3f(0.0f);
//        }
//        f_light=intersect.bsdf->f(woW,wiW);
//        scatteringPdf=intersect.bsdf->Pdf(woW,wiW);
//        lightweight=PowerHeuristic(1,lightPdf,1,scatteringPdf);
//        //lightweight=BalanceHeuristic(1,lightPdf,1,scatteringPdf);
//        Ld+=f_light*AbsDot(intersect.normalGeometric,wiW)*L_i*lightweight/lightPdf;

//        //////////////////////////////////////////////////////////////////////////
//        //Remap xi
//        xi=sampler->Get2D();
//        f_scattering=intersect.bsdf->Sample_f(woW,&wiW,xi,&scatteringPdf,BSDF_ALL,&sampletype);
//        specular_check=(sampletype&BSDF_SPECULAR) != 0;
//        wiW=glm::normalize(wiW);
//        //lightPdf = selected_light->Pdf_Li(intersect,wiW)/light_num;
//        if(!specular_check)
//        {
//            lightPdf = selected_light->Pdf_Li(intersect,wiW);
//            Intersection occlusion2;
//            if(lightPdf!=0&&scatteringPdf!=0)
//            {
//                r=intersect.SpawnRay(wiW);
//                if(scene.Intersect(r,&occlusion2))
//                {
//                    //judge if the shadow ray is occluded
//                    if(occlusion2.objectHit->areaLight==selected_light)
//                    {
//                        scatterWeight=PowerHeuristic(1,scatteringPdf,1,lightPdf);
//                        //scatterWeight=BalanceHeuristic(1,scatteringPdf,1,lightPdf);
//                        L_i=occlusion2.Le(-wiW);
//                        Ld+=f_scattering*AbsDot(intersect.normalGeometric,wiW)*L_i*scatterWeight/scatteringPdf;
//                    }
//                }
//            }
//        }
//        Ld*=beta_loop;
//        Ld*=light_num;
//        L+=Ld;
//        //BIS
//        xi=sampler->Get2D();
//        float pdf=0;
//        Color3f f;
//        f=intersect.bsdf->Sample_f(woW,&wiW,xi,&pdf,BSDF_ALL,&sampletype);
//        wiW=glm::normalize(wiW);
//        if(fabs(pdf)==0||glm::length(f-Color3f(0.0f))==0.0f)
//            break;
//        beta_loop*=f*AbsDot(intersect.normalGeometric,wiW)/pdf;
//        specular_check=(sampletype&BSDF_SPECULAR) != 0;
//        r=intersect.SpawnRay(wiW);
////        if(recursionLimit-depth>=3)
////        {
////            Float temp=std::max(beta.x,beta.y);
////            Float max_componenet=std::max(temp,beta.z);
////            Float q = std::max((Float).05, 1.0f - max_componenet);
////            if (sampler->Get1D() < q)
////                break;
////            beta /= 1.0f - q;
////        }
//        depth--;


//    }
//    return L;


}

float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //TODO
    if(fabs(nf * fPdf + ng * gPdf)<FLT_EPSILON)
        return 0.0f;
    return (float)(nf * fPdf) / (nf * fPdf + ng * gPdf);
    //return 0.f;
}

float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    //TODO

    Float f = nf * fPdf, g = ng * gPdf;
    if(fabs(f * f + g * g)<FLT_EPSILON)
        return 0.0f;
    return (float)(f * f) / (f * f + g * g);
    //return 0.f;
}


#include <scene/geometry/mesh.h>
#include <la.h>
#include <tinyobj/tiny_obj_loader.h>
#include <iostream>

float Triangle::Area() const
{
    //TODO
    float S = 0.5f * glm::length(glm::cross(points[0] - points[1], points[0] - points[2]));
    return S;
    //return 0;
}

float Mesh::Area() const
{
    //TODO
    float s(0.0f);
    for(int i = 0; i < faces.size(); i++)
    {
        s+=faces[i]->Area();
    }
    return s;
}

Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3):
    Triangle(p1, p2, p3, glm::vec3(1), glm::vec3(1), glm::vec3(1), glm::vec2(0), glm::vec2(0), glm::vec2(0))
{
    for(int i = 0; i < 3; i++)
    {
        normals[i] = planeNormal;
    }
}


Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3):
    Triangle(p1, p2, p3, n1, n2, n3, glm::vec2(0), glm::vec2(0), glm::vec2(0))
{}


Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3, const glm::vec2 &t1, const glm::vec2 &t2, const glm::vec2 &t3)
{
    planeNormal = glm::normalize(glm::cross(p2 - p1, p3 - p2));
    points[0] = p1;
    points[1] = p2;
    points[2] = p3;
    normals[0] = n1;
    normals[1] = n2;
    normals[2] = n3;
    uvs[0] = t1;
    uvs[1] = t2;
    uvs[2] = t3;
}

float TriArea(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3)
{
    return glm::length(glm::cross(p1 - p2, p3 - p2)) * 0.5f;
}

//Returns the interpolation of the triangle's three normals based on the point inside the triangle that is given.
Normal3f Triangle::GetNormal(const Point3f &P) const
{
    float A = TriArea(points[0], points[1], points[2]);
    float A0 = TriArea(points[1], points[2], P);
    float A1 = TriArea(points[0], points[2], P);
    float A2 = TriArea(points[0], points[1], P);
    return glm::normalize(normals[0] * A0/A + normals[1] * A1/A + normals[2] * A2/A);
}


//The ray in this function is not transformed because it was *already* transformed in Mesh::GetIntersection
bool Triangle::Intersect(const Ray& r, Intersection* isect) const
{
    //1. Ray-plane intersection
    float t =  glm::dot(planeNormal, (points[0] - r.origin)) / glm::dot(planeNormal, r.direction);
    if(t < 0) return false;

    glm::vec3 P = r.origin + t * r.direction;
    //2. Barycentric test
    float S = 0.5f * glm::length(glm::cross(points[0] - points[1], points[0] - points[2]));
    float s1 = 0.5f * glm::length(glm::cross(P - points[1], P - points[2]))/S;
    float s2 = 0.5f * glm::length(glm::cross(P - points[2], P - points[0]))/S;
    float s3 = 0.5f * glm::length(glm::cross(P - points[0], P - points[1]))/S;
    float sum = s1 + s2 + s3;

    if(s1 >= 0 && s1 <= 1 && s2 >= 0 && s2 <= 1 && s3 >= 0 && s3 <= 1 && fequal(sum, 1.0f)){
        isect->t = t;
        return true;
    }
    return false;
}

bool Mesh::Intersect(const Ray& r, Intersection* isect) const
{
    Ray r_loc = r.GetTransformedCopy(transform.invT());
    float closest_t = -1;
    Triangle* closestTri = nullptr;
    for(int i = 0; i < faces.size(); i++){
        Intersection isx;
        if(faces[i]->Intersect(r_loc, &isx)){
            if(isx.t > 0 && (isx.t < closest_t || closest_t < 0)){
                closest_t = isx.t;
                closestTri = faces[i];
            }
        }
    }
    if(closest_t > 0)
    {
        Point3f P = Point3f(closest_t * r_loc.direction + r_loc.origin);
        closestTri->InitializeIntersection(isect, closest_t, P);
        this->InitializeIntersection(isect, closest_t, P);
        return true;
    }
    return false;
}

void Triangle::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = pLocal;
    isect->uv = GetUVCoordinates(pLocal);
    ComputeTriangleTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent), isect->uv);
    isect->t = t;
}

void Triangle::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    //Triangle uses ComputeTriangleTBN instead of this function.
}

void Triangle::ComputeTriangleTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit, const Point2f &uv) const
{
    *nor = GetNormal(P);
    //TODO: Compute tangent and bitangent based on UV coordinates.

//    Vector3f deltaPos1=points[1]-points[0];
//    Vector3f deltaPos2=points[2]-points[0];
//    Vector2f deltaUV1=uvs[1]-uvs[0];
//    Vector2f deltaUV2=uvs[2]-uvs[0];
//    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
//    Vector3f tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
//    Vector3f bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;
//    Point2f triUV=GetUVCoordinates(P);
//    Vector3f pos1=P-points[1];
//    Vector3f pos2=P-points[2];
//    Vector2f uv1=triUV-uvs[1];
//    Vector2f uv2=triUV-uvs[2];
//    Vector3f tangent=(uv2.y*pos1-uv1.y*pos2)/(uv2.y*uv1.x-uv1.y*uv2.x);
//    Vector3f bitangent=(pos2-uv2.x*tangent)/uv2.y;
    Vector3f tangent;
    Vector3f bitangent;
    CoordinateSystem(*nor,&tangent,&bitangent);
    *tan=glm::normalize(tangent);
    *bit=glm::normalize(bitangent);
}


void Mesh::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

void Mesh::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{

    //TODO: Transform nor, tan, and bit into world space
    for(int i=0;i<faces.size();i++)
    {
        Point3f points[3]=faces[i]->points;
        float S = 0.5f * glm::length(glm::cross(points[0] - points[1], points[0] - points[2]));
        float s1 = 0.5f * glm::length(glm::cross(P - points[1], P - points[2]))/S;
        float s2 = 0.5f * glm::length(glm::cross(P - points[2], P - points[0]))/S;
        float s3 = 0.5f * glm::length(glm::cross(P - points[0], P - points[1]))/S;
        float sum = s1 + s2 + s3;

        if(s1 >= 0 && s1 <= 1 && s2 >= 0 && s2 <= 1 && s3 >= 0 && s3 <= 1 && fequal(sum, 1.0f)){
            faces[i]->ComputeTriangleTBN(P,nor,tan,bit,faces[i]->GetUVCoordinates(P));
            break;
        }
    }
    *nor = transform.invTransT() * (*nor);
    *tan=glm::normalize(transform.T3()*(*tan));
    *bit=glm::normalize(transform.T3()*(*bit));

}


Point2f Mesh::GetUVCoordinates(const Point3f &point) const
{
    return glm::vec2();
}


Point2f Triangle::GetUVCoordinates(const Point3f &point) const
{
    float A = TriArea(points[0], points[1], points[2]);
    float A0 = TriArea(points[1], points[2], point);
    float A1 = TriArea(points[0], points[2], point);
    float A2 = TriArea(points[0], points[1], point);
    return uvs[0] * A0/A + uvs[1] * A1/A + uvs[2] * A2/A;
}

void Mesh::LoadOBJ(const QStringRef &filename, const QStringRef &local_path)
{
    QString filepath = local_path.toString(); filepath.append(filename);
    std::vector<tinyobj::shape_t> shapes; std::vector<tinyobj::material_t> materials;
    std::string errors = tinyobj::LoadObj(shapes, materials, filepath.toStdString().c_str());
    std::cout << errors << std::endl;
    if(errors.size() == 0)
    {
        //Read the information from the vector of shape_ts
        for(unsigned int i = 0; i < shapes.size(); i++)
        {
            std::vector<float> &positions = shapes[i].mesh.positions;
            std::vector<float> &normals = shapes[i].mesh.normals;
            std::vector<float> &uvs = shapes[i].mesh.texcoords;
            std::vector<unsigned int> &indices = shapes[i].mesh.indices;
            for(unsigned int j = 0; j < indices.size(); j += 3)
            {
                glm::vec3 p1(positions[indices[j]*3], positions[indices[j]*3+1], positions[indices[j]*3+2]);
                glm::vec3 p2(positions[indices[j+1]*3], positions[indices[j+1]*3+1], positions[indices[j+1]*3+2]);
                glm::vec3 p3(positions[indices[j+2]*3], positions[indices[j+2]*3+1], positions[indices[j+2]*3+2]);

                Triangle* t = new Triangle(p1, p2, p3);
                if(normals.size() > 0)
                {
                    glm::vec3 n1(normals[indices[j]*3], normals[indices[j]*3+1], normals[indices[j]*3+2]);
                    glm::vec3 n2(normals[indices[j+1]*3], normals[indices[j+1]*3+1], normals[indices[j+1]*3+2]);
                    glm::vec3 n3(normals[indices[j+2]*3], normals[indices[j+2]*3+1], normals[indices[j+2]*3+2]);
                    t->normals[0] = n1;
                    t->normals[1] = n2;
                    t->normals[2] = n3;
                }
                if(uvs.size() > 0)
                {
                    glm::vec2 t1(uvs[indices[j]*2], uvs[indices[j]*2+1]);
                    glm::vec2 t2(uvs[indices[j+1]*2], uvs[indices[j+1]*2+1]);
                    glm::vec2 t3(uvs[indices[j+2]*2], uvs[indices[j+2]*2+1]);
                    t->uvs[0] = t1;
                    t->uvs[1] = t2;
                    t->uvs[2] = t3;
                }
                this->faces.append(t);
            }
        }
        std::cout << "" << std::endl;
        //TODO: .mtl file loading
    }
    else
    {
        //An error loading the OBJ occurred!
        std::cout << errors << std::endl;
    }
}
Intersection Triangle::Sample(const Point2f &xi, Float *pdf) const
{
    //return Intersection();
    //Point2f b = UniformSampleTriangle(u);
    Float su0 = std::sqrt(xi[0]);
    Point2f b(1 - su0, xi[1] * su0);
        Intersection it;
        it.point = b[0] * points[0] + b[1] * points[1] + (1 - b[0] - b[1]) * points[2];
        // Compute surface normal for sampled point on triangle
        it.normalGeometric = glm::normalize(Normal3f(glm::cross(points[1] - points[0], points[2] - points[0])));
        // Ensure correct orientation of the geometric normal; follow the same
        // approach as was used in Triangle::Intersect().
        if (planeNormal!=Normal3f(0.0f)) {
            Normal3f ns(b[0] * normals[0] + b[1] * normals[1] +
                        (1 - b[0] - b[1]) *normals[2]);
            it.normalGeometric=(glm::dot(it.normalGeometric, ns) < 0.f) ? -it.normalGeometric : it.normalGeometric;
        } /*else if (reverseOrientation ^ transformSwapsHandedness)
            it.n *= -1;*/


        *pdf = 1 / Area();
        return it;
}
Intersection Mesh::Sample(const Point2f &xi, Float *pdf) const
{
    return Intersection();
}

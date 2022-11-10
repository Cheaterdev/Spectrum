#pragma once
import Core;

class mesh_simplifyer
{
        //std::function<vec3(unsigned int)> positions;
        //std::function<ivec3(unsigned int)> faces;



        struct vertex
        {
            vec3 pos;
        };

        struct face
        {
            vertex* a;
            vertex* b;
            vertex* c;
        };

        std::vector<vertex> vertexes;
        std::vector<face> faces;

        unsigned int faces_count;

        vertex* find_best_vertex(vertex* a, vertex* b)
        {
            return a;
        }

    public:

        mesh_simplifyer(std::function<vec3(unsigned int)> positions, std::function<ivec3(unsigned int)> faces, unsigned int faces_count)
        {
            /*	this->faces_count = faces_count;
            	this->positions = positions;
            	this->faces = faces;*/
        }

        void simplify(unsigned int target_count)
        {
        }


};

class tangent_generator
{
    public:

        tangent_generator(std::function<vec3(unsigned int)> positions, std::function<vec2(unsigned int)> tc, std::function<vec3(unsigned int)> normals, std::function<ivec3(unsigned int)> faces, unsigned int faces_count , unsigned int vertex_count)
        {
            std::vector<vec3> tan1, tan2;
            tan1.resize(vertex_count, vec4(0));
            tan2.resize(vertex_count, vec4(0));
            tangents.resize(vertex_count, vec4(0));

            //binormals.resize(vertex_count, vec3(0, 0, 0));
            for (unsigned int i = 0; i < faces_count; i++)
            {
                ivec3 face = faces(i);
                vec3 v1 = positions(face.x);
                vec3 v2 = positions(face.y);
                vec3 v3 = positions(face.z);
                vec2 w1 = tc(face.x);
                vec2 w2 = tc(face.y);
                vec2 w3 = tc(face.z);
                float x1 = v2.x - v1.x;
                float x2 = v3.x - v1.x;
                float y1 = v2.y - v1.y;
                float y2 = v3.y - v1.y;
                float z1 = v2.z - v1.z;
                float z2 = v3.z - v1.z;
                float s1 = w2.x - w1.x;
                float s2 = w3.x - w1.x;
                float t1 = w2.y - w1.y;
                float t2 = w3.y - w1.y;
                float r = (s1 * t2 - s2 * t1);

                if (r < 0.000001f)
                    r = 1;
                else
                    r = 1.0f / r;

                vec3 sdir = vec3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
                vec3 tdir = vec3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
                sdir.normalize();
                tdir.normalize();
                tan1[face.x] += sdir;
                tan1[face.y] += sdir;
                tan1[face.z] += sdir;
                tan2[face.x] += tdir;
                tan2[face.y] += tdir;
                tan2[face.z] += tdir;
            }

            for (unsigned int i = 0; i < vertex_count; i++)
            {
                vec3 n = normals(i);
                tan1[i].normalize();
                tan2[i].normalize();
                tangents[i] = vec4(tan1[i], (vec3::dot(vec3::cross(n, tan1[i]), tan2[i]) < 0.0f) ? -1.0f : 1.0f);
            }

            /*  for (unsigned int i = 0; i < vertex_count; i++)
              {
                  vec3 n = normals(i);
                  vec3 t = tan1[i];
                  n.normalize();
                  t.normalize();
                  // Gram-Schmidt orthogonalize
            	t = (t - n*vec3::dot(n, t)).normalize();
            	tangents[i] = vec4(t, (vec3::dot(vec3::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f);
              }*/
        }
        std::vector<vec4> tangents;
//	std::vector<vec3> binormals;
};
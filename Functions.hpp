
void createCylinder(int segs_theta, int segs_rho, float r, GLfloat *&verts, GLushort *&indices, GLfloat *&normals, GLfloat *&theta_coords);
void displaceCylinder(int segs, float r, GLfloat *&verts, GLfloat *&normals, GLfloat *&theta_coords, int octaves, float baseFrequency);
void cameraMoveTo(float &camera_x, float &camera_y, float &camera_z, float moveto_x, float moveto_y, float moveto_z, float base_speed, float t, bool speed);
inline float fade5(float t){  return t*t*t*(t*(t*6.0f-15.0f)+10.0f); }
/* THE CLAMPS */
inline float clamp(float a, float b, float v){return v < a ? a : (v > b ? b : v);};
inline float max(float a, float b){return a > b ? a : b;}
inline float min(float a, float b){return a < b ? a : b;}
inline float sign(float a){return a > 0.0 ? 1.0 : -1.0;}

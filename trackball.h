#ifndef __TRACKBALL_H__
#define __TRACKBALL_H__
#include "cgmath.h"

/*       A4. Full Solar System        */
/*                                    */
/*        2017312665 Park Gyeol       */
/*         Window Environment         */

struct trackball
{
	bool	b_tracking = false;
	bool	b_zooming = false;
	bool	b_panning = false;
	float	scale;			// controls how much rotation is applied
	mat4	view_matrix0;	// initial view matrix
	mat4	rotation = mat4::identity();

	mat4	rotation_degree;
	vec3	pan_degree;
	vec3	zoom_degree;

	vec3	eye0; 
	vec3	at0;
	vec3	up0;

	vec2	m0;				// the last mouse position
	vec2	m0_zoom;		// the last mouse position for zooming
	vec2	m0_pan;			// the last mouse position for panning

	// Function //
	trackball( float rot_scale=1.0f ) : scale(rot_scale){}
	void init();
	bool is_tracking() const { return b_tracking; }
	bool is_panning() const { return b_panning; }
	bool is_zooming() const { return b_zooming; }
	void begin( const mat4& view_matrix, vec2 m );
	void begin_pan(const vec3& eye, const vec3& at, const vec3& up, vec2 m);
	void begin_zoom( const vec3& eye, const vec3& at, const vec3& up, vec2 m );
	void end();
	void end_zoom(vec3& eye);
	void end_pan(vec3& eye, vec3& at);
	mat4 update(vec2 m);
	mat4 update_pan(vec2 m);
	mat4 update_zoom(vec2 m);
};

inline void trackball::init() {
	rotation = mat4::identity();
}

inline void trackball::begin( const mat4& view_matrix, vec2 m )
{
	b_tracking = true;			// enable trackball tracking
	m0 = m;						// save current mouse position
	view_matrix0 = view_matrix;	// save current view matrix
}

inline void trackball::begin_pan(const vec3& eye, const vec3& at, const vec3& up, vec2 m)
{
	b_panning = true;			// enable trackball tracking
	m0_pan = m;					// save current mouse position
	eye0 = eye;
	at0 = at;
	up0 = up;
}

inline void trackball::begin_zoom(const vec3& eye, const vec3& at, const vec3& up, vec2 m)
{
	b_zooming = true;			// enable trackball tracking
	m0_zoom = m;				// save current mouse position
	eye0 = eye;
	at0 = at;
	up0 = up;
}

inline void trackball::end() {
	b_tracking = false;
	rotation = rotation * rotation_degree;
}

inline void trackball::end_zoom(vec3& eye) {
	b_zooming = false;
	eye += zoom_degree;
}

inline void trackball::end_pan(vec3& eye, vec3& at) {
	b_panning = false;
	eye += pan_degree;
	at += pan_degree;
}

inline mat4 trackball::update_pan(vec2 m)
{
	vec3 eye_ = eye0;
	vec3 at_ = at0;
	vec3 up_ = up0;
	vec3 n_vector_norm = (eye_ - at_).length();
	pan_degree = vec3(m0_pan.x - m.x, 0, m0_pan.y - m.y) * n_vector_norm;
	mat4 result = mat4::look_at(eye_ + pan_degree, at_ + pan_degree, up_) * rotation;
	return result;
}

inline mat4 trackball::update_zoom(vec2 m)
{
	vec3 eye_ = eye0;
	vec3 at_ = at0;
	vec3 up_ = up0;
	vec3 n_vector = (eye_ - at_);
	zoom_degree = -(m0_zoom.y - m.y) * n_vector;
	mat4 result = mat4::look_at(eye_ + zoom_degree, at_, up_) * rotation;
	return result;
}

inline mat4 trackball::update( vec2 m )
{
	// project a 2D mouse position to a unit sphere
	static const vec3 p0 = vec3(0,0,1.0f);	// reference position on sphere
	vec3 p1 = vec3(m-m0,0);					// displacement
	if( !b_tracking || length(p1)<0.0001f ) return view_matrix0;		// ignore subtle movement
	p1 *= scale;														// apply rotation scale
	p1 = vec3(p1.x,p1.y,sqrtf(std::max(0.0f,1.0f-length2(p1)))).normalize();	// back-project z=0 onto the unit sphere

	// find rotation axis and angle in world space
	// - trackball self-rotation should be done at first in the world space
	// - mat3(view_matrix0): rotation-only view matrix
	// - mat3(view_matrix0).transpose(): inverse view-to-world matrix
	vec3 v = mat3(view_matrix0).transpose()*p0.cross(p1);
	float theta = asin( std::min(v.length(),1.0f) );

	// resulting view matrix, which first applies
	// trackball rotation in the world space
	rotation_degree = mat4::rotate(v.normalize(), theta);
	return view_matrix0 * rotation_degree;
}

// utility function
inline vec2 cursor_to_ndc( dvec2 cursor, ivec2 window_size )
{
	// normalize window pos to [0,1]^2
	vec2 npos = vec2( float(cursor.x)/float(window_size.x-1),
					  float(cursor.y)/float(window_size.y-1) );
	
	// normalize window pos to [-1,1]^2 with vertical flipping
	// vertical flipping: window coordinate system defines y from
	// top to bottom, while the trackball from bottom to top
	return vec2(npos.x*2.0f-1.0f,1.0f-npos.y*2.0f);
}

#endif // __TRACKBALL_H__

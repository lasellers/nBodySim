// nBody.h: interface for the nBody class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(H_CNBODY)
#define H_CNBODY

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
class CnBody
{
public:
	CnBody();
	CnBody(int const nb);
	virtual ~CnBody();
	
	//
	bool ComputeForModel();

	bool RigidBody_StaticCollision_2D();
	bool RigidBody_StaticCollision_3D();
	bool RigidBody_DynamicCollision_2D();
	bool RigidBody_DynamicCollision_3D();

	//
	void Reset(int const nb);

	//
	double random_range(int const min, int const max);
	bool iscollision3d(int const n, int const i);
	void kill_velocities();
	void kill_velocity(int const n);
	void TimeFactor(int const new_factor);
	void recompute_viewport();
	void unignore_all();
	void ignore_all();
	int count_ignored();
	int count_notignored();
	
	//
	void reset_collision_flags();
	void flag_ignoreable_bodies_2D();
	void flag_ignoreable_bodies_3D();
	void figure_viewport_and_center_2D();
	void figure_viewport_and_center_3D();
	
public:
	double centerx, centery;
	double centerz;
	double left, top, right, bottom;
	
	double close, distant;
	bool random_initial_velocities;
	int frame;
	int iterations;
	bool lock_viewport;
	int viewport_distance;
	int nBodies;
	int nViewable;
	double time_factor;
	
	double e; //coefficient of restitution
	
	typedef struct _pav
	{
		double position;
		double velocity;
	} PAV;
	
	typedef struct _BODY
	{
		PAV x;
		PAV y;
		PAV z;
		double mass; //mass;
		double radius; //radius
		bool collision;
		bool ignore;
		bool merge;
		//bool deformable
	} BODY;
	BODY *body;
	
public:
	// physics modelling: body, collision and dimensions
	enum BODY_TYPE
	{
		RIGID_BODY	=0,
			FLUID_BODY	=1
	};
	BODY_TYPE body_type;
	enum COLLISION_TYPE
	{
		STATIC_COLLISION	=0,
			DYNAMIC_COLLISION	=1
	};
	COLLISION_TYPE collision_type;
	enum DIMENSION_TYPE
	{
		TWO_DIMENSIONS	=2,
			THREE_DIMENSIONS	=3
	};
	DIMENSION_TYPE dimension_type;
	
private:
	static const double G;
	static const double PI;
};

#endif

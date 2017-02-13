// nBody.cpp: implementation of the nBody class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "nBody.h"

#include "stdlib.h" //for rand
#include <time.h> //for time for srand
#include "math.h" //for log

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// 6.673x10-11 N-m2/kg2 in SI units
const double CnBody::G=-6.670e-11; //.00000000006673;

const double CnBody::PI=3.14159265359;

//
CnBody::CnBody()
{
	//
	random_initial_velocities=false;
	time_factor=100.0;
	
	body_type=RIGID_BODY;
	collision_type=STATIC_COLLISION;
	dimension_type=TWO_DIMENSIONS;
	
	viewport_distance=1000;
	lock_viewport=false;
	e=0.5;
	
	nViewable=0;
	
	body=NULL;
}

CnBody::CnBody(int const nb)
{
	//
	random_initial_velocities=false;
	time_factor=100.0;

	body_type=RIGID_BODY;
	collision_type=DYNAMIC_COLLISION;
	dimension_type=TWO_DIMENSIONS;
	
	viewport_distance=1000;
	lock_viewport=false;
	e=0.0;
	nViewable=nb;
	
	body=NULL;
	Reset(nb);
}


/*
*/
void CnBody::Reset(int const nb)
{
	//
	srand(time(NULL));
	
	frame=0;
	iterations=0;
	
	centerx=0;
	centery=0;
	centerz=0;
	
	left=0; right=600;
	top=0; bottom=480;
	close=0; distant=600;
	
	nBodies=nb;
	
	//
	delete [] body;
	body=new BODY[nBodies];
	
	//
	for(int n=0;n<nBodies;n++)
	{
		//
		body[n].collision=false;
		body[n].ignore=false;
		body[n].merge=false;
		
		body[n].mass=random_range(0,100000);
		body[n].radius=log(body[n].mass);
		
		body[n].x.position=random_range(-200,200);
		body[n].y.position=random_range(-200,200);
		body[n].z.position=random_range(-200,200);
		
		//
		if(random_initial_velocities)
		{
			body[n].x.velocity=random_range(-1,1);
			body[n].y.velocity=random_range(-1,1);
			body[n].z.velocity=random_range(-3,3);
		}
		else
		{
			body[n].x.velocity=0;
			body[n].y.velocity=0;
			body[n].z.velocity=0;
		}
	}
}


/*
*/
CnBody::~CnBody()
{
	delete[] body;
	body=NULL;
}

/*
*/
double CnBody::random_range(int const min, int const max)
{
	return (((double)rand()/(double)RAND_MAX)*(double)(max-min))+min;
}

/*
*/
void CnBody::kill_velocities()
{
	for(int n=0;n<nBodies;n++)
	{
		body[n].x.velocity=0;
		body[n].y.velocity=0;
		body[n].z.velocity=0;
	}
}

/*
*/
void CnBody::kill_velocity(int const n)
{
	if(n>=0 && n<nBodies)
	{
		body[n].x.velocity=0;
		body[n].y.velocity=0;
		body[n].z.velocity=0;
	}
}

/*
*/
void CnBody::TimeFactor(int const new_factor)
{
	//
	const double f=(double)new_factor/(double)time_factor;
	for(int n=0;n<nBodies;n++)
	{
		body[n].x.velocity*=f;
		body[n].y.velocity*=f;
		body[n].z.velocity*=f;
	}
	
	//
	time_factor=new_factor;
}



/*
*/					  
void CnBody::recompute_viewport()
{
	right=viewport_distance/2;
	left=-right;
	bottom=viewport_distance/2;
	top=-bottom;
	distant=viewport_distance/2;
	close=-distant;
	centerx=(right+left)/2.0;
	centery=(bottom+top)/2.0;
	centerz=(distant+close)/2.0;
}

/*
*/
void CnBody::unignore_all()
{
	for(int n=0;n<nBodies;n++) body[n].ignore=false;
}

/*
*/
void CnBody::ignore_all()
{
	for(int n=0;n<nBodies;n++) body[n].ignore=true;
}

/*
*/
int CnBody::count_ignored()
{
	int count=0;
	for(int n=0;n<nBodies;n++)
	{
		if(body[n].ignore)
			count++;
	}
	return count;
}

/*
*/
int CnBody::count_notignored()
{
	int count=0;
	for(int n=0;n<nBodies;n++)
	{
		if(!body[n].ignore)
			count++;
	}
	return count;
}














// ////////////////////////////////////////////////////////////////////////

//
void CnBody::reset_collision_flags()
{
	if(viewport_distance>0)
	{
		for(int a=0;a<nBodies;a++)
		{
			body[a].collision=false;
		}
	}
}
// flag anything that falls outside the "ignore distance"
void CnBody::flag_ignoreable_bodies_2D()
{
	if(viewport_distance>0)
	{
		for(int a=0;a<nBodies;a++)
		{
			if(
				(body[a].x.position<-viewport_distance) ||
				(body[a].x.position>viewport_distance) ||
				(body[a].y.position<-viewport_distance) ||
				(body[a].y.position>viewport_distance)
				)
				body[a].ignore=true;
		}
	}
}
// flag anything that falls outside the "ignore distance"
void CnBody::flag_ignoreable_bodies_3D()
{
	if(viewport_distance>0)
	{
		for(int a=0;a<nBodies;a++)
		{
			if(
				(body[a].x.position<-viewport_distance) ||
				(body[a].x.position>viewport_distance)  ||
				(body[a].y.position<-viewport_distance) ||
				(body[a].y.position>viewport_distance)  ||
				(body[a].z.position<-viewport_distance) ||
				(body[a].z.position>viewport_distance)
				)
				body[a].ignore=true;
		}
	}
}
// figure viewport and center
void CnBody::figure_viewport_and_center_2D()
{
	if(!lock_viewport)
	{
		int a;
		for(a=0;a<nBodies;a++)
		{
			if(!body[a].ignore)
			{
				left=body[a].x.position;
				right=body[a].x.position;
				top=body[a].y.position;
				bottom=body[a].y.position;
				break;
			}
		}
		for(;a<nBodies;a++)
		{
			if(!body[a].ignore)
			{
				double d=body[a].radius;
				if(body[a].x.position-d<left) left=body[a].x.position-d;
				if(body[a].x.position+d>right) right=body[a].x.position+d;
				if(body[a].y.position-d<top) top=body[a].y.position-d;
				if(body[a].y.position+d>bottom) bottom=body[a].y.position+d;
			}
		}
		centerx=(right+left)/2.0;
		centery=(bottom+top)/2.0;
	}
	
	//
	nViewable=count_notignored();
}

// figure viewport and center
void CnBody::figure_viewport_and_center_3D()
{
	if(!lock_viewport)
	{
		int a;
		for(a=0;a<nBodies;a++)
		{
			if(!body[a].ignore)
			{
				left=body[a].x.position;
				right=body[a].x.position;
				top=body[a].y.position;
				bottom=body[a].y.position;
				close=body[a].z.position;
				distant=body[a].z.position;
				break;
			}
		}
		for(;a<nBodies;a++)
		{
			if(!body[a].ignore)
			{
				double d=body[a].radius;
				if(body[a].x.position-d<left) left=body[a].x.position-d;
				if(body[a].x.position+d>right) right=body[a].x.position+d;
				if(body[a].y.position-d<top) top=body[a].y.position-d;
				if(body[a].y.position+d>bottom) bottom=body[a].y.position+d;
				if(body[a].z.position-d<close) close=body[a].z.position-d;
				if(body[a].z.position+d>distant) distant=body[a].z.position+d;
			}
		}
		centerx=(right+left)/2.0;
		centery=(bottom+top)/2.0;
		centerz=(distant+close)/2.0;
	}
	
	//
	nViewable=count_notignored();
}








// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////

/*
*/
bool CnBody::ComputeForModel()
{
	bool collision=false;

	//
	frame++;
	iterations++;

	//
	if(dimension_type==THREE_DIMENSIONS)
	{
		//
		reset_collision_flags();
		flag_ignoreable_bodies_3D();
		
		//
		switch(body_type)
		{
		case RIGID_BODY:
			switch(collision_type)
			{
			case STATIC_COLLISION: collision=RigidBody_StaticCollision_3D(); break;
			case DYNAMIC_COLLISION: collision=RigidBody_DynamicCollision_3D(); break;
			}
			break;
		}

		//
		figure_viewport_and_center_3D();
	}
	else //	if(dimension_type==TWO_DIMENSIONS)
	{
		//
		reset_collision_flags();
		flag_ignoreable_bodies_2D();

		//
		switch(body_type)
		{
		case RIGID_BODY:
			switch(collision_type)
			{
			case STATIC_COLLISION: collision=RigidBody_StaticCollision_2D(); break;
			case DYNAMIC_COLLISION: collision=RigidBody_DynamicCollision_2D(); break;
			}
			break;
		}

		//
		figure_viewport_and_center_2D();
	}

	return collision;
}








































// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////


// ////////////////////////////////////////////////////////////////////////

//
bool CnBody::RigidBody_StaticCollision_2D()
{
	int collisions=0;

	int a,b;
	
	// acceleration due to gravity
	for(a=0;a<nBodies;a++)
	{
		double Ax=0.0, Ay=00.0;
		for(b=0;b<nBodies;b++)
		{
			if(a!=b)
			{
				const double dx=body[a].x.position-body[b].x.position;
				const double dy=body[a].y.position-body[b].y.position;
				
				const double d=sqrt( dx*dx + dy*dy);
				const double A = G * ( body[b].mass / (d*d) );
				
				Ax+=(A*dx/d);
				Ay+=(A*dy/d);
			}
		}
		body[a].x.velocity+=Ax*time_factor;
		body[a].y.velocity+=Ay*time_factor;
	}
	
	// detect collisions
	for(a=0;a<nBodies;a++)
	{
		for(b=0;b<nBodies;b++)
		{
			if(a!=b)
			{
				double Vx=body[a].x.position-body[b].x.position;
				double Vy=body[a].y.position-body[b].y.position;
				double Dab=sqrt( Vx*Vx + Vy*Vy); //length of vector
				
				double Rab=body[a].radius+body[b].radius;
				
				if(Dab<Rab) //collision?
				{ //collision....
					collisions++;

					body[a].collision=true;
					
					if(Vx==0) Vx=0.0001; //bug fix
					if(Vy==0) Vy=0.0001; //bug fix
					
					//get the normal N which is the line running from the center of A to B
					double Nx=Vx/Rab; //Dab
					double Ny=Vy/Rab; //Dab
					
					// now get the line at 90 degrees that runs perpendicular to N
					// so that we can apply positive impulse force to A and negative
					// impulse force to B along that that line
					double perpNx=Nx*Vx;
					double perpNy=Ny*Vy;
					
					// relative velocities
					double Vabx=body[a].x.velocity-body[b].x.velocity;
					double Vaby=body[a].y.velocity-body[b].y.velocity;
					
					// impulse
					double Mab=(1/body[a].mass + 1/body[b].mass);
					double jx = (-(1+e)*Vabx*perpNx) / (perpNx*perpNx*Mab);
					double jy = (-(1+e)*Vaby*perpNy) / (perpNy*perpNy*Mab);
					
					// MaVa + jn = MaV'a
					// V'a = Va + (j/Ma) * n
					body[a].x.velocity=(jx/body[a].mass)*perpNx; //*time_factor;
					body[a].y.velocity=(jy/body[a].mass)*perpNy; //*time_factor;
					
					// MbVb - jn = MbV'b
					// V'b = Vb - (j/Mb) * n
					body[b].x.velocity=-(jx/body[b].mass)*perpNx; //*time_factor;
					body[b].y.velocity=-(jy/body[b].mass)*perpNy; //*time_factor;
					
					//
					body[a].x.position+=body[a].x.velocity;
					body[a].y.position+=body[a].y.velocity;
					
					body[b].x.position+=body[b].x.velocity;
					body[b].y.position+=body[b].y.velocity;
				}
				else
					// no collisions, change position due to velocity
				{
					body[a].x.position+=body[a].x.velocity;
					body[a].y.position+=body[a].y.velocity;
				}
			}
		}
	}

	return (collisions>0);
}


// ////////////////////////////////////////////////////////////////////////
//
bool CnBody::RigidBody_StaticCollision_3D()
{
	int collisions=0;

	int a,b;
	
	// acceleration due to gravity
	for(a=0;a<nBodies;a++)
	{
		double Ax=0.0, Ay=00.0, Az=0.0;
		for(b=0;b<nBodies;b++)
		{
			if(a!=b)
			{
				const double dx=body[a].x.position-body[b].x.position;
				const double dy=body[a].y.position-body[b].y.position;
				const double dz=body[a].z.position-body[b].z.position;
				
				const double d=sqrt( dx*dx + dy*dy + dz*dz);
				const double A = G * ( body[b].mass / (d*d*d) );
				
				Ax+=(A*dx/d);
				Ay+=(A*dy/d);
				Az+=(A*dz/d);
			}
		}
		body[a].x.velocity+=Ax*time_factor;
		body[a].y.velocity+=Ay*time_factor;
		body[a].z.velocity+=Az*time_factor;
	}
	
	// detect collisions
	for(a=0;a<nBodies;a++)
	{
		for(b=0;b<nBodies;b++)
		{
			if(a!=b)
			{
				double Vx=body[a].x.position-body[b].x.position;
				double Vy=body[a].y.position-body[b].y.position;
				double Vz=body[a].z.position-body[b].z.position;
				double Dab=sqrt( Vx*Vx + Vy*Vy + Vz*Vz); //length of vector
				
				double Rab=body[a].radius+body[b].radius;
				
				if(Dab<Rab) //collision?
				{ //collision....
					collisions++;

					body[a].collision=true;
					
					if(Vx==0) Vx=0.0001; //bug fix
					if(Vy==0) Vy=0.0001; //bug fix
					if(Vz==0) Vz=0.0001; //bug fix
					
					//get the normal N which is the line running from the center of A to B
					double Nx=Vx/Rab; //Dab
					double Ny=Vy/Rab; //Dab
					double Nz=Vz/Rab; //Dab
					
					// now get the line at 90 degrees that runs perpendicular to N
					// so that we can apply positive impulse force to A and negative
					// impulse force to B along that that line
					double perpNx=Nx*Vx;
					double perpNy=Ny*Vy;
					double perpNz=Nz*Vz;
					
					// relative velocities
					double Vabx=body[a].x.velocity-body[b].x.velocity;
					double Vaby=body[a].y.velocity-body[b].y.velocity;
					double Vabz=body[a].z.velocity-body[b].z.velocity;
					
					// impulse
					double Mab=(1/body[a].mass + 1/body[b].mass);
					double jx = (-(1+e)*Vabx*perpNx) / (perpNx*perpNx*Mab);
					double jy = (-(1+e)*Vaby*perpNy) / (perpNy*perpNy*Mab);
					double jz = (-(1+e)*Vabz*perpNz) / (perpNz*perpNz*Mab);
					
					// MaVa + jn = MaV'a
					// V'a = Va + (j/Ma) * n
					body[a].x.velocity=(jx/body[a].mass)*perpNx;
					body[a].y.velocity=(jy/body[a].mass)*perpNy;
					body[a].z.velocity=(jz/body[a].mass)*perpNz;
					
					// MbVb - jn = MbV'b
					// V'b = Vb - (j/Mb) * n
					body[b].x.velocity=-(jx/body[b].mass)*perpNx;
					body[b].y.velocity=-(jy/body[b].mass)*perpNy;
					body[b].z.velocity=-(jz/body[b].mass)*perpNz;
					
					//
					body[a].x.position+=body[a].x.velocity;
					body[a].y.position+=body[a].y.velocity;
					body[a].z.position+=body[a].z.velocity;
					
					body[b].x.position+=body[b].x.velocity;
					body[b].y.position+=body[b].y.velocity;
					body[b].z.position+=body[b].z.velocity;
				}
				else
					// no collisions, change position due to velocity
				{
					body[a].x.position+=body[a].x.velocity;
					body[a].y.position+=body[a].y.velocity;
					body[a].z.position+=body[a].z.velocity;
				}
			}
		}
	}
	
	return (collisions>0);
}





















// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////


// ////////////////////////////////////////////////////////////////////////

//
bool CnBody::RigidBody_DynamicCollision_2D()
{
	int collisions=0;

	int a,b;
	
	// acceleration due to gravity
	for(a=0;a<nBodies;a++)
	{
		double Ax=0.0, Ay=00.0;
		for(b=0;b<nBodies;b++)
		{
			if(a!=b)
			{
				const double dx=body[a].x.position-body[b].x.position;
				const double dy=body[a].y.position-body[b].y.position;
				
				const double d=sqrt( dx*dx + dy*dy);
				const double A = G * ( body[b].mass / (d*d) );
				
				Ax+=(A*dx/d);
				Ay+=(A*dy/d);
			}
		}
		body[a].x.velocity+=Ax*time_factor;
		body[a].y.velocity+=Ay*time_factor;
	}
	
	// detect collisions
	for(a=0;a<nBodies;a++)
	{
		for(b=0;b<nBodies;b++)
		{
			if(a!=b)
			{
				double Vx=body[a].x.position-body[b].x.position;
				double Vy=body[a].y.position-body[b].y.position;
				double Dab=sqrt( Vx*Vx + Vy*Vy); //length of vector
				
				double Rab=body[a].radius+body[b].radius;
				
				if(Dab<Rab) //collision?
				{ //collision....
					collisions++;

					body[a].collision=true;
					
					if(Vx==0) Vx=0.0001; //bug fix
					if(Vy==0) Vy=0.0001; //bug fix
					
					//get the normal N which is the line running from the center of A to B
					double Nx=Vx/Rab; //Dab
					double Ny=Vy/Rab; //Dab
					
					// now get the line at 90 degrees that runs perpendicular to N
					// so that we can apply positive impulse force to A and negative
					// impulse force to B along that that line
					double perpNx=Nx*Vx;
					double perpNy=Ny*Vy;
					
					// relative velocities
					double Vabx=body[a].x.velocity-body[b].x.velocity;
					double Vaby=body[a].y.velocity-body[b].y.velocity;
					
					// impulse
					double Mab=(1/body[a].mass + 1/body[b].mass);
					double jx = (-(1+e)*Vabx*perpNx) / (perpNx*perpNx*Mab);
					double jy = (-(1+e)*Vaby*perpNy) / (perpNy*perpNy*Mab);
					
					// MaVa + jn = MaV'a
					// V'a = Va + (j/Ma) * n
					body[a].x.velocity=(jx/body[a].mass)*perpNx; //*time_factor;
					body[a].y.velocity=(jy/body[a].mass)*perpNy; //*time_factor;
					
					// MbVb - jn = MbV'b
					// V'b = Vb - (j/Mb) * n
					body[b].x.velocity=-(jx/body[b].mass)*perpNx; //*time_factor;
					body[b].y.velocity=-(jy/body[b].mass)*perpNy; //*time_factor;
					
					//
					body[a].x.position+=body[a].x.velocity;
					body[a].y.position+=body[a].y.velocity;
					
					body[b].x.position+=body[b].x.velocity;
					body[b].y.position+=body[b].y.velocity;
				}
				else
					// no collisions, change position due to velocity
				{
					body[a].x.position+=body[a].x.velocity;
					body[a].y.position+=body[a].y.velocity;
				}
			}
		}
	}

	return (collisions>0);
}


// ////////////////////////////////////////////////////////////////////////
//
bool CnBody::RigidBody_DynamicCollision_3D()
{
	int collisions=0;

	int a,b;
	
	// acceleration due to gravity
	for(a=0;a<nBodies;a++)
	{
		double Ax=0.0, Ay=00.0, Az=0.0;
		for(b=0;b<nBodies;b++)
		{
			if(a!=b)
			{
				const double dx=body[a].x.position-body[b].x.position;
				const double dy=body[a].y.position-body[b].y.position;
				const double dz=body[a].z.position-body[b].z.position;
				
				const double d=sqrt( dx*dx + dy*dy + dz*dz);
				const double A = G * ( body[b].mass / (d*d*d) );
				
				Ax+=(A*dx/d);
				Ay+=(A*dy/d);
				Az+=(A*dz/d);
			}
		}
		body[a].x.velocity+=Ax*time_factor;
		body[a].y.velocity+=Ay*time_factor;
		body[a].z.velocity+=Az*time_factor;
	}
	
	// detect collisions
	for(a=0;a<nBodies;a++)
	{
		for(b=0;b<nBodies;b++)
		{
			if(a!=b)
			{
				double Vx=body[a].x.position-body[b].x.position;
				double Vy=body[a].y.position-body[b].y.position;
				double Vz=body[a].z.position-body[b].z.position;
				double Dab=sqrt( Vx*Vx + Vy*Vy + Vz*Vz); //length of vector
				
				double Rab=body[a].radius+body[b].radius;
				
				if(Dab<Rab) //collision?
				{ //collision....
					collisions++;

					body[a].collision=true;
					
					if(Vx==0) Vx=0.0001; //bug fix
					if(Vy==0) Vy=0.0001; //bug fix
					if(Vz==0) Vz=0.0001; //bug fix
					
					//get the normal N which is the line running from the center of A to B
					double Nx=Vx/Rab; //Dab
					double Ny=Vy/Rab; //Dab
					double Nz=Vz/Rab; //Dab
					
					// now get the line at 90 degrees that runs perpendicular to N
					// so that we can apply positive impulse force to A and negative
					// impulse force to B along that that line
					double perpNx=Nx*Vx;
					double perpNy=Ny*Vy;
					double perpNz=Nz*Vz;
					
					// relative velocities
					double Vabx=body[a].x.velocity-body[b].x.velocity;
					double Vaby=body[a].y.velocity-body[b].y.velocity;
					double Vabz=body[a].z.velocity-body[b].z.velocity;
					
					// impulse
					double Mab=(1/body[a].mass + 1/body[b].mass);
					double jx = (-(1+e)*Vabx*perpNx) / (perpNx*perpNx*Mab);
					double jy = (-(1+e)*Vaby*perpNy) / (perpNy*perpNy*Mab);
					double jz = (-(1+e)*Vabz*perpNz) / (perpNz*perpNz*Mab);
					
					// MaVa + jn = MaV'a
					// V'a = Va + (j/Ma) * n
					body[a].x.velocity=(jx/body[a].mass)*perpNx;
					body[a].y.velocity=(jy/body[a].mass)*perpNy;
					body[a].z.velocity=(jz/body[a].mass)*perpNz;
					
					// MbVb - jn = MbV'b
					// V'b = Vb - (j/Mb) * n
					body[b].x.velocity=-(jx/body[b].mass)*perpNx;
					body[b].y.velocity=-(jy/body[b].mass)*perpNy;
					body[b].z.velocity=-(jz/body[b].mass)*perpNz;
					
					//
					body[a].x.position+=body[a].x.velocity;
					body[a].y.position+=body[a].y.velocity;
					body[a].z.position+=body[a].z.velocity;
					
					body[b].x.position+=body[b].x.velocity;
					body[b].y.position+=body[b].y.velocity;
					body[b].z.position+=body[b].z.velocity;
				}
				else
					// no collisions, change position due to velocity
				{
					body[a].x.position+=body[a].x.velocity;
					body[a].y.position+=body[a].y.velocity;
					body[a].z.position+=body[a].z.velocity;
				}
			}
		}
	}

	return (collisions>0);
}

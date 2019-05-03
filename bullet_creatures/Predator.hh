#ifndef PREDATOR_HH

#define PREDATOR_HH

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//
// "Creature Box" -- flocking app
//
// by Christopher Rasmussen
//
// CISC 440/640, March, 2014
// updated to OpenGL 3.3, March, 2016
//
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include "Creature.hh"
#include "Flocker.hh"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define MAX_PREDATOR_SPEED                           0.04

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class Predator : public Creature {
public:

	glm::vec3 hunger_force;

	Predator(int,                    // index
			 double, double, double, // initial position
			 double, double, double, // initial velocity
			 float, float, float,    // base color
			 int = 1);               // number of past states to save

	void draw(glm::mat4);

	void update();

	bool compute_hunger_force();


};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif

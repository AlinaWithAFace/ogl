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

#include "Predator.hh"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//int flocker_history_length = 30;
//int flocker_draw_mode = DRAW_MODE_POLY;
//vector<Flocker *> flocker_array;
//vector<vector<double> > flocker_squared_distance;

vector<Predator *> predator_array;
int predator_history_length = 30;

extern glm::mat4 ViewMat;
extern glm::mat4 ProjectionMat;

//extern GLuint ModelMatrixID;
//extern GLuint ViewMatrixID;
extern GLuint MatrixID;

extern GLuint predModelMatrixID;
extern GLuint predViewMatrixID;
extern GLuint predMatrixID;
extern GLuint predLightID;
extern GLuint predTextureID;

extern GLuint predator_texture;
extern GLuint predator_vertexbuffer;
extern GLuint predator_uvbuffer;
extern GLuint predator_normalbuffer;
extern GLuint predator_elementbuffer;
extern vector<unsigned short> predator_indices;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

Predator::Predator(int _index,
				   double init_x, double init_y, double init_z,
				   double init_vx, double init_vy, double init_vz,
				   float r, float g, float b,
				   int max_hist) :
		Creature(_index, init_x, init_y, init_z, init_vx, init_vy, init_vz, r, g, b, max_hist) {
}

//----------------------------------------------------------------------------

void Predator::draw(glm::mat4 Model) {
	// set light position

	glm::vec3 lightPos = glm::vec3(4, 4, 4);
	glUniform3f(predLightID, lightPos.x, lightPos.y, lightPos.z);

	glm::mat4 RotationMatrix = glm::mat4(); // identity    -- glm::toMat4(quat_orientations[i]);
	glm::mat4 TranslationMatrix = translate(glm::mat4(), glm::vec3(position.x, position.y, position.z));
	glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;

	glm::mat4 MVP = ProjectionMat * ViewMat * Model * ModelMatrix;

	// Send our transformation to the currently bound shader,
	// in the "MVP" uniform
	glUniformMatrix4fv(predMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(predModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(predViewMatrixID, 1, GL_FALSE, &ViewMat[0][0]);

	// Bind this object's texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, predator_texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(predTextureID, 0);

	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, predator_vertexbuffer);
	glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void *) 0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, predator_uvbuffer);
	glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void *) 0                          // array buffer offset
	);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, predator_normalbuffer);
	glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void *) 0                          // array buffer offset
	);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, predator_elementbuffer);

	// Draw the triangles !
	glDrawElements(
			GL_TRIANGLES,      // mode
			predator_indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void *) 0           // element array buffer offset
	);


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

}

//----------------------------------------------------------------------------

void Predator::update() {

	// set accelerations (aka forces)

	acceleration = glm::vec3(0, 0, 0);

	// deterministic behaviors

//	compute_separation_force();
//	acceleration += separation_force;
//
//	compute_alignment_force();
//	acceleration += alignment_force;
//
//	compute_cohesion_force();
//	acceleration += cohesion_force;

//	draw_color.r = glm::length(separation_force);
//	draw_color.g = glm::length(alignment_force);
//	draw_color.b = glm::length(cohesion_force);
	if (draw_color.r > 0 || draw_color.g > 0 || draw_color.b > 0)
		draw_color = glm::normalize(draw_color);
	else
		draw_color = base_color;

	// randomness

//	if (random_force_limit > 0.0) {
//		acceleration.x += uniform_random(-random_force_limit, random_force_limit);
//		acceleration.y += uniform_random(-random_force_limit, random_force_limit);
//		acceleration.z += uniform_random(-random_force_limit, random_force_limit);
//	}

	// update velocity

	new_velocity = velocity + acceleration;   // scale acceleration by dt?

	// limit velocity

	double mag = glm::length(new_velocity);
	if (mag > MAX_FLOCKER_SPEED)
		new_velocity *= (float) (MAX_FLOCKER_SPEED / mag);

	// update position

	new_position = position + new_velocity;   // scale new_velocity by dt?

}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
